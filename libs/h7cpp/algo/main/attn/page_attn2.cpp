#include "algo/main/attn/page_attn2.h"


using namespace h7_page_attn2;


void main_test_page_attn2(){
    std::cout << "===== PagedAttention 简易Demo开始 =====" << std::endl;

    // 1. 初始化参数（使用极小的规模以便演示）
    const int NUM_PHYSICAL_BLOCKS = 2;  // 只有2个物理块
    const int BLOCK_SIZE = 4;           // 每个块存4个token
    const int HEAD_SIZE = 8;            // 注意力头维度为8
    const int CONTEXT_LEN = 7;          // 我们的序列长度为7 (需要2个逻辑块)

    // 2. 初始化块管理器（模拟GPU显存中的KV缓存池）
    BlockManager manager(NUM_PHYSICAL_BLOCKS, BLOCK_SIZE, HEAD_SIZE);

    // 3. 模拟序列生成过程：随着token增加，按需分配物理块
    std::cout << "\n--- 模拟序列生成与KV缓存填充 ---" << std::endl;
    // 为逻辑块0分配物理块（当第一个token到来时触发）
    manager.allocate_block_for_sequence();
    // 为逻辑块1分配物理块（当第5个token到来，块0已满时触发）
    manager.allocate_block_for_sequence();

    // 4. 模拟写入一些随机的KV数据（模拟历史KV缓存）
    std::vector<float> dummy_k(HEAD_SIZE);
    std::vector<float> dummy_v(HEAD_SIZE);
    for (int pos = 0; pos < CONTEXT_LEN; ++pos) {
        for (int i = 0; i < HEAD_SIZE; ++i) {
            dummy_k[i] = static_cast<float>(rand() % 100) / 100.0f - 0.5f; // 随机值[-0.5,0.5)
            dummy_v[i] = static_cast<float>(rand() % 100) / 100.0f - 0.5f;
        }
        int logical_block_id = pos / BLOCK_SIZE;
        int offset_in_block = pos % BLOCK_SIZE;
        manager.write_kv_to_block(logical_block_id, offset_in_block, dummy_k, dummy_v);
    }

    // 5. 模拟一个当前查询token
    std::vector<float> query(HEAD_SIZE);
    std::vector<float> output(HEAD_SIZE);
    std::cout << "\n--- 生成随机查询向量 ---" << std::endl;
    for (int i = 0; i < HEAD_SIZE; ++i) {
        query[i] = static_cast<float>(rand() % 100) / 100.0f - 0.5f;
        std::cout << query[i] << " ";
    }
    std::cout << std::endl;

    // 6. 执行分页注意力计算
    paged_attention_demo(query.data(), manager, CONTEXT_LEN, output.data());

    // 7. 打印结果
    std::cout << "注意力输出向量: ";
    for (float val : output) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    std::cout << "\n===== Demo结束 =====" << std::endl;
}
