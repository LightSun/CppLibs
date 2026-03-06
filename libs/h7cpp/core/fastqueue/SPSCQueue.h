#pragma once

#include <iostream>
#include <cstdint>
#include <atomic>
#include <bitset>
#include <array>

namespace h7 {

//SPSC: single produer with single consumer. Capacity = 2^N
template<typename T, uint64_t Capacity, uint64_t L1_CACHE_LNE = 64>
class SPSCQueue {
    static_assert(sizeof(T) == 8, "Only 64 bit objects are supported");
    static_assert(sizeof(void*) == 8, "The architecture is not 64-bits");
    static_assert((Capacity & (Capacity - 1)) == 0, "RING_BUFFER_SIZE must be a number of contiguous bits set from LSB. Example: 0b00001111 not 0b01001111");
public:
    static constexpr uint64_t MASK = Capacity - 1;

    template<typename... Args>
    void push(Args&&... args) noexcept {
        while (mRingBuffer[mWritePosition & MASK].mObj) if (mReqExit) [[unlikely]] return;
        new(&mRingBuffer[mWritePosition++ & MASK].mObj) T{ std::forward<Args>(args)... };
    }

    inline void pop(T& aOut) noexcept {
        uint64_t current_read;
        while (true) {
            // use relaxed to load current pos，cause here use while
            current_read = mReadPosition.load(std::memory_order_relaxed);
            size_t index = current_read & MASK;
            T& obj = mRingBuffer[index].mObj;

            // use acquire to ensure the producer see the new-data
            std::atomic_thread_fence(std::memory_order_acquire);

            if (obj != nullptr) {
                aOut = obj;
                // set buf to null
                mRingBuffer[index].mObj = nullptr;
                // use release for sync with visit_elements.
                mReadPosition.fetch_add(1, std::memory_order_release);
                return;
            }

            // check exit
            if ((mExitPos == current_read) && mReqExit) [[unlikely]] {
                    aOut = nullptr;
                    return;
                }

            yield();
        }
    }

    //Stop queue (Maybe called from any thread)
    void stopQueue() {
        mExitPos = mWritePosition;
        mReqExit = true;
    }

    template<typename Func>
    void visit_elements(Func&& visitor) const {
        // get the new values
        uint64_t read_pos = mReadPosition.load(std::memory_order_acquire);
        uint64_t write_pos = mWritePosition.load(std::memory_order_acquire);

        //travel
        for (uint64_t pos = read_pos; pos < write_pos; ++pos) {
            uint64_t index = pos & MASK;
            const T& obj = mRingBuffer[index].mObj;
            if (obj != nullptr) {
                visitor(obj);
            }
        }
    }

    static inline void yield(){
#ifdef __x86_64__
        __builtin_ia32_pause(); //_mm_pause
#elif defined(__aarch64__)
        asm volatile("yield" ::: "memory");
#else
        std::this_thread::yield();
#endif
    }

private:
    struct AlignedDataObjects {
        alignas(L1_CACHE_LNE) T mObj = nullptr;
    };
    alignas(L1_CACHE_LNE) volatile std::atomic<uint64_t> mReadPosition = 0;
    alignas(L1_CACHE_LNE) volatile std::atomic<uint64_t> mWritePosition = 0;
    alignas(L1_CACHE_LNE) volatile uint64_t mExitPos = 0;
    alignas(L1_CACHE_LNE) volatile bool mReqExit = false;
    std::array<AlignedDataObjects, Capacity> mRingBuffer;
    alignas(L1_CACHE_LNE) volatile uint8_t mBorderDown[L1_CACHE_LNE]{};
};

}

