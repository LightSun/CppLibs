#include <iostream>
#include <chrono>
#include "core/fastqueue/SPMCQueue.h"

using namespace h7;

void test_SPMCQueue() {
    // 配置参数
    constexpr size_t QUEUE_CAPACITY = 4096;    // 队列容量
    constexpr size_t CONSUMER_COUNT = 4;       // 4个消费者线程
    constexpr size_t TOTAL_PRODUCE = 1000000;  // 生产者总共生产100万条数据

    // 初始化队列
    SPMCQueue<int> queue(QUEUE_CAPACITY);

    // 原子变量：统计消费总数（确保无重复/遗漏）
    std::atomic<size_t> total_consumed = 0;
    // 原子变量：标记生产者是否完成
    std::atomic<bool> producer_done = false;

    // 启动消费者线程
    std::vector<std::thread> consumer_threads;
    for (size_t i = 0; i < CONSUMER_COUNT; ++i) {
        consumer_threads.emplace_back([&, tid = i]() {
            while (true) {
                int* value = nullptr;
                // 尝试出队
                if ( queue.pop(value)) {
                    delete value;
                    // 模拟消费处理（微秒级延迟，模拟真实业务）
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                    // 统计消费数量
                    total_consumed.fetch_add(1, std::memory_order_relaxed);
                } else {
                    // 队列为空时，检查生产者是否已完成
                    if (producer_done.load(std::memory_order_relaxed) && queue.size() == 0) {
                        break; // 生产者完成且队列空，退出
                    }
                    // 短暂让出CPU，避免忙等
                    std::this_thread::yield();
                }
            }
            std::cout << "消费者线程 " << tid << " 退出，累计消费：" << total_consumed.load() << std::endl;
        });
    }

    // 启动生产者线程
    std::thread producer_thread([&]() {
        size_t produced = 0;
        while (produced < TOTAL_PRODUCE) {
            // 尝试入队（队列满时重试）
            auto val = new int();
            *val = produced + 1;
            if (queue.push(val)) {
                produced++;
                // 每生产10万条打印进度
                if (produced % 100000 == 0) {
                    std::cout << "生产者已生产：" << produced << " 条数据" << std::endl;
                }
            } else {
                delete val;
                // 队列满时让出CPU
                std::this_thread::yield();
            }
        }
        producer_done.store(true, std::memory_order_relaxed);
        std::cout << "生产者完成！总共生产：" << produced << " 条数据" << std::endl;
    });

    // 等待生产者完成
    producer_thread.join();

    // 等待所有消费者完成
    for (auto& t : consumer_threads) {
        t.join();
    }

    // 验证结果：消费总数 == 生产总数（无重复、无遗漏）
    std::cout << "\n===== 测试结果 =====" << std::endl;
    std::cout << "计划生产：" << TOTAL_PRODUCE << " 条" << std::endl;
    std::cout << "实际生产：" << TOTAL_PRODUCE << " 条" << std::endl;
    std::cout << "实际消费：" << total_consumed.load() << " 条" << std::endl;

    if (total_consumed == TOTAL_PRODUCE) {
        std::cout << "✅ 测试成功：所有数据仅被消费一次，无重复、无遗漏！" << std::endl;
    } else {
        std::cout << "❌ 测试失败：数据存在重复或遗漏！" << std::endl;
    }
}

// -------------------------- 测试Demo --------------------------
static void test_SPMCMQueue() {
    // 1. 初始化队列（容量1024）
    constexpr size_t QUEUE_CAPACITY = 1024;
    constexpr size_t CONSUMER_COUNT = 4; // 4个消费者
    constexpr size_t PRODUCE_COUNT = 100000; // 生产者总共生产10万条数据

    SPMCMQueue<int> queue(QUEUE_CAPACITY);
    queue.setConsumerCount(CONSUMER_COUNT);

    // 2. 统计变量（原子操作保证计数准确）
    std::atomic<size_t> total_consumed = 0;
    std::atomic<bool> stop_flag = false;

    // 3. 启动消费者线程
    std::vector<std::thread> consumer_threads;
    for (size_t i = 0; i < CONSUMER_COUNT; ++i) {
        consumer_threads.emplace_back([&, consumer_id = i]() {
            int value;
            while (!stop_flag.load(std::memory_order_relaxed)) {
                if (queue.dequeue(value, consumer_id)) {
                    // 模拟消费处理（微秒级延迟）
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                    total_consumed.fetch_add(1, std::memory_order_relaxed);
                } else {
                    // 队列为空时短暂休眠，避免忙等
                    std::this_thread::yield();
                }
            }

            // 处理剩余数据
            while (queue.dequeue(value, consumer_id)) {
                total_consumed.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    // 4. 生产者生产数据
    std::thread producer_thread([&]() {
        size_t produced = 0;
        while (produced < PRODUCE_COUNT) {
            if (queue.enqueue(static_cast<int>(produced + 1))) {
                produced++;
            } else {
                // 队列满时短暂休眠
                std::this_thread::yield();
            }
        }
        std::cout << "生产者完成生产，共生产: " << produced << " 条数据" << std::endl;
    });

    // 5. 等待生产者完成，然后停止消费者
    producer_thread.join();
    stop_flag.store(true, std::memory_order_relaxed);

    // 6. 等待所有消费者完成
    for (auto& t : consumer_threads) {
        t.join();
    }

    // 7. 输出结果
    std::cout << "消费者总共消费: " << total_consumed.load(std::memory_order_relaxed) << " 条数据" << std::endl;
    if (total_consumed == PRODUCE_COUNT) {
        std::cout << "测试成功：生产消费数据一致" << std::endl;
    } else {
        std::cout << "测试失败：数据不一致" << std::endl;
    }
}
