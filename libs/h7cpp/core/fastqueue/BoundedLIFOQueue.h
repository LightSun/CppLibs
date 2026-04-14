#pragma once

#include <atomic>
#include <cstddef>
#include <new>
#include <type_traits>
#include <array>
#include <functional>

namespace h7 {

//LIFO
template<typename T, size_t LI_CACHE_SIZE = 64>
class BoundedLIFOQueue {
    using PTR = T*;

    struct alignas(LI_CACHE_SIZE) Slot {
        volatile PTR ptr {nullptr};
    };

public:
    BoundedLIFOQueue(size_t capacity, std::function<void(T*)> func_release)
        :capacity_(nextPowerOfTwo(capacity)),func_release_(func_release){
        //assert(func_release, "func_release must be valid");
        slots_.resize(capacity);
    };
    BoundedLIFOQueue(size_t capacity):capacity_(nextPowerOfTwo(capacity)){
        slots_.resize(capacity);
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
    void setFuncDrop(void* context, std::function<bool(void*, T*)> func){
        context_ = context;
        func_drop_ = func;
    }

    // push to tail , if full drop head
    void push(PTR ptr) {
        size_t h = head_.load(std::memory_order_relaxed);
        size_t t = tail_.load(std::memory_order_relaxed);
        size_t count = t - h;

        if (count < capacity_) {
            // not full
            ptr_set(t, ptr);
            tail_.store(t + 1, std::memory_order_release);
            return;
        }

        // may be full，need ensure（avoid consumer pop cause not full）
        h = head_.load(std::memory_order_acquire);
        t = tail_.load(std::memory_order_acquire);
        count = t - h;
        if (count < capacity_) {
            // not full
            ptr_set(t, ptr);
            tail_.store(t + 1, std::memory_order_release);
            return;
        }

        // full, drop head
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
    bool full() const { return size() == capacity_; }

    static inline void yield(){
#ifdef __x86_64__
        __builtin_ia32_pause(); //_mm_pause
//#elif defined(__aarch64__)
//        asm volatile("yield" ::: "memory");
#else
        std::this_thread::yield();
#endif
    }

    BoundedLIFOQueue(const BoundedLIFOQueue&) = delete;
    BoundedLIFOQueue& operator=(const BoundedLIFOQueue&) = delete;

private:
    PTR ptr_at(size_t i) {
        return slots_[i % capacity_].ptr;
    }
    PTR ptr_get_set(size_t i, PTR v) {
        auto& slot = slots_[i % capacity_];
        auto ret = slot.ptr;
        slot.ptr = v;
        return ret;
    }
    const PTR ptr_at(size_t i)const {
        return slots_[i % capacity_].ptr;
    }
    void ptr_set(size_t i, PTR p) {
        auto& slot = slots_[i % capacity_];
        if(slot.ptr != nullptr){
            if(func_drop_ && func_drop_(context_, slot.ptr)){
                //empty
            }else{
                func_release_(slot.ptr);
            }
        }
        slot.ptr = p;
    }
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
    alignas(LI_CACHE_SIZE) std::vector<Slot> slots_;
    alignas(LI_CACHE_SIZE) std::atomic<size_t> head_{0};
    alignas(LI_CACHE_SIZE) std::atomic<size_t> tail_{0};   //idle pos
    std::function<void(T*)> func_release_;
    std::function<bool(void*, T*)> func_drop_; //return true, means released by func_drop_.
    void* context_ {nullptr};
    int capacity_;
};
}

