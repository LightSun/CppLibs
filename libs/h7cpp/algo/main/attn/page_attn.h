#pragma once

#include <vector>
#include <cmath>
#include <unordered_map>

//just used to recognize.
namespace h7_page_attn {

// 定义物理块的数据结构
struct PhysicalBlock {
    float* k_data; // 指向Key数据的指针
    float* v_data; // 指向Value数据的指针
    int block_id;  // 物理块ID
    bool is_free;  // 是否空闲
};

// 分页注意力计算参数
struct PagedAttentionParams {
    // 输入Query: [num_tokens, num_heads, head_size]
    const float* query;

    // 全局分页KV缓存池
    // 假设布局: [num_physical_blocks, 2, num_heads, head_size, block_size]
    // 其中第2维: 0=Key, 1=Value
    const float* kv_cache;

    // 块表: [batch_size][variable_length] 存储物理块ID
    const std::vector<std::vector<int>>* block_tables;

    // 序列元数据
    const int* context_lens; // 每个序列的当前长度 [batch_size]
    const int* slot_mapping; // 可选的：预先计算好的 token->物理位置 映射

    // 输出: [num_tokens, num_heads, head_size]
    float* output;

    // 超参数
    int num_tokens;
    int num_heads;
    int head_size;
    int block_size;
    float scale; // 通常为 1.0f / sqrt(head_size)
};

// 简化的分页注意力计算函数 (CPU概念版本，便于理解)
void paged_attention_impl(const PagedAttentionParams& params, int token_idx, int head_idx) {
    // 1. 获取当前token的查询向量
    const float* q = params.query +
                     token_idx * params.num_heads * params.head_size +
                     head_idx * params.head_size;

    // 2. 确定当前token属于哪个序列 (此处简化，实际需通过元数据查找)
    int seq_id = 0; // 假设为序列0
    int context_len = params.context_lens[seq_id];

    // 3. 准备累加器
    float max_logit = -INFINITY;
    float sum_exp = 0.0f;
    std::vector<float> acc_v(params.head_size, 0.0f);

    // 4. 遍历该序列中当前token之前的所有历史token
    for (int pos = 0; pos < context_len; ++pos) {
        // a. 关键步骤：通过分页机制定位物理内存
        //    计算当前历史token所在的逻辑块和在块内的偏移
        int logical_block_id = pos / params.block_size;
        int offset_in_block = pos % params.block_size;

        // b. 查询块表，获取存储该逻辑块的物理块ID
        int physical_block_id = (*params.block_tables)[seq_id][logical_block_id];

        // c. 计算该token的Key在kv_cache中的精确内存地址
        //    公式：物理块起始地址 + Key/Value区分 + 头偏移 + 特征维度偏移 + 块内token偏移
        //    (此处为示意，实际GPU kernel中索引计算更复杂以优化合并访存[citation:10])
        long long k_offset = static_cast<long long>(physical_block_id) * 2 * params.num_heads * params.head_size * params.block_size
                             + 0 // 选择Key部分
                             + head_idx * params.head_size * params.block_size
                             + offset_in_block * params.head_size; // 注意步长

        const float* k_ptr = params.kv_cache + k_offset;

        // d. 计算点积 Q·K^T
        float qk = 0.0f;
        for (int i = 0; i < params.head_size; ++i) {
            qk += q[i] * k_ptr[i]; // 注意：实际布局中k_ptr的步长可能不同[citation:10]
        }
        qk *= params.scale;

        // e. 在线数值稳定的Softmax（避免存储中间注意力分数矩阵）
        if (qk > max_logit) {
            // 更新最大值，并重新调整之前的累加值
            for (int i = 0; i < params.head_size; ++i) {
                acc_v[i] *= expf(max_logit - qk);
            }
            sum_exp *= expf(max_logit - qk);
            max_logit = qk;
        }

        float exp_qk = expf(qk - max_logit);
        sum_exp += exp_qk;

        // f. 同时，定位并累加Value
        long long v_offset = k_offset + params.num_heads * params.head_size * params.block_size; // 跳到Value部分
        const float* v_ptr = params.kv_cache + v_offset;

        for (int i = 0; i < params.head_size; ++i) {
            acc_v[i] += exp_qk * v_ptr[i];
        }
    }

    // 5. 最终归一化并写入输出
    float* out_ptr = params.output +
                     token_idx * params.num_heads * params.head_size +
                     head_idx * params.head_size;
    float inv_sum = 1.0f / (sum_exp + 1e-12f);
    for (int i = 0; i < params.head_size; ++i) {
        out_ptr[i] = acc_v[i] * inv_sum;
    }
}

// 一个简化的块管理器类 (运行在Host端)
class BlockManager {
private:
    std::vector<PhysicalBlock> physical_blocks; // 物理块池
    std::vector<int> free_block_ids;           // 空闲块ID列表
    // 记录序列到物理块的映射：seq_id -> vector<physical_block_id>
    std::unordered_map<int, std::vector<int>> seq_to_blocks;

public:
    BlockManager(int num_blocks, int block_size, int num_heads, int head_size) {
        // 初始化物理块池，分配内存
        physical_blocks.resize(num_blocks);
        for (int i = 0; i < num_blocks; ++i) {
            physical_blocks[i].k_data = new float[2 * num_heads * head_size * block_size];
            physical_blocks[i].v_data = physical_blocks[i].k_data + num_heads * head_size * block_size; // Value紧随Key之后
            physical_blocks[i].block_id = i;
            physical_blocks[i].is_free = true;
            free_block_ids.push_back(i);
        }
    }

    ~BlockManager() {
        for (auto& block : physical_blocks) {
            delete[] block.k_data;
        }
    }

    // 为序列分配一个新的物理块
    int allocate_block_for_sequence(int seq_id) {
        if (free_block_ids.empty()) {
            return -1; // 分配失败，OOM
        }
        int block_id = free_block_ids.back();
        free_block_ids.pop_back();
        physical_blocks[block_id].is_free = false;
        seq_to_blocks[seq_id].push_back(block_id);
        return block_id;
    }

    // 获取序列的块表（逻辑块索引 -> 物理块ID）
    std::vector<int> get_block_table(int seq_id) {
        return seq_to_blocks[seq_id];
    }

    // 释放序列占用的所有块
    void free_sequence_blocks(int seq_id) {
        for (int block_id : seq_to_blocks[seq_id]) {
            physical_blocks[block_id].is_free = true;
            free_block_ids.push_back(block_id);
        }
        seq_to_blocks.erase(seq_id);
    }
};

}
