#pragma once

#include <atomic>
#include <cstddef>
#include <functional>
#include <mutex>
#include <vector>

namespace h7 {

// 有界 FIFO 队列，多生产者多消费者，满时覆盖队首元素
template <typename T>
class BoundedFIFOQueueMPMC {
    using PTR = T*;

public:
    // capacity: 队列最大容量
    // func_release: 释放元素的回调，默认 delete
    explicit BoundedFIFOQueueMPMC(size_t capacity,
                                  std::function<void(T*)> func_release = nullptr)
        : capacity_(capacity),
        buffer_(capacity, nullptr),
        func_release_(func_release ? func_release : [](T* p) { delete p; }) {}

    ~BoundedFIFOQueueMPMC() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (size_t i = 0; i < size_; ++i) {
            size_t idx = (head_ + i) % capacity_;
            if (buffer_[idx]) {
                func_release_(buffer_[idx]);
            }
        }
    }

    // 设置丢弃回调（当队列满覆盖旧元素时调用）
    void setDropCallback(void* context, std::function<bool(void*, T*)> func) {
        std::lock_guard<std::mutex> lock(mutex_);
        context_ = context;
        func_drop_ = func;
    }

    // 生产者：将元素推入队列
    // 队列满时自动覆盖队首元素，被覆盖的元素通过回调处理
    void push(PTR ptr) {
        if (!ptr) return;
        std::lock_guard<std::mutex> lock(mutex_);

        if (size_ < capacity_) {
            // 未满：直接添加到尾部
            size_t tail_idx = (head_ + size_) % capacity_;
            buffer_[tail_idx] = ptr;
            ++size_;
        } else {
            // 已满：覆盖 head 位置
            size_t head_idx = head_ % capacity_;
            PTR old = buffer_[head_idx];
            buffer_[head_idx] = ptr;
            // 处理被覆盖的旧元素
            if (func_drop_ && func_drop_(context_, old)) {
                // 已由回调处理
            } else {
                func_release_(old);
            }
            // head 向前移动一位，size 保持不变
            head_ = (head_ + 1) % capacity_;
        }
    }

    // 消费者：弹出队首元素
    // 返回 true 并填充 value，false 表示队列空
    bool pop(PTR& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (size_ == 0) {
            return false;
        }
        size_t head_idx = head_ % capacity_;
        value = buffer_[head_idx];
        buffer_[head_idx] = nullptr;
        head_ = (head_ + 1) % capacity_;
        --size_;
        return true;
    }

    // 遍历当前队列中的所有元素（仅调试用，持有锁）
    template <typename Func>
    void visit(Func&& visitor) const {
        std::lock_guard<std::mutex> lock(mutex_);
        for (size_t i = 0; i < size_; ++i) {
            size_t idx = (head_ + i) % capacity_;
            if (buffer_[idx]) {
                visitor(buffer_[idx]);
            }
        }
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return size_;
    }

    bool empty() const { return size() == 0; }
    bool full() const { return size() == capacity_; }

    BoundedFIFOQueueMPMC(const BoundedFIFOQueueMPMC&) = delete;
    BoundedFIFOQueueMPMC& operator=(const BoundedFIFOQueueMPMC&) = delete;

private:
    mutable std::mutex mutex_;
    const size_t capacity_;
    std::vector<PTR> buffer_;   // 环形缓冲区
    size_t head_ = 0;           // 队首索引（逻辑位置）
    size_t size_ = 0;           // 当前元素个数

    std::function<void(T*)> func_release_;
    std::function<bool(void*, T*)> func_drop_;
    void* context_ = nullptr;
};

} // namespace h7
