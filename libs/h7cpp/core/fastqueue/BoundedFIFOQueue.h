#pragma once

#include <atomic>
#include <cstddef>
#include <functional>
#include <new>
#include <type_traits>
#include <vector>
#include <memory>

namespace h7 {

template <typename T, size_t CACHE_LINE_SIZE = 64>
class BoundedFIFOQueue {
    using PTR = T*;

    struct alignas(CACHE_LINE_SIZE) Slot {
        std::shared_ptr<std::atomic<PTR>> ptr;

        Slot(){
            ptr = std::make_shared<std::atomic<PTR>>();
            ptr->store(nullptr, std::memory_order_relaxed);
        }
    };

public:
    explicit BoundedFIFOQueue(size_t capacity,
                              std::function<void(T*)> func_release = nullptr)
        : capacity_(nextPowerOfTwo(capacity)),
        slots_(capacity_),
        func_release_(func_release ? func_release : [](T* p) { delete p; }) {
        mask_ = capacity_ - 1;
    }

    ~BoundedFIFOQueue() {
        size_t h = head_.load(std::memory_order_acquire);
        size_t t = tail_.load(std::memory_order_acquire);
        for (size_t i = h; i < t; ++i) {
            PTR p = slots_[i & mask_].ptr->load(std::memory_order_relaxed);
            if (p) {
                func_release_(p);
            }
        }
    }

    void setDropCallback(void* context, std::function<bool(void*, T*)> func) {
        context_ = context;
        func_drop_ = func;
    }

    void push(PTR ptr){
        while (!try_push(ptr)) {
            yield();
        }
    }

    bool try_push(PTR ptr) {
        if (!ptr) return false;

        while (true) {
            size_t h = head_.load(std::memory_order_acquire);
            size_t t = tail_.load(std::memory_order_acquire);

            // not full, try put to tail
            if (t - h < capacity_) {
                if (tail_.compare_exchange_weak(t, t + 1,
                                                std::memory_order_release,
                                                std::memory_order_relaxed)) {
                    Slot& slot = slots_[t & mask_];
                    PTR expected = nullptr;
                    // 正常情况下槽位应为空，若不为空（极少数情况）则释放旧值
                    if (!slot.ptr->compare_exchange_strong(expected, ptr,
                                                          std::memory_order_release,
                                                          std::memory_order_relaxed)) {
                        // 槽位已被占用（例如消费者过慢导致队列逻辑未及时更新）
                        if (func_drop_ && func_drop_(context_, expected)) {
                            // 已由回调处理
                        } else {
                            func_release_(expected);
                        }
                        slot.ptr->store(ptr, std::memory_order_release);
                    }
                    return true;
                }
                // CAS 失败，重试
                continue;
            }

            // 队列已满：需要覆盖队首元素（head 位置）
            // 尝试原子地增加 head 和 tail（相当于整体滑动一格）
            size_t new_h = h + 1;
            size_t new_t = t + 1;
            // 先尝试更新 head，因为 tail 依赖于 head 的一致性
            if (head_.compare_exchange_weak(h, new_h,
                                            std::memory_order_release,
                                            std::memory_order_relaxed)) {
                // 成功更新 head，现在需要更新 tail
                // 注意：此时 tail 可能已被其他生产者修改，使用 CAS 循环更新
                while (!tail_.compare_exchange_weak(t, new_t,
                                                    std::memory_order_release,
                                                    std::memory_order_relaxed)) {
                    // 若 CAS 失败，重新读取 t 并尝试
                    t = tail_.load(std::memory_order_acquire);
                    new_t = t + 1;
                }
                // 现在 head 和 tail 都向前移动了一格，可以安全地覆盖旧的 head 位置
                Slot& slot = slots_[h & mask_];
                PTR old = slot.ptr->exchange(ptr, std::memory_order_release);
                if (old) {
                    // 处理被覆盖的旧元素
                    if (func_drop_ && func_drop_(context_, old)) {
                        // 已由回调处理
                    } else {
                        func_release_(old);
                    }
                }
                return true;
            }
            // head CAS 失败，重试整个流程
            return false;
        }
    }

    // 单消费者：弹出队首元素
    // 返回 true 并填充 value，false 表示队列空
    bool pop(PTR& value) {
        size_t h = head_.load(std::memory_order_acquire);
        size_t t = tail_.load(std::memory_order_acquire);

        if (h == t) {
            return false;
        }

        Slot& slot = slots_[h & mask_];
        PTR expected = slot.ptr->load(std::memory_order_acquire);
        if (!expected) {
            // can't reach here
            return false;
        }

        if (slot.ptr->compare_exchange_strong(expected, nullptr,
                                             std::memory_order_release,
                                             std::memory_order_relaxed)) {
            value = expected;
            head_.store(h + 1, std::memory_order_release);
            return true;
        }
        return false;
    }

    // 遍历当前队列中的所有元素（仅用于调试，非线程安全）
    template <typename Func>
    void visit(Func&& visitor) const {
        size_t h = head_.load(std::memory_order_acquire);
        size_t t = tail_.load(std::memory_order_acquire);
        for (size_t i = h; i < t; ++i) {
            PTR p = slots_[i & mask_].ptr->load(std::memory_order_acquire);
            if (p) {
                visitor(p);
            }
        }
    }

    size_t size() const {
        size_t h = head_.load(std::memory_order_acquire);
        size_t t = tail_.load(std::memory_order_acquire);
        return t - h;
    }

    bool empty() const { return size() == 0; }
    bool full() const { return size() == capacity_; }

    // 用于忙等待时的 CPU 让步
    static inline void yield() {
#ifdef __x86_64__
        __builtin_ia32_pause();
#else
        std::this_thread::yield();
#endif
    }

    BoundedFIFOQueue(const BoundedFIFOQueue&) = delete;
    BoundedFIFOQueue& operator=(const BoundedFIFOQueue&) = delete;

private:
    static size_t nextPowerOfTwo(size_t n) {
        if (n == 0) return 1;
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n |= n >> 32;
        return n + 1;
    }

private:
    size_t capacity_;
    size_t mask_;
    std::vector<Slot> slots_;

    alignas(CACHE_LINE_SIZE) std::atomic<size_t> head_{0};
    alignas(CACHE_LINE_SIZE) std::atomic<size_t> tail_{0};

    std::function<void(T*)> func_release_;
    std::function<bool(void*, T*)> func_drop_;
    void* context_{nullptr};
};

} // namespace h7
