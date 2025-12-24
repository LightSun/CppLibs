#pragma once

#include "algo/main/attn/attn_tensor.h"

namespace h7_attn {

// ===================== 在线Softmax类 =====================
class OnlineSoftmax {
private:
    std::vector<float> m;  // 每行的最大值
    std::vector<float> d;  // softmax分母
    int size_;

public:
    OnlineSoftmax(int size) : size_(size) {
        reset(size);
    }

    void reset(int size) {
        size_ = size;
        m.assign(size, -INFINITY);
        d.assign(size, 0.0f);
    }

    // 处理新块
    void process_block(const Tensor& block, int row_offset) {
        int rows = block.rows();
        int cols = block.cols();

        for (int i = 0; i < rows; ++i) {
            int row_idx = row_offset + i;
            const float* block_row = block.row_ptr(i);

            // 计算当前块行的最大值
            float block_max = block_row[0];
            for (int j = 1; j < cols; ++j) {
                if (block_row[j] > block_max) {
                    block_max = block_row[j];
                }
            }

            // 更新全局最大值
            float old_max = m[row_idx];
            float new_max = std::max(old_max, block_max);

            // 更新分母
            if (old_max > -INFINITY) {
                // 调整现有分母
                d[row_idx] *= std::exp(old_max - new_max);
            }

            // 添加新块的贡献
            for (int j = 0; j < cols; ++j) {
                d[row_idx] += std::exp(block_row[j] - new_max);
            }

            m[row_idx] = new_max;
        }
    }

    // 获取softmax结果
    float get_result(int row_idx, float value) const {
        return std::exp(value - m[row_idx]) / d[row_idx];
    }

    // 获取当前统计量
    std::pair<std::vector<float>, std::vector<float>> get_stats() const {
        return {m, d};
    }
};

}
