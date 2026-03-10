#pragma once

#include <atomic>
#include <vector>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <thread>

namespace h7 {

template <typename T, size_t CACHE_LINE_SIZE = 64>
class SPMCQueue {
public:
    explicit SPMCQueue(size_t capacity)
        : capacity_(nextPowerOfTwo(capacity)),
        mask_(capacity_ - 1){
        buffer_.resize(capacity_);
        if (capacity_ < 2) {
            throw std::invalid_argument("capacity_ < 2");
        }
        // init
        write_idx_.store(0, std::memory_order_relaxed);
        read_idx_.store(0, std::memory_order_relaxed);
    }

    ~SPMCQueue() {
        for(auto& t: buffer_){
            if(t.ptr){
                delete t.ptr;
                t.ptr = nullptr;
            }
        }
    }

    //
    SPMCQueue(const SPMCQueue&) = delete;
    SPMCQueue& operator=(const SPMCQueue&) = delete;
    SPMCQueue(SPMCQueue&&) = delete;
    SPMCQueue& operator=(SPMCQueue&&) = delete;

    bool enqueue(T* value) {
        const uint64_t current_write = write_idx_.load(std::memory_order_relaxed);
        const uint64_t next_write = current_write + 1;

        if (next_write - read_idx_.load(std::memory_order_acquire) > capacity_) {
            return false;
        }
        //already have data
        if(buffer_[next_write & mask_].ptr != nullptr){
            return false;
        }
        buffer_[next_write & mask_].ptr = value;
        write_idx_.store(next_write, std::memory_order_release);
        return true;
    }

    T* dequeue() {
        uint64_t current_read = read_idx_.load(std::memory_order_relaxed);
        uint64_t current_write = write_idx_.load(std::memory_order_acquire);
        // empty
        if (current_read == current_write) {
            return nullptr;
        }

        const uint64_t next_read = current_read + 1;
        // CAS to read
        if (!read_idx_.compare_exchange_weak(
                current_read, next_read,
                std::memory_order_release,  // success-memory-order： update read index
                std::memory_order_relaxed   // fail-memory-order: no need
                )) {
            return nullptr; //CAS failed
        }
        auto value = buffer_[next_read & mask_].ptr;
        buffer_[next_read & mask_].ptr = nullptr;
        return value;
    }

    size_t size() const {
        uint64_t w = write_idx_.load(std::memory_order_relaxed);
        uint64_t r = read_idx_.load(std::memory_order_relaxed);
        return w - r;
    }
    size_t capacity() const { return capacity_; }

private:
    struct Item{
        alignas(CACHE_LINE_SIZE) T* ptr {nullptr};
    };
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
    alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> write_idx_ = 0;
    alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> read_idx_ = 0;
    alignas(CACHE_LINE_SIZE) size_t capacity_ = 0;
    alignas(CACHE_LINE_SIZE) size_t mask_ = 0;
    alignas(CACHE_LINE_SIZE) std::vector<Item> buffer_;
};

//one data can be consume by every consumer
template <typename T, size_t CACHE_LINE_SIZE = 64>
class SPMCMQueue {
public:
    explicit SPMCMQueue(size_t capacity)
        : capacity_(nextPowerOfTwo(capacity)),
        mask_(capacity_ - 1),
        buffer_(new T[capacity_]) {
        if (capacity_ < 2) {
            throw std::invalid_argument("capacity_ < 2");
        }
        memset(buffer_, 0, sizeof(T) * capacity_);
        // init
        producer_idx_.store(0, std::memory_order_relaxed);
        for (auto& consumer_idx : consumer_idx_) {
            consumer_idx.store(0, std::memory_order_relaxed);
        }
    }

    ~SPMCMQueue() {
        delete[] buffer_;
    }

    //
    SPMCMQueue(const SPMCMQueue&) = delete;
    SPMCMQueue& operator=(const SPMCMQueue&) = delete;
    SPMCMQueue(SPMCMQueue&&) = delete;
    SPMCMQueue& operator=(SPMCMQueue&&) = delete;

    //single producer
    bool enqueue(const T& value) {
        const uint64_t current_producer = producer_idx_.load(std::memory_order_relaxed);
        const uint64_t next_producer = current_producer + 1;

        // check full
        uint64_t min_consumer = consumer_idx_[0].load(std::memory_order_acquire);
        for (size_t i = 1; i < consumer_idx_.size(); ++i) {
            min_consumer = std::min(min_consumer, consumer_idx_[i].load(std::memory_order_acquire));
        }

        // full：producer_index - slowest_consumer_index >= cap
        if (next_producer - min_consumer > capacity_) {
            return false;
        }

        buffer_[next_producer & mask_] = value;
        // update producer index
        producer_idx_.store(next_producer, std::memory_order_release);
        return true;
    }

    bool dequeue(T& value, size_t consumer_id) {
        if (consumer_id >= consumer_idx_.size()) {
            throw std::invalid_argument("consumer_id is wrong");
        }
        uint64_t current_consumer = consumer_idx_[consumer_id].load(std::memory_order_relaxed);
        const uint64_t current_producer = producer_idx_.load(std::memory_order_acquire);

        // queue -> empty
        if (current_consumer == current_producer) {
            return false;
        }
        //
        value = buffer_[(current_consumer + 1) & mask_];
        // update consumer index
        consumer_idx_[consumer_id].store(current_consumer + 1, std::memory_order_release);
        return true;
    }
    void setConsumerCount(size_t count) {
        consumer_idx_.resize(count);
        for (size_t i = 0; i < count; ++i) {
            consumer_idx_[i].store(0, std::memory_order_relaxed);
        }
    }
    size_t capacity() const { return capacity_; }

private:
    // 2 ^ K
    struct ConsumerItem{
        std::unique_ptr<std::atomic<uint64_t>> idx;

        ConsumerItem(){
            idx = std::make_unique<std::atomic<uint64_t>>();
        }
        void store(uint64_t id, std::memory_order m){
            idx->store(id, m);
        }
        uint64_t load(std::memory_order m){
            return idx->load(m);
        }
    };
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
    alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> producer_idx_ = 0;
    alignas(CACHE_LINE_SIZE) std::vector<ConsumerItem> consumer_idx_;
    alignas(CACHE_LINE_SIZE) size_t capacity_ = 0;
    alignas(CACHE_LINE_SIZE) size_t mask_ = 0;
    alignas(CACHE_LINE_SIZE) T* buffer_;
};

}
