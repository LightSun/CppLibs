#pragma once


#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>

namespace h7_page_attn2 {

// ============================ 第一部分：模拟数据结构 ============================
// 定义一个物理块。为了简化，我们假设一个块只存一个头的Key和Value数据。
struct PhysicalBlock {
    int block_id;
    // 每个块存储固定大小（block_size）的token数据
    std::vector<float> k_data; // Key数据
    std::vector<float> v_data; // Value数据
    bool is_free;

    PhysicalBlock(int id, int block_size, int head_size)
        : block_id(id), k_data(block_size * head_size, 0.0f),
          v_data(block_size * head_size, 0.0f), is_free(true) {}
};

// 块管理器：管理所有物理块，并处理分配和映射
class BlockManager {
private:
    std::vector<PhysicalBlock> blocks;
    std::vector<int> free_list; // 空闲块ID列表
    // 块表：记录序列的逻辑块索引 -> 物理块ID
    std::vector<int> block_table;

public:
    int block_size;   // 每个块能存的token数
    int head_size;    // 每个注意力头的维度

    BlockManager(int num_blocks, int b_size, int h_size)
        : block_size(b_size), head_size(h_size) {
        // 初始化物理块池
        for (int i = 0; i < num_blocks; ++i) {
            blocks.emplace_back(i, block_size, head_size);
            free_list.push_back(i);
        }
        std::cout << "初始化块管理器: " << num_blocks << " 个物理块，块大小="
                  << block_size << ", 头维度=" << head_size << std::endl;
    }

    // 为序列分配一个物理块，并更新块表
    int allocate_block_for_sequence() {
        if (free_list.empty()) {
            std::cerr << "错误: 没有空闲块了！" << std::endl;
            return -1;
        }
        int physical_id = free_list.back();
        free_list.pop_back();
        blocks[physical_id].is_free = false;
        block_table.push_back(physical_id);
        // 将新分配的物理块ID加入块表
        std::cout << "  分配物理块 " << physical_id << " 给序列。当前块表: [";
        for (int id : block_table) std::cout << id << " ";
        std::cout << "]" << std::endl;
        return physical_id;
    }

    // 获取序列的块表（供注意力计算时查询）
    const std::vector<int>& get_block_table() const {
        return block_table;
    }

    // 根据块表和偏移，获取Key或Value数据指针
    float* get_k_ptr(int logical_block_id, int offset_in_block) {
        int physical_id = block_table[logical_block_id];
        return &(blocks[physical_id].k_data[offset_in_block * head_size]);
    }
    float* get_v_ptr(int logical_block_id, int offset_in_block) {
        int physical_id = block_table[logical_block_id];
        return &(blocks[physical_id].v_data[offset_in_block * head_size]);
    }

    // 在指定位置写入数据（模拟填充KV缓存）
    void write_kv_to_block(int logical_block_id, int offset_in_block,
                           const std::vector<float>& k_vec, const std::vector<float>& v_vec) {
        int physical_id = block_table[logical_block_id];
        int start_idx = offset_in_block * head_size;
        for (int i = 0; i < head_size; ++i) {
            blocks[physical_id].k_data[start_idx + i] = k_vec[i];
            blocks[physical_id].v_data[start_idx + i] = v_vec[i];
        }
        std::cout << "  写入KV数据 -> 逻辑块" << logical_block_id
                  << "(物理块" << physical_id << "), 块内偏移" << offset_in_block << std::endl;
    }
};

// ============================ 第二部分：简化的分页注意力计算 ============================
void paged_attention_demo(
    const float* query,          // 当前查询向量 [head_size]
    BlockManager& manager,       // 块管理器（包含所有数据和映射）
    int context_len,             // 当前序列的总长度（token数）
    float* output                // 输出向量 [head_size]
    ) {
    int block_size = manager.block_size;
    int head_size = manager.head_size;
    const std::vector<int>& block_table = manager.get_block_table();

    std::cout << "\n[开始分页注意力计算]" << std::endl;
    std::cout << "  序列长度: " << context_len << ", 块大小: " << block_size
              << ", 需要逻辑块数: " << (context_len + block_size - 1) / block_size << std::endl;

    // 用于在线Softmax的变量
    float max_logit = -INFINITY;
    float sum_exp = 0.0f;
    std::vector<float> acc_v(head_size, 0.0f);
    const float scale = 1.0f / std::sqrt(head_size);

    // 遍历所有历史token
    for (int pos = 0; pos < context_len; ++pos) {
        // ***** 核心步骤：通过分页机制定位物理内存 *****
        int logical_block_id = pos / block_size;        // 属于哪个逻辑块
        int offset_in_block = pos % block_size;         // 在块内的偏移
        // 注意：这里不需要直接计算物理地址，而是通过管理器查询
        const float* k_ptr = manager.get_k_ptr(logical_block_id, offset_in_block);
        const float* v_ptr = manager.get_v_ptr(logical_block_id, offset_in_block);

        // 计算 Q·K^T
        float qk = 0.0f;
        for (int i = 0; i < head_size; ++i) {
            qk += query[i] * k_ptr[i];
        }
        qk *= scale;

        // 在线Softmax（数值稳定版本）
        float exp_qk;
        if (qk > max_logit) {
            // 发现新的最大值，重新调整之前的累加值
            float old_max = max_logit;
            max_logit = qk;
            exp_qk = std::exp(qk - max_logit); // 现在exp_qk=1.0
            float rescale = std::exp(old_max - max_logit);
            sum_exp *= rescale;
            for (int i = 0; i < head_size; ++i) {
                acc_v[i] *= rescale;
            }
            sum_exp += exp_qk;
        } else {
            exp_qk = std::exp(qk - max_logit);
            sum_exp += exp_qk;
        }

        // 累加加权后的Value
        for (int i = 0; i < head_size; ++i) {
            acc_v[i] += exp_qk * v_ptr[i];
        }

        std::cout << "  Token位置 " << pos << " -> 逻辑块" << logical_block_id
                  << ", 偏移" << offset_in_block << ", QK值=" << qk << std::endl;
    }

    // 最终归一化，得到输出
    float inv_sum = 1.0f / (sum_exp + 1e-12f);
    for (int i = 0; i < head_size; ++i) {
        output[i] = acc_v[i] * inv_sum;
    }
    std::cout << "[计算完成]\n" << std::endl;
}

}

void main_test_page_attn2();
