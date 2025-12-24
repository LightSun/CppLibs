#pragma once

#include "algo/main/attn/attn_tensor.h"

namespace h7_attn {
namespace tensor_ops {

// 矩阵乘法
Tensor matmul(const Tensor& A, const Tensor& B) {
    assert(A.cols() == B.rows());
    int m = A.rows();
    int n = B.cols();
    int k = A.cols();

    Tensor C(m, n);

    for (int i = 0; i < m; ++i) {
        const float* a_row = A.row_ptr(i);
        float* c_row = C.row_ptr(i);

        for (int j = 0; j < n; ++j) {
            float sum = 0.0f;
            for (int t = 0; t < k; ++t) {
                sum += a_row[t] * B(t, j);
            }
            c_row[j] = sum;
        }
    }

    return C;
}

// 矩阵乘法优化版（缓存友好）
Tensor matmul_optimized(const Tensor& A, const Tensor& B) {
    assert(A.cols() == B.rows());
    int m = A.rows();
    int n = B.cols();
    int k = A.cols();

    Tensor C(m, n);

    // 分块大小
    const int block_size = 64;

    for (int i0 = 0; i0 < m; i0 += block_size) {
        int i1 = std::min(i0 + block_size, m);

        for (int j0 = 0; j0 < n; j0 += block_size) {
            int j1 = std::min(j0 + block_size, n);

            for (int k0 = 0; k0 < k; k0 += block_size) {
                int k1 = std::min(k0 + block_size, k);

                // 计算当前块
                for (int i = i0; i < i1; ++i) {
                    const float* a_row = A.row_ptr(i);
                    float* c_row = C.row_ptr(i);

                    for (int kt = k0; kt < k1; ++kt) {
                        float a_val = a_row[kt];

                        for (int j = j0; j < j1; ++j) {
                            c_row[j] += a_val * B(kt, j);
                        }
                    }
                }
            }
        }
    }

    return C;
}

// 逐元素运算
Tensor exp(const Tensor& A) {
    Tensor result(A.rows(), A.cols());
    for (int i = 0; i < A.rows(); ++i) {
        const float* a_row = A.row_ptr(i);
        float* r_row = result.row_ptr(i);
        for (int j = 0; j < A.cols(); ++j) {
            r_row[j] = std::exp(a_row[j]);
        }
    }
    return result;
}

// 按行最大值
Tensor row_max(const Tensor& A) {
    Tensor result(A.rows(), 1);
    for (int i = 0; i < A.rows(); ++i) {
        const float* a_row = A.row_ptr(i);
        float max_val = a_row[0];
        for (int j = 1; j < A.cols(); ++j) {
            if (a_row[j] > max_val) {
                max_val = a_row[j];
            }
        }
        result(i, 0) = max_val;
    }
    return result;
}

// 按行求和
Tensor row_sum(const Tensor& A) {
    Tensor result(A.rows(), 1);
    for (int i = 0; i < A.rows(); ++i) {
        const float* a_row = A.row_ptr(i);
        float sum = 0.0f;
        for (int j = 0; j < A.cols(); ++j) {
            sum += a_row[j];
        }
        result(i, 0) = sum;
    }
    return result;
}

// 缩放
void scale(Tensor& A, float factor) {
    int size = A.data_size();
    for(int i = 0 ; i < size ; ++i){
        auto& val = A.data_ptr()[i];
        val *= factor;
    }
}

}
}
