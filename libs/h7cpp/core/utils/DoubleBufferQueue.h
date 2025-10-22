#pragma once

#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <thread>

namespace h7 {

template<typename T>
class DoubleBufferQueue {
private:
    std::vector<T> buffers[2];  // 双缓冲区
    std::atomic<int> frontIndex; // 前端缓冲区索引
    std::atomic<int> backIndex;  // 后端缓冲区索引
    std::atomic<bool> swapping;  // 交换标志

    std::mutex swapMutex;        // 交换互斥锁
    std::condition_variable swapCV; // 交换条件变量

public:
    DoubleBufferQueue(size_t bufferSize = 1024)
        : frontIndex(0), backIndex(1), swapping(false) {
        buffers[0].reserve(bufferSize);
        buffers[1].reserve(bufferSize);
    }

    // 生产者接口：向前端缓冲区添加数据
    void push(const T& item) {
        int currentFront = frontIndex.load(std::memory_order_acquire);
        buffers[currentFront].push_back(item);
    }

    // 生产者接口：批量添加数据
    template<typename InputIt>
    void pushRange(InputIt first, InputIt last) {
        int currentFront = frontIndex.load(std::memory_order_acquire);
        buffers[currentFront].insert(buffers[currentFront].end(), first, last);
    }

    // 消费者接口：交换缓冲区
    bool swapBuffers() {
        if (swapping.exchange(true, std::memory_order_acq_rel)) {
            return false; // 已经在交换中
        }

        std::unique_lock<std::mutex> lock(swapMutex);

        // 交换前后端缓冲区索引
        int oldFront = frontIndex.load(std::memory_order_acquire);
        int oldBack = backIndex.load(std::memory_order_acquire);

        frontIndex.store(oldBack, std::memory_order_release);
        backIndex.store(oldFront, std::memory_order_release);

        // 清空新的前端缓冲区（原后端缓冲区）
        buffers[oldBack].clear();

        swapping.store(false, std::memory_order_release);
        swapCV.notify_all();

        return true;
    }

    const std::vector<T>& getBackBuffer() const {
        return buffers[backIndex.load(std::memory_order_acquire)];
    }

    void waitForSwap() {
        std::unique_lock<std::mutex> lock(swapMutex);
        swapCV.wait(lock, [this]() {
            return !swapping.load(std::memory_order_acquire);
        });
    }
    size_t frontSize() const {
        return buffers[frontIndex.load(std::memory_order_acquire)].size();
    }
    size_t backSize() const {
        return buffers[backIndex.load(std::memory_order_acquire)].size();
    }
    void clear() {
        waitForSwap();
        buffers[0].clear();
        buffers[1].clear();
    }
};
}
/*
void producerConsumerExample() {
    DoubleBufferQueue<int> queue(1000);

    // 生产者线程
    std::thread producer([&queue]() {
        for (int i = 0; i < 100; ++i) {
            queue.push(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

            // 批量添加示例
            if (i % 10 == 0) {
                std::vector<int> batch = {i+1, i+2, i+3};
                queue.pushRange(batch.begin(), batch.end());
            }
        }
    });

    // 消费者线程
    std::thread consumer([&queue]() {
        for (int i = 0; i < 10; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // 交换缓冲区并处理数据
            if (queue.swapBuffers()) {
                const auto& data = queue.getBackBuffer();
                std::cout << "Consumer processed " << data.size() << " items: ";
                for (const auto& item : data) {
                    std::cout << item << " ";
                }
                std::cout << std::endl;
            }
        }
    });

    producer.join();
    consumer.join();
}

int main() {
    producerConsumerExample();
    return 0;
}
*/
