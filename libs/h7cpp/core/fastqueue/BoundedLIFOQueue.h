#pragma once

#include <atomic>
#include <cstddef>
#include <new>
#include <type_traits>
#include <array>
#include <functional>

namespace h7 {

//T is pointer
//LIFO
template<typename T, size_t Capacity, size_t LI_CACHE_SIZE = 64>
class BoundedLIFOQueue {
    static_assert(Capacity > 0, "Capacity must be positive");
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power of two for efficient masking");

    using PTR = T*;

    struct alignas(LI_CACHE_SIZE) Slot {
        volatile PTR ptr {nullptr};
    };

public:
    BoundedLIFOQueue(std::function<void(T*)> func_release):func_release_(func_release){
        assert(func_release, "func_release must be valid");
    };
    BoundedLIFOQueue(){
        func_release_ = [](T* p){
            delete p;
        };
    }

    ~BoundedLIFOQueue() {
        size_t h = head_.load(std::memory_order_acquire);
        size_t t = tail_.load(std::memory_order_acquire);
        for (size_t i = h; i < t; ++i) {
            auto p = ptr_at(i);
            if(p){
                func_release_(p);
            }
        }
    }

    // push to tail , if full drop head
    void push(PTR ptr) {
        size_t h = head_.load(std::memory_order_relaxed);
        size_t t = tail_.load(std::memory_order_relaxed);
        size_t count = t - h;

        if (count < Capacity) {
            // not full
            ptr_set(t, ptr);
            tail_.store(t + 1, std::memory_order_release);
            return;
        }

        // may be full，need ensure（avoid consumer pop cause not full）
        h = head_.load(std::memory_order_acquire);
        t = tail_.load(std::memory_order_acquire);
        count = t - h;
        if (count < Capacity) {
            // not full
            ptr_set(t, ptr);
            tail_.store(t + 1, std::memory_order_release);
            return;
        }

        // full drop head
        // head and tail: t == h + Capacity. assign to the same
        // set new
        ptr_set(h, ptr);

        //update head and tail
        head_.store(h + 1, std::memory_order_release);
        tail_.store(t + 1, std::memory_order_release);
    }

    // LIFO
    bool pop(PTR& value) {
        size_t t = tail_.load(std::memory_order_acquire);
        size_t h = head_.load(std::memory_order_acquire);
        if (t == h) {
            return false;
        }
        value = ptr_get_set(t - 1, nullptr);
        if(value == nullptr){
            return false;
        }
        tail_.store(t - 1, std::memory_order_release);
        return true;
    }

    template<typename Func>
    void visit(Func&& visitor) const {
        size_t h = head_.load(std::memory_order_acquire);
        size_t t = tail_.load(std::memory_order_acquire);

        for (size_t i = h; i < t; ++i) {
            PTR p = ptr_at(i);
            if(p){
                visitor(p);
            }
        }
    }
    //may be not the real size. by concurrent
    size_t size() const {
        size_t h = head_.load(std::memory_order_acquire);
        size_t t = tail_.load(std::memory_order_acquire);
        return t - h;
    }

    bool empty() const { return size() == 0; }
    bool full() const { return size() == Capacity; }

    static inline void yield(){
#ifdef __x86_64__
        __builtin_ia32_pause(); //_mm_pause
#elif defined(__aarch64__)
        asm volatile("yield" ::: "memory");
#else
        std::this_thread::yield();
#endif
    }

    BoundedLIFOQueue(const BoundedLIFOQueue&) = delete;
    BoundedLIFOQueue& operator=(const BoundedLIFOQueue&) = delete;

private:
    PTR ptr_at(size_t i) {
        return slots_[i % Capacity].ptr;
    }
    PTR ptr_get_set(size_t i, PTR v) {
        auto& slot = slots_[i % Capacity];
        auto ret = slot.ptr;
        slot.ptr = v;
        return ret;
    }
    const PTR ptr_at(size_t i)const {
        return slots_[i % Capacity].ptr;
    }
    void ptr_set(size_t i, PTR p) {
        auto& slot = slots_[i % Capacity];
        if(slot.ptr != nullptr){
            func_release_(slot.ptr);
        }
        slot.ptr = p;
    }

private:
    alignas(LI_CACHE_SIZE) std::array<Slot, Capacity> slots_;
    alignas(LI_CACHE_SIZE) std::atomic<size_t> head_{0};
    alignas(LI_CACHE_SIZE) std::atomic<size_t> tail_{0};   //idle pos
    std::function<void(T*)> func_release_;
};
}

