#include <cmath>
#include <type_traits>
#include "activation.h"

// 基础内层kernel模板（需特化）
template <int BLOCK_M, int BLOCK_N, int BLOCK_K, typename T, template <typename> class ActivationFunc>
struct BlockGEMM
{
    static void compute(const T *A, const T *B, T *C,
                        int M, int N, int K,
                        int m_start, int n_start, int k_start)
    {
        {
            // 手动展开的SIMD优化计算逻辑
            for (int i = 0; i < BLOCK_M; ++i)
            {
                for (int j = 0; j < BLOCK_N; ++j)
                {
                    T sum = C[(m_start + i) * N + (n_start + j)];
                    for (int k = 0; k < BLOCK_N; ++k)
                    {
                        sum += A[(m_start + i) * K + (k_start + k)] *
                               B[(k_start + k) * N + (n_start + j)];
                    }
                    C[(m_start + i) * N + (n_start + j)] = ActivationFunc<T>::apply(sum);
                }
            }
        }
    }
};

// 特化16x16x4分块
template <typename T, template <typename> class ActivationFunc>
struct BlockGEMM<16, 16, 4, T, ActivationFunc>
{
    static void compute(const T *A, const T *B, T *C,
                        int M, int N, int K,
                        int m_start, int n_start, int k_start)
    {
        // 手动展开的SIMD优化计算逻辑
        for (int i = 0; i < 16; ++i)
        {
            for (int j = 0; j < 16; ++j)
            {
                T sum = C[(m_start + i) * N + (n_start + j)];
                for (int k = 0; k < 4; ++k)
                {
                    sum += A[(m_start + i) * K + (k_start + k)] *
                           B[(k_start + k) * N + (n_start + j)];
                }
                C[(m_start + i) * N + (n_start + j)] = ActivationFunc<T>::apply(sum);
            }
        }
    }
};

// 特化32x32x8分块（AVX512优化版本）
template <typename T, template <typename> class ActivationFunc>
struct BlockGEMM<32, 32, 8, T, ActivationFunc>
{
    static void compute(const T *A, const T *B, T *C,
                        int M, int N, int K,
                        int m_start, int n_start, int k_start)
    {
        // 使用AVX512指令的展开实现
        // ...（具体向量化实现逻辑）
    }
};

// 特化64x64x16分块，数据类型为double（AVX512优化版本）
template <template <typename> class ActivationFunc>
struct BlockGEMM<32, 32, 8, double, ActivationFunc>
{
    static void compute(const double *A, const double *B, double *C,
                        int M, int N, int K,
                        int m_start, int n_start, int k_start)
    {
        // 使用AVX512指令的展开实现
        // ...（具体向量化实现逻辑）
    }
};

// 调度器：根据分块策略选择实现
template <int BLOCK_M, int BLOCK_N, int BLOCK_K, typename T,
         template <typename> class ActivationFunc>
void gemm_impl(const T *A, const T *B, T *C, int M, int N, int K)
{
    static_assert(BLOCK_M > 0 && BLOCK_N > 0 && BLOCK_K > 0,
                  "Block size must be positive");

    for (int m = 0; m < M; m += BLOCK_M)
    {
        int m_end = std::min(m + BLOCK_M, M);
        for (int n = 0; n < N; n += BLOCK_N)
        {
            int n_end = std::min(n + BLOCK_N, N);
            for (int k = 0; k < K; k += BLOCK_K)
            {
                int k_end = std::min(k + BLOCK_K, K);

                BlockGEMM<BLOCK_M, BLOCK_N, BLOCK_K, T, ActivationFunc>::compute(
                    A, B, C, M, N, K, m, n, k);
            }
        }
    }
}

// 入口函数：自动选择最优分块策略
template <typename T, ActType act_type>
void gemm_with_activation(const T *A, const T *B, T *C, int M, int N, int K)
{
    if constexpr (std::is_same_v<T, float>)
    {
        if (M * N * K < 4096)
        {
            if constexpr (act_type == ActType::relu)
            {
                gemm_impl<16, 16, 4, float, ReLU>(A, B, C, M, N, K);
            }
            else
            {
                gemm_impl<16, 16, 4, float, Sigmoid<float>>(A, B, C, M, N, K);
            }
        }
        else
        {
            if constexpr (act_type == ActType::relu)
            {
                gemm_impl<32, 32, 8, float, ReLU>(A, B, C, M, N, K);
            }
            else
            {
                gemm_impl<32, 32, 8, float, Sigmoid<float>>(A, B, C, M, N, K);
            }
        }
    }
    else if constexpr (std::is_same_v<T, double>)
    {
        if constexpr (act_type == ActType::relu)
        {
            gemm_impl<64, 64, 16, double, ReLU<double>>(A, B, C, M, N, K);
        }
        else
        {
            gemm_impl<64, 64, 16, double, Sigmoid<double>>(A, B, C, M, N, K);
        }
    }
}
