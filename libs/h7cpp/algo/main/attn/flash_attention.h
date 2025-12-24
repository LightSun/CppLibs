#pragma once

#include "algo/main/attn/attn_ops.h"
#include "algo/main/attn/attn_online_softmax.h"

namespace h7_attn {

// ===================== FlashAttention 实现 =====================
class FlashAttention {
private:
    int block_size_q;  // 查询分块大小
    int block_size_kv; // 键值分块大小

public:
    FlashAttention(int block_size_q = 64, int block_size_kv = 64)
        : block_size_q(block_size_q), block_size_kv(block_size_kv) {}

    // 前向传播（修正版）
    Tensor forward(const Tensor& Q, const Tensor& K, const Tensor& V) {
         auto start = std::chrono::high_resolution_clock::now();
         //
        int N = Q.rows();      // 序列长度
        int d = Q.cols();      // 查询/键维度
        int d_v = V.cols();    // 值维度
        float scale = 1.0f / std::sqrt(static_cast<float>(d));

        // 验证维度
        assert(K.rows() == N && K.cols() == d);
        assert(V.rows() == N);

        // 输出矩阵 O，以及softmax统计量
        Tensor O(N, d_v, 0.0f);
        std::vector<float> m(N, -INFINITY);  // 每行的最大值
        std::vector<float> l(N, 0.0f);       // softmax分母（已缩放）

        // FlashAttention核心：分块处理
        // 外层循环：查询块
        for (int i_start = 0; i_start < N; i_start += block_size_q) {
            int i_end = std::min(i_start + block_size_q, N);
            int block_rows = i_end - i_start;

            // 内层循环：键值块
            for (int j_start = 0; j_start < N; j_start += block_size_kv) {
                int j_end = std::min(j_start + block_size_kv, N);
                int block_cols = j_end - j_start;

                // 1. 计算当前块的注意力分数
                Tensor S_block = compute_attention_block(Q, K, i_start, i_end,
                                                         j_start, j_end, scale);

                // 2. 更新在线softmax和输出
                update_online_softmax(S_block, V, O, m, l,
                                     i_start, i_end, j_start, j_end);
            }
        }

        // 3. 最终归一化输出
        for (int i = 0; i < N; ++i) {
            if (l[i] > 0) {
                float norm = 1.0f / l[i];
                for (int j = 0; j < d_v; ++j) {
                    O(i, j) *= norm;
                }
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "FlashAttention 前向传播耗时: " << duration.count() / 1000.0 << " ms" << std::endl;

        return O;
    }

private:
    // 计算一个块的注意力分数
    Tensor compute_attention_block(const Tensor& Q, const Tensor& K,
                                   int i_start, int i_end,
                                   int j_start, int j_end,
                                   float scale) {
        int block_rows = i_end - i_start;
        int block_cols = j_end - j_start;
        int d = Q.cols();

        Tensor S(block_rows, block_cols);

        // 计算 Q[i_start:i_end] @ K[j_start:j_end]^T
        for (int i = 0; i < block_rows; ++i) {
            int q_idx = i_start + i;
            for (int j = 0; j < block_cols; ++j) {
                int k_idx = j_start + j;
                float dot = 0.0f;
                for (int k = 0; k < d; ++k) {
                    dot += Q(q_idx, k) * K(k_idx, k);
                }
                S(i, j) = dot * scale;
            }
        }

        return S;
    }

    // 更新在线softmax（核心算法，修正版）
    void update_online_softmax(const Tensor& S_block, const Tensor& V,
                               Tensor& O, std::vector<float>& m,
                               std::vector<float>& l,
                               int i_start, int i_end,
                               int j_start, int j_end) {
        int block_rows = i_end - i_start;
        int block_cols = j_end - j_start;
        int d_v = V.cols();

        for (int i = 0; i < block_rows; ++i) {
            int row_idx = i_start + i;

            // 计算当前块行的最大值
            float block_max = S_block(i, 0);
            for (int j = 1; j < block_cols; ++j) {
                if (S_block(i, j) > block_max) {
                    block_max = S_block(i, j);
                }
            }

            // 更新全局最大值
            float old_max = m[row_idx];
            float new_max = std::max(old_max, block_max);

            // 计算当前块的指数值（相对于new_max）
            std::vector<float> exp_vals(block_cols);
            float block_l = 0.0f;
            for (int j = 0; j < block_cols; ++j) {
                exp_vals[j] = std::exp(S_block(i, j) - new_max);
                block_l += exp_vals[j];
            }

            // 关键修正：正确的在线softmax更新公式
            // l_new = l_old * exp(old_max - new_max) + block_l
            float exp_old_to_new = (old_max > -INFINITY) ? std::exp(old_max - new_max) : 0.0f;
            float l_new = l[row_idx] * exp_old_to_new + block_l;

            // 更新输出（需要调整现有输出）
            if (exp_old_to_new > 0) {
                // 调整现有输出：O_old = O_old * exp(old_max - new_max)
                float scale = exp_old_to_new;
                for (int k = 0; k < d_v; ++k) {
                    O(row_idx, k) *= scale;
                }
            }

            // 添加当前块的贡献
            for (int j = 0; j < block_cols; ++j) {
                int v_idx = j_start + j;
                float weight = exp_vals[j];
                for (int k = 0; k < d_v; ++k) {
                    O(row_idx, k) += weight * V(v_idx, k);
                }
            }

            // 更新统计量
            m[row_idx] = new_max;
            l[row_idx] = l_new;
        }
    }
};

// ===================== FlashAttention-2 优化实现 =====================
class FlashAttention2 {
private:
    int block_size;

public:
    FlashAttention2(int block_size = 64) : block_size(block_size) {}

    Tensor forward(const Tensor& Q, const Tensor& K, const Tensor& V) {
        auto start = std::chrono::high_resolution_clock::now();

        int N = Q.rows();
        int d = Q.cols();
        int d_v = V.cols();
        float scale = 1.0f / std::sqrt(static_cast<float>(d));

        // 输出和统计量
        Tensor O(N, d_v, 0.0f);
        Tensor L(N, 1, 0.0f);      // softmax分母
        Tensor M(N, 1, -INFINITY); // 最大值

        // FlashAttention-2优化：先遍历查询块
        for (int i1 = 0; i1 < N; i1 += block_size) {
            int i2 = std::min(i1 + block_size, N);

            Tensor Q_i = Q.slice_rows(i1, i2);
            Tensor O_i = O.slice_rows(i1, i2);
            Tensor L_i = L.slice_rows(i1, i2);
            Tensor M_i = M.slice_rows(i1, i2);

            // 遍历键值块
            for (int j1 = 0; j1 < N; j1 += block_size) {
                int j2 = std::min(j1 + block_size, N);

                Tensor K_j = K.slice_rows(j1, j2);
                Tensor V_j = V.slice_rows(j1, j2);

                // 计算注意力分数
                Tensor K_j_T = K_j.transpose();
                Tensor S_ij = tensor_ops::matmul(Q_i, K_j_T);

                // 应用缩放
                for (int i = 0; i < S_ij.rows(); ++i) {
                    float* s_row = S_ij.row_ptr(i);
                    for (int j = 0; j < S_ij.cols(); ++j) {
                        s_row[j] *= scale;
                    }
                }

                // 更新统计量
                update_stats_fast(S_ij, O_i, L_i, M_i, V_j);
            }

            // 最终归一化（FlashAttention-2优化：只在最后归一化一次）
            for (int i = 0; i < O_i.rows(); ++i) {
                float norm = 1.0f / L_i(i, 0);
                float* o_row = O_i.row_ptr(i);
                for (int j = 0; j < O_i.cols(); ++j) {
                    o_row[j] *= norm;
                }
            }

            // 保存回原矩阵
            for (int i = 0; i < O_i.rows(); ++i) {
                float* dst = O.row_ptr(i1 + i);
                const float* src = O_i.row_ptr(i);
                std::copy(src, src + d_v, dst);

                L(i1 + i, 0) = L_i(i, 0);
                M(i1 + i, 0) = M_i(i, 0);
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "FlashAttention-2前向传播耗时: " << duration.count() / 1000.0 << " ms" << std::endl;

        return O;
    }

private:
    // 快速更新统计量
    void update_stats_fast(Tensor& S, Tensor& O, Tensor& L, Tensor& M, const Tensor& V) {
        int B_r = S.rows();
        int B_c = S.cols();
        int d_v = O.cols();

        for (int i = 0; i < B_r; ++i) {
            float* s_row = S.row_ptr(i);
            float* o_row = O.row_ptr(i);
            float& l_val = L(i, 0);
            float& m_val = M(i, 0);

            // 找到当前块行的最大值
            float block_max = s_row[0];
            for (int j = 1; j < B_c; ++j) {
                if (s_row[j] > block_max) {
                    block_max = s_row[j];
                }
            }

            // 更新全局最大值
            float old_max = m_val;
            float new_max = std::max(old_max, block_max);

            // 预计算缩放因子
            float scale_old = (old_max > -INFINITY) ? std::exp(old_max - new_max) : 0.0f;

            // 计算exp并累加
            float block_l = 0.0f;
            for (int j = 0; j < B_c; ++j) {
                s_row[j] = std::exp(s_row[j] - new_max);
                block_l += s_row[j];
            }

            // 更新分母
            l_val = l_val * scale_old + block_l;

            // 更新输出
            if (scale_old > 0) {
                for (int k = 0; k < d_v; ++k) {
                    o_row[k] *= scale_old;
                }
            }

            for (int j = 0; j < B_c; ++j) {
                float weight = s_row[j];
                const float* v_row = V.row_ptr(j);

                for (int k = 0; k < d_v; ++k) {
                    o_row[k] += weight * v_row[k];
                }
            }

            // 更新最大值
            m_val = new_max;
        }
    }
};

// ===================== 标准注意力（用于对比） =====================
class StandardAttention {
public:
    Tensor forward(const Tensor& Q, const Tensor& K, const Tensor& V) {
        auto start = std::chrono::high_resolution_clock::now();

        int N = Q.rows();
        int d = Q.cols();
        float scale = 1.0f / std::sqrt(static_cast<float>(d));

        // 计算 Q * (K^T)
        Tensor K_T = K.transpose();
        Tensor scores = tensor_ops::matmul(Q, K_T);

        // 应用缩放
        tensor_ops::scale(scores, scale);

        // 计算softmax
        Tensor attention = softmax(scores);

        // 乘以V
        Tensor result = tensor_ops::matmul(attention, V);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "标准注意力耗时: " << duration.count() / 1000.0 << " ms" << std::endl;

        return result;
    }

private:
    Tensor softmax(const Tensor& scores) {
        int rows = scores.rows();
        int cols = scores.cols();
        Tensor result(rows, cols);

        for (int i = 0; i < rows; ++i) {
            const float* s_row = scores.row_ptr(i);
            float* r_row = result.row_ptr(i);

            // 找最大值（数值稳定性）
            float max_val = s_row[0];
            for (int j = 1; j < cols; ++j) {
                if (s_row[j] > max_val) {
                    max_val = s_row[j];
                }
            }

            // 计算指数和
            float exp_sum = 0.0f;
            for (int j = 0; j < cols; ++j) {
                r_row[j] = std::exp(s_row[j] - max_val);
                exp_sum += r_row[j];
            }

            // 归一化
            for (int j = 0; j < cols; ++j) {
                r_row[j] /= exp_sum;
            }
        }

        return result;
    }
};

void test_attention_impl();
void benchmark_attention();
}

void test_attn_all();
