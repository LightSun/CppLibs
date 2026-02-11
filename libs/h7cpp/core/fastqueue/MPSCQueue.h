#pragma once

#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <array>
#include <type_traits>
#include <new>

namespace h7 {

//Capacity: 2^N
template<typename T, size_t Capacity, size_t CACHE_LINE_SIZE = 64>
class MPSCQueue {
    static_assert(Capacity > 0, "Capacity must be positive");
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power of two for efficient masking");

    enum class SlotState : uint8_t {
        Empty,
        Writing,
        Ready,
        Reading
    };
    struct alignas(CACHE_LINE_SIZE) Slot{
        union  {
            T data;
            uint8_t storage[sizeof(T)];
        };
        std::atomic<SlotState> state{SlotState::Empty};

        Slot() : storage{} {}
        ~Slot() {
            if (state.load(std::memory_order_relaxed) == SlotState::Ready) {
                data.~T();
            }
        }
    };

public:
    MPSCQueue() {
        for (size_t i = 0; i < Capacity; ++i) {
            m_slots[i].state.store(SlotState::Empty, std::memory_order_relaxed);
        }
    }

    ~MPSCQueue() {
        for (size_t i = 0; i < Capacity; ++i) {
            Slot& slot = m_slots[i];
            SlotState state = slot.state.load(std::memory_order_relaxed);
            if (state == SlotState::Ready) {
                slot.data.~T();
            }
        }
    }

    // non-block-push（multi-producer safe）
    template<typename... Args>
    bool try_push(Args&&... args) noexcept {
        uint64_t write_idx = m_write_idx.load(std::memory_order_relaxed);

        for (;;) {
            Slot& slot = m_slots[write_idx & (Capacity - 1)];
            //if can be write
            SlotState expected = SlotState::Empty;
            if (slot.state.compare_exchange_weak(
                    expected,
                SlotState::Writing,
                    std::memory_order_acquire,
                    std::memory_order_relaxed)) {

                try {
                    new (&slot.data) T(std::forward<Args>(args)...);
                } catch (...) {
                    slot.state.store(SlotState::Empty, std::memory_order_release);
                    return false;
                }

                slot.state.store(SlotState::Ready, std::memory_order_release);
                //can be used by consumer.
                m_write_idx.fetch_add(1, std::memory_order_release);

                return true;
            }

            // no slot for write,check full
            uint64_t read_idx = m_read_idx.load(std::memory_order_acquire);
            uint64_t size = write_idx - read_idx;

            if (size >= Capacity) {
                return false;
            }

            //get next position
            write_idx = m_write_idx.load(std::memory_order_relaxed);
        }
    }

    // may block
    template<typename... Args>
    void push(Args&&... args) {
        while (!try_push(std::forward<Args>(args)...)) {
            yield();
        }
    }

    // never-block, used by single consumer
    bool try_pop(T& value) noexcept {
        // only used by consumer, so relax
        uint64_t read_idx = m_read_idx.load(std::memory_order_relaxed);

        uint64_t write_idx = m_write_idx.load(std::memory_order_acquire);
        if (read_idx == write_idx) {
            return false; // empty
        }

        Slot& slot = m_slots[read_idx & (Capacity - 1)];

        // wait data ready
        while (slot.state.load(std::memory_order_acquire) != SlotState::Ready) {
            yield();
        }

        slot.state.store(SlotState::Reading, std::memory_order_relaxed);

        value = std::move(slot.data);
        slot.data.~T();

        slot.state.store(SlotState::Empty, std::memory_order_release);
        m_read_idx.store(read_idx + 1, std::memory_order_relaxed);

        return true;
    }
    //may block
    bool pop(T& value) {
        while (!try_pop(value)) {
            yield();
            if (m_shutdown.load(std::memory_order_acquire)) {
                return false;
            }
        }
        return true;
    }

    void shutdown() {
        m_shutdown.store(true, std::memory_order_release);
    }

    // may not the real size.
    size_t size() const noexcept {
        uint64_t write_idx = m_write_idx.load(std::memory_order_acquire);
        uint64_t read_idx = m_read_idx.load(std::memory_order_acquire);
        return static_cast<size_t>(write_idx - read_idx);
    }

    bool empty() const noexcept {
        return size() == 0;
    }

    bool full() const noexcept {
        return size() >= Capacity;
    }

public:
    static inline void yield(){
#ifdef __x86_64__
        __builtin_ia32_pause();
#elif defined(__aarch64__)
        asm volatile("yield" ::: "memory");
#else
        std::this_thread::yield();
#endif
    }

private:
    alignas(CACHE_LINE_SIZE) std::array<Slot, Capacity> m_slots;
    alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> m_write_idx{0};
    alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> m_read_idx{0};
    alignas(CACHE_LINE_SIZE) std::atomic<bool> m_shutdown{false};

    MPSCQueue(const MPSCQueue&) = delete;
    MPSCQueue& operator=(const MPSCQueue&) = delete;

    // opt
    MPSCQueue(MPSCQueue&&) = delete;
    MPSCQueue& operator=(MPSCQueue&&) = delete;
};

}
