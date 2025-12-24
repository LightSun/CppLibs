#include "algo/main/attn/flash_attention.h"

namespace h7_attn {

void test_attention_impl(){
    std::cout << "=== FlashAttention 测试 ===" << std::endl;

    // 测试参数
    int N = 256;     // 序列长度
    int d = 64;      // 查询/键维度
    int d_v = 64;    // 值维度

    std::cout << "序列长度: " << N << std::endl;
    std::cout << "特征维度: " << d << std::endl;
    std::cout << "值维度: " << d_v << std::endl;

    // 创建测试数据
    Tensor Q(N, d);
    Tensor K(N, d);
    Tensor V(N, d_v);

    Q.random_init(-1.0f, 1.0f);
    K.random_init(-1.0f, 1.0f);
    V.random_init(-1.0f, 1.0f);

    // 测试标准注意力
    std::cout << "\n1. 标准注意力测试..." << std::endl;
    StandardAttention std_attn;
    Tensor std_result = std_attn.forward(Q, K, V);

    // 测试FlashAttention
    std::cout << "\n2. FlashAttention测试..." << std::endl;
    FlashAttention flash_attn(64, 64);
    Tensor flash_result = flash_attn.forward(Q, K, V);

    // 测试FlashAttention-2
    std::cout << "\n3. FlashAttention-2测试..." << std::endl;
    FlashAttention2 flash_attn2(64);
    Tensor flash2_result = flash_attn2.forward(Q, K, V);

    // 验证结果一致性
    std::cout << "\n=== 结果验证 ===" << std::endl;
    bool match1 = std_result.is_close(flash_result, 1e-4, 1e-6);
    bool match2 = std_result.is_close(flash2_result, 1e-4, 1e-6);

    std::cout << "标准注意力 vs FlashAttention: "
              << (match1 ? "✓ 匹配" : "✗ 不匹配") << std::endl;
    std::cout << "标准注意力 vs FlashAttention-2: "
              << (match2 ? "✓ 匹配" : "✗ 不匹配") << std::endl;

    // 打印部分结果
    if (N <= 16) {
        std::cout << "\n=== 结果示例（前5x5）===" << std::endl;
        std_result.print("标准注意力结果", 5, 5);
        flash_result.print("FlashAttention结果", 5, 5);
        flash2_result.print("FlashAttention-2结果", 5, 5);
    }

    // 内存分析
    std::cout << "\n=== 内存分析 ===" << std::endl;
    size_t std_memory = N * N * sizeof(float);  // 注意力矩阵
    size_t flash_memory = N * d_v * sizeof(float) + 2 * N * sizeof(float);  // 输出 + 统计量

    std::cout << "标准注意力峰值内存: " << std_memory / 1024 << " KB" << std::endl;
    std::cout << "FlashAttention峰值内存: " << flash_memory / 1024 << " KB" << std::endl;
    std::cout << "内存节省比例: " << (1.0 - (double)flash_memory / std_memory) * 100 << "%" << std::endl;
}

void benchmark_attention(){
    std::cout << "\n=== 性能基准测试 ===" << std::endl;

    std::vector<int> sequence_lengths = {128, 256, 512, 1024};
    int d = 64;
    int d_v = 64;

    for (int N : sequence_lengths) {
        std::cout << "\n序列长度: " << N << std::endl;

        Tensor Q(N, d);
        Tensor K(N, d);
        Tensor V(N, d_v);

        Q.random_init();
        K.random_init();
        V.random_init();

        // 测试每种实现
        StandardAttention std_attn;
        FlashAttention flash_attn(64, 64);
        FlashAttention2 flash_attn2(64);

        auto start = std::chrono::high_resolution_clock::now();
        auto result_std = std_attn.forward(Q, K, V);
        auto end = std::chrono::high_resolution_clock::now();
        auto time_std = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        start = std::chrono::high_resolution_clock::now();
        auto result_flash = flash_attn.forward(Q, K, V);
        end = std::chrono::high_resolution_clock::now();
        auto time_flash = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        start = std::chrono::high_resolution_clock::now();
        auto result_flash2 = flash_attn2.forward(Q, K, V);
        end = std::chrono::high_resolution_clock::now();
        auto time_flash2 = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        std::cout << "标准注意力: " << time_std / 1000.0 << " ms" << std::endl;
        std::cout << "FlashAttention: " << time_flash / 1000.0 << " ms" << std::endl;
        std::cout << "FlashAttention-2: " << time_flash2 / 1000.0 << " ms" << std::endl;
        std::cout << "加速比(Flash/Std): " << (double)time_std / time_flash << "x" << std::endl;
    }
}

}

void test_attn_all(){
    using namespace h7_attn;
    std::cout << "=========================================" << std::endl;
    std::cout << "      FlashAttention C++ 实现演示        " << std::endl;
    std::cout << "=========================================" << std::endl;

    // 测试注意力实现
    test_attention_impl();

    // 性能基准测试
    //benchmark_attention();

    std::cout << "\n测试完成！" << std::endl;
}

