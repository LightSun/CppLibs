#include <immintrin.h>
#include <math.h>

// 激活函数类型枚举
typedef enum
{
    RELU,
    SIGMOID,
    TAHN
} act_type_t;

// 通用激活函数宏
#define DEFINE_ACTIVATION(TYPE, T, SUFFIX)   \
    static inline T activation_##SUFFIX(T x) \
    {                                        \
        return TYPE##_IMPL(x);               \
    }

// ReLU实现
#define RELU_IMPL(x) ((x) > 0 ? (x) : 0)
// DEFINE_ACTIVATION(RELU, float, relu_f32)
DEFINE_ACTIVATION(RELU, double, relu_f64)

// Sigmoid实现
#define SIGMOID_IMPL(x) (1.0 / (1.0 + exp(-(x))))
DEFINE_ACTIVATION(SIGMOID, float, sigmoid_f32)
DEFINE_ACTIVATION(SIGMOID, double, sigmoid_f64)

// Tanh实现
#define TANH_IMPL(x) tanh(x)
DEFINE_ACTIVATION(TANH, float, tanh_f32)
DEFINE_ACTIVATION(TANH, double, tanh_f64)

// AVX2优化的ReLU特化，使用宏实现，避免了代码可能无法被inline，直接被展开
#define ACTIVATION_RELU_F32(x) ({ \
    __m128 vec = _mm_set_ss(x); \
    vec = _mm_max_ss(vec, _mm_setzero_ps()); \
    _mm_cvtss_f32(vec); \
})
//分块矩阵乘法和主体计算模板
#include <math.h>
#include <string.h>
#include "activation.h"

// 基础分块计算宏提前特化版本
#define DEFINE_BLOCK_F32_16x16x4(SUFFIX, ACT_FUNC)                                    \
    static void block_gemm_16x16x4_##SUFFIX(const float *A, const float *B, float *C, \
                                            int M, int N, int K,                      \
                                            int m_start, int n_start, int k_start)    \
    {                                                                                 \
        for (int i = 0; i < 16; ++i)                                                  \
        {                                                                             \
            for (int j = 0; j < 16; ++j)                                              \
            {                                                                         \
                float sum = C[(m_start + i) * N + (n_start + j)];                     \
                for (int k = 0; k < 4; ++k)                                           \
                {                                                                     \
                    sum += A[(m_start + i) * K + (k_start + k)] *                     \
                           B[(k_start + k) * N + (n_start + j)];                      \
                }                                                                     \
                C[(m_start + i) * N + (n_start + j)] = ACT_FUNC(sum);                 \
            }                                                                         \
        }                                                                             \
    }

DEFINE_BLOCK_F32_16x16x4(f32_sigmoid, activation_sigmoid_f32)
    DEFINE_BLOCK_F32_16x16x4(f32_relu, ACTIVATION_RELU_F32)
// 基础分块计算宏一般版本
#define DEFINE_BLOCK_GEMM(DTYPE, SUFFIX, BLOCK_M, BLOCK_N, BLOCK_K, ACT_FUNC)                                     \
    static void block_gemm_##BLOCK_M##x##BLOCK_N##x##BLOCK_K##_##SUFFIX(const DTYPE *A, const DTYPE *B, DTYPE *C, \
                                                                        int M, int N, int K,                      \
                                                                        int m_start, int n_start, int k_start)    \
    {                                                                                                             \
        for (int i = 0; i < BLOCK_M; ++i)                                                                         \
        {                                                                                                         \
            for (int j = 0; j < BLOCK_N; ++j)                                                                     \
            {                                                                                                     \
                DTYPE sum = C[(m_start + i) * N + (n_start + j)];                                                 \
                for (int k = 0; k < BLOCK_K; ++k)                                                                 \
                {                                                                                                 \
                    sum += A[(m_start + i) * K + (k_start + k)] *                                                 \
                           B[(k_start + k) * N + (n_start + j)];                                                  \
                }                                                                                                 \
                C[(m_start + i) * N + (n_start + j)] = ACT_FUNC(sum);                                             \
            }                                                                                                     \
        }                                                                                                         \
    }

// 调度器宏
#define GEMM_IMPL(DTYPE, SUFFIX, BLOCK_M, BLOCK_N, BLOCK_K)                                               \
    void gemm_impl_##BLOCK_M##x##BLOCK_N##x##BLOCK_K##_##SUFFIX(const DTYPE *A, const DTYPE *B, DTYPE *C, \
                                                                int M, int N, int K)                      \
    {                                                                                                     \
        for (int m = 0; m < M; m += BLOCK_M)                                                              \
        {                                                                                                 \
            int m_end = m + BLOCK_M < M ? m + BLOCK_M : M;                                                \
            for (int n = 0; n < N; n += BLOCK_N)                                                          \
            {                                                                                             \
                int n_end = n + BLOCK_N < N ? n + BLOCK_N : N;                                            \
                for (int k = 0; k < K; k += BLOCK_K)                                                      \
                {                                                                                         \
                    int k_end = k + BLOCK_K < K ? k + BLOCK_K : K;                                        \
                    block_gemm_##BLOCK_M##x##BLOCK_N##x##BLOCK_K##_##SUFFIX(A, B, C, M, N, K, m, n, k);   \
                }                                                                                         \
            }                                                                                             \
        }                                                                                                 \
    }

// 组合宏：生成指定分块和激活函数的实现
#define GENERATE_IMPLEMENTATION(DTYPE, SUFFIX, BLOCK_M, BLOCK_N, BLOCK_K, ACT_FUNC) \
    DEFINE_BLOCK_GEMM(DTYPE, SUFFIX, BLOCK_M, BLOCK_N, BLOCK_K, ACT_FUNC)           \
    GEMM_IMPL(DTYPE, SUFFIX, BLOCK_M, BLOCK_N, BLOCK_K)

    /******************** 具体实现实例化 ********************/
    // float类型实现

    // 提前特化版本
    // 小矩阵: 16x16x4 + ReLU
    GEMM_IMPL(float, f32_relu, 16, 16, 4)
    // 小矩阵: 16x16x4 + Sigmoid
    GEMM_IMPL(float, f32_sigmoid, 16, 16, 4)


    // 大矩阵: 32x32x8 + ReLU
    GENERATE_IMPLEMENTATION(float, f32_relu, 32, 32, 8, ACTIVATION_RELU_F32)
    // 大矩阵: 32x32x8 + Sigmoid
    GENERATE_IMPLEMENTATION(float, f32_sigmoid, 32, 32, 8, activation_sigmoid_f32)
    // 超大矩阵: 64x64x16 + ReLU
    GENERATE_IMPLEMENTATION(double, f64_relu, 64, 64, 16, activation_relu_f64)
    // 超大矩阵: 64x64x16 + Sigmoid
    GENERATE_IMPLEMENTATION(double, f64_sigmoid, 64, 64, 16, activation_sigmoid_f64)

    typedef enum dtype_e {
        F32,
        F64
    } dtype_t;

/******************** 入口函数选择逻辑 ********************/
void gemm_with_activation(dtype_t dtype, act_type_t act_type, const void *A,
                          const void *B,
                          void *C,
                          int M, int N, int K)
{
    if (dtype == F32)
    {
        if (M * N * K < 4096)
        {
            if (act_type == RELU)
            {
                gemm_impl_16x16x4_f32_relu((const float *)A, (const float *)B, (float *)C, M, N, K);
            }
            else
            {
                gemm_impl_16x16x4_f32_sigmoid((const float *)A, (const float *)B, (float *)C, M, N, K);
            }
        }
        else
        {
            if (act_type == RELU)
            {
                gemm_impl_32x32x8_f32_relu((const float *)A, (const float *)B, (float *)C, M, N, K);
            }
            else
            {
                gemm_impl_32x32x8_f32_sigmoid((const float *)A, (const float *)B, (float *)C, M, N, K);
            }
        }
    }
    else
    {
        if (act_type == RELU)
        {
            gemm_impl_64x64x16_f64_relu((const double *)A, (const double *)B, (double *)C, M, N, K);
        }
        else
        {
            gemm_impl_64x64x16_f64_sigmoid((const double *)A, (const double *)B, (double *)C, M, N, K);
        }
    }
}
