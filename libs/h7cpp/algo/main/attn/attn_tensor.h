#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cassert>
#include <memory>
#include <functional>
#include <chrono>
#include <random>

namespace h7_attn {
// ===================== Tensor 类实现 =====================
class Tensor {
private:
    std::vector<float> data;   // 一维连续存储，行主序
    int rows_;
    int cols_;

public:
    // 构造函数
    Tensor() : rows_(0), cols_(0) {}

    Tensor(int rows, int cols) : rows_(rows), cols_(cols) {
        data.resize(rows * cols, 0.0f);
    }

    Tensor(int rows, int cols, float value) : rows_(rows), cols_(cols) {
        data.resize(rows * cols, value);
    }

    // 从二维vector构造
    Tensor(const std::vector<std::vector<float>>& vec) {
        rows_ = vec.size();
        if (rows_ > 0) {
            cols_ = vec[0].size();
            data.resize(rows_ * cols_);
            for (int i = 0; i < rows_; ++i) {
                assert(vec[i].size() == cols_);
                std::copy(vec[i].begin(), vec[i].end(), data.begin() + i * cols_);
            }
        } else {
            cols_ = 0;
        }
    }

    // 拷贝构造函数
    Tensor(const Tensor& other)
        : data(other.data), rows_(other.rows_), cols_(other.cols_) {}

    // 移动构造函数
    Tensor(Tensor&& other) noexcept
        : data(std::move(other.data)), rows_(other.rows_), cols_(other.cols_) {
        other.rows_ = 0;
        other.cols_ = 0;
    }

    // 赋值运算符
    Tensor& operator=(const Tensor& other) {
        if (this != &other) {
            data = other.data;
            rows_ = other.rows_;
            cols_ = other.cols_;
        }
        return *this;
    }

    // 移动赋值运算符
    Tensor& operator=(Tensor&& other) noexcept {
        if (this != &other) {
            data = std::move(other.data);
            rows_ = other.rows_;
            cols_ = other.cols_;
            other.rows_ = 0;
            other.cols_ = 0;
        }
        return *this;
    }

    // 获取维度
    int rows() const { return rows_; }
    int cols() const { return cols_; }
    size_t size() const { return data.size(); }

    // 元素访问（行主序）
    float& operator()(int i, int j) {
        assert(i >= 0 && i < rows_ && j >= 0 && j < cols_);
        return data[i * cols_ + j];
    }

    const float& operator()(int i, int j) const {
        assert(i >= 0 && i < rows_ && j >= 0 && j < cols_);
        return data[i * cols_ + j];
    }

    // 获取行指针
    float* row_ptr(int i) {
        assert(i >= 0 && i < rows_);
        return data.data() + i * cols_;
    }

    const float* row_ptr(int i) const {
        assert(i >= 0 && i < rows_);
        return data.data() + i * cols_;
    }

    // 获取原始数据指针
    float* data_ptr() { return data.data(); }
    const float* data_ptr() const { return data.data(); }
    size_t data_size()const{return data.size();}

    // 切片操作（获取连续行）
    Tensor slice_rows(int start, int end) const {
        assert(start >= 0 && end <= rows_ && start < end);
        int num_rows = end - start;
        Tensor result(num_rows, cols_);

        for (int i = 0; i < num_rows; ++i) {
            const float* src = row_ptr(start + i);
            float* dst = result.row_ptr(i);
            std::copy(src, src + cols_, dst);
        }

        return result;
    }

    // 转置
    Tensor transpose() const {
        Tensor result(cols_, rows_);
        for (int i = 0; i < rows_; ++i) {
            for (int j = 0; j < cols_; ++j) {
                result(j, i) = (*this)(i, j);
            }
        }
        return result;
    }

    // 填充值
    void fill(float value) {
        std::fill(data.begin(), data.end(), value);
    }

    // 随机初始化
    void random_init(float min = -1.0f, float max = 1.0f) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(min, max);

        for (float& val : data) {
            val = dis(gen);
        }
    }

    // 打印Tensor
    void print(const std::string& name = "", int max_rows = 5, int max_cols = 5) const {
        if (!name.empty()) {
            std::cout << name << " [" << rows_ << " x " << cols_ << "]:\n";
        }

        int print_rows = std::min(rows_, max_rows);
        int print_cols = std::min(cols_, max_cols);

        for (int i = 0; i < print_rows; ++i) {
            std::cout << "  ";
            for (int j = 0; j < print_cols; ++j) {
                std::cout << (*this)(i, j) << " ";
            }
            if (print_cols < cols_) std::cout << "...";
            std::cout << "\n";
        }
        if (print_rows < rows_) std::cout << "  ...\n";
        std::cout << std::endl;
    }

    // 验证与另一个Tensor是否接近
    bool is_close(const Tensor& other, float rtol = 1e-5, float atol = 1e-8) const {
        if (rows_ != other.rows_ || cols_ != other.cols_) {
            return false;
        }

        for (int i = 0; i < rows_; ++i) {
            for (int j = 0; j < cols_; ++j) {
                float a = (*this)(i, j);
                float b = other(i, j);
                if (std::abs(a - b) > atol + rtol * std::abs(b)) {
                    fprintf(stderr, "delta = %.6f\n", std::abs(a - b));
                    return false;
                }
            }
        }
        return true;
    }
};

}
