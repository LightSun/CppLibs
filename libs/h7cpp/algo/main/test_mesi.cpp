#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

namespace mesi {

// 简化的MESI状态
enum CacheState { INVALID, SHARED, EXCLUSIVE, MODIFIED };

// 缓存行结构
struct CacheLine {
    std::atomic<int> state;
    std::atomic<int> data;
    std::atomic<int> tag;

    CacheLine() : state(INVALID), data(0), tag(-1) {}

    CacheLine(const CacheLine& c){
        auto s1 = c.state.load(std::memory_order_acquire);
        auto d1 = c.data.load(std::memory_order_acquire);
        auto t1 = c.tag.load(std::memory_order_acquire);
        state.store(s1, std::memory_order_relaxed);
        data.store(d1, std::memory_order_relaxed);
        tag.store(t1, std::memory_order_relaxed);
    }
};

// 模拟多核处理器
class MultiCoreProcessor {
private:
    std::vector<std::vector<CacheLine>> L1_cache;  // 4个核的私有L1缓存
    std::vector<CacheLine> L2_shared;    // 共享L2缓存
    std::mutex bus_mutex;                // 模拟总线仲裁
    std::atomic<int> memory[1024];       // 主内存

public:
    MultiCoreProcessor() {
        // 初始化缓存
        L1_cache.resize(4);
        for (int i = 0; i < 4; i++){
            L1_cache[i].resize(8);  // 每个核8个缓存行
        }
        L2_shared.resize(32);  // 共享L2缓存32行

        // 初始化内存
        for (int i = 0; i < 1024; i++) {
            memory[i].store(i * 10, std::memory_order_relaxed);
        }
    }

    // 总线事务处理器
    void busTransaction(int core_id, int address, char operation, int* value = nullptr) {
        std::lock_guard<std::mutex> lock(bus_mutex);  // 模拟总线仲裁

        std::cout << "Core " << core_id << " issues bus "
                  << (operation == 'R' ? "READ" : "WRITE")
                  << " for address " << address << std::endl;

        // 通知其他核心进行监听
        for (int i = 0; i < 4; i++) {
            if (i != core_id) {
                snoopCache(i, address, operation, core_id);
            }
        }

        // 处理内存访问
        if (operation == 'R') {
            if (value) {
                *value = memory[address].load(std::memory_order_relaxed);
            }
        } else if (operation == 'W') {
            if (value) {
                memory[address].store(*value, std::memory_order_relaxed);
            }
        }
    }

    // 缓存监听
    void snoopCache(int core_id, int address, char operation, int requestor_id) {
        for (auto& line : L1_cache[core_id]) {
            if (line.tag.load(std::memory_order_acquire) == address &&
                line.state.load(std::memory_order_acquire) != INVALID) {

                int old_state = line.state.load(std::memory_order_acquire);

                if (operation == 'R') {  // 总线读请求
                    if (old_state == MODIFIED) {
                        // 写回数据到内存
                        memory[address].store(line.data.load(std::memory_order_relaxed),
                                             std::memory_order_relaxed);
                        line.state.store(SHARED, std::memory_order_release);
                        std::cout << "  Core " << core_id << ": M->S (write back)" << std::endl;
                    } else if (old_state == EXCLUSIVE) {
                        line.state.store(SHARED, std::memory_order_release);
                        std::cout << "  Core " << core_id << ": E->S" << std::endl;
                    }
                } else if (operation == 'W') {  // 总线写请求（读无效）
                    line.state.store(INVALID, std::memory_order_release);
                    std::cout << "  Core " << core_id << ": "
                              << stateToString(old_state) << "->I" << std::endl;
                }
                break;
            }
        }
    }

    // 读取数据（模拟线程读取共享变量）
    int read(int core_id, int address) {
        // 查找L1缓存
        for (auto& line : L1_cache[core_id]) {
            if (line.tag.load(std::memory_order_acquire) == address) {
                auto state = line.state.load(std::memory_order_acquire);
                if (state != INVALID) {
                    // 缓存命中
                    int data = line.data.load(std::memory_order_acquire);
                    std::cout << "Core " << core_id << " READ hit: addr=" << address
                              << ", state=" << stateToString(state)
                              << ", data=" << data << std::endl;
                    return data;
                }
            }
        }

        // 缓存未命中，发起总线事务
        int value;
        busTransaction(core_id, address, 'R', &value);

        // 分配缓存行
        allocateCacheLine(core_id, address, value, EXCLUSIVE);

        std::cout << "Core " << core_id << " READ miss: loaded " << value
                  << " from memory" << std::endl;
        return value;
    }

    // 写入数据（模拟线程写入共享变量）
    void write(int core_id, int address, int value) {
        // 查找L1缓存
        for (auto& line : L1_cache[core_id]) {
            if (line.tag.load(std::memory_order_acquire) == address) {
                auto state = line.state.load(std::memory_order_acquire);

                if (state == MODIFIED || state == EXCLUSIVE) {
                    // 可以本地写入
                    line.data.store(value, std::memory_order_release);
                    line.state.store(MODIFIED, std::memory_order_release);

                    std::cout << "Core " << core_id << " WRITE hit: "
                              << stateToString(state) << "->M" << std::endl;
                    return;
                } else if (state == SHARED) {
                    // 需要升级到MODIFIED，发送总线无效
                    busTransaction(core_id, address, 'W');
                    line.data.store(value, std::memory_order_release);
                    line.state.store(MODIFIED, std::memory_order_release);

                    std::cout << "Core " << core_id << " WRITE to shared: S->M" << std::endl;
                    return;
                }
            }
        }

        // 写未命中，写分配策略
        busTransaction(core_id, address, 'W');
        allocateCacheLine(core_id, address, value, MODIFIED);

        std::cout << "Core " << core_id << " WRITE miss: alloc new line in M state" << std::endl;
    }

    // 分配缓存行
    void allocateCacheLine(int core_id, int address, int value, CacheState state) {
        // 简单策略：替换第一个无效行，否则替换第一个
        for (auto& line : L1_cache[core_id]) {
            if (line.state.load(std::memory_order_acquire) == INVALID) {
                line.tag.store(address, std::memory_order_release);
                line.data.store(value, std::memory_order_release);
                line.state.store(state, std::memory_order_release);
                return;
            }
        }

        // 如果没有无效行，替换第一个
        auto& line = L1_cache[core_id][0];

        // 如果被替换的行是MODIFIED，需要写回
        if (line.state.load(std::memory_order_acquire) == MODIFIED) {
            memory[line.tag.load(std::memory_order_relaxed)].store(
                line.data.load(std::memory_order_relaxed),
                std::memory_order_relaxed);
            std::cout << "  Write back modified line: addr="
                      << line.tag.load(std::memory_order_relaxed) << std::endl;
        }

        line.tag.store(address, std::memory_order_release);
        line.data.store(value, std::memory_order_release);
        line.state.store(state, std::memory_order_release);
    }

    std::string stateToString(int state) {
        switch (state) {
        case MODIFIED: return "M";
        case EXCLUSIVE: return "E";
        case SHARED: return "S";
        case INVALID: return "I";
        default: return "?";
        }
    }

    // 显示缓存状态
    void displayCacheState(int core_id) {
        std::cout << "\nCore " << core_id << " L1 Cache:" << std::endl;
        for (size_t i = 0; i < L1_cache[core_id].size(); i++) {
            auto& line = L1_cache[core_id][i];
            auto state = line.state.load(std::memory_order_acquire);
            if (state != INVALID) {
                std::cout << "  Line " << i << ": tag=" << line.tag.load(std::memory_order_relaxed)
                          << ", state=" << stateToString(state)
                          << ", data=" << line.data.load(std::memory_order_relaxed) << std::endl;
            }
        }
    }
};

// 共享计数器（模拟多线程共享变量）
class SharedCounter {
private:
    MultiCoreProcessor& proc;
    std::atomic<int> counter_addr;  // 计数器内存地址

public:
    SharedCounter(MultiCoreProcessor& p, int addr) : proc(p), counter_addr(addr) {
        // 初始化计数器
        proc.write(0, addr, 0);  // 假设由核心0初始化
    }

    // 线程函数：增加计数器
    void increment(int core_id, int times) {
        for (int i = 0; i < times; i++) {
            // 读取-修改-写入序列
            int value = proc.read(core_id, counter_addr);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));  // 模拟处理时间
            proc.write(core_id, counter_addr, value + 1);
        }
    }

    int getValue() {
        return proc.read(0, counter_addr);  // 由核心0读取最终值
    }
};

// 测试1：多线程计数器（有缓存一致性问题）
void testCounter() {
    std::cout << "=== Test 1: Multi-threaded Counter ===" << std::endl;

    MultiCoreProcessor proc;
    SharedCounter counter(proc, 100);  // 计数器在地址100

    std::thread threads[4];

    // 启动4个线程，每个增加5次
    for (int i = 0; i < 4; i++) {
        threads[i] = std::thread(&SharedCounter::increment, &counter, i, 5);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "\nFinal counter value: " << counter.getValue() << std::endl;
    std::cout << "Expected: 20" << std::endl;

    // 显示最终缓存状态
    for (int i = 0; i < 4; i++) {
        proc.displayCacheState(i);
    }
}

// 测试2：虚假共享（False Sharing）问题
void testFalseSharing() {
    std::cout << "\n\n=== Test 2: False Sharing Problem ===" << std::endl;

    MultiCoreProcessor proc;

    // 两个变量位于同一个缓存行（假设缓存行大小为4个int）
    int var1_addr = 200;
    int var2_addr = 201;  // 与var1相邻，很可能在同一个缓存行

    // 初始化变量
    proc.write(0, var1_addr, 0);
    proc.write(0, var2_addr, 0);

    auto thread_func = [&](int core_id, int addr, int iterations) {
        for (int i = 0; i < iterations; i++) {
            int val = proc.read(core_id, addr);
            proc.write(core_id, addr, val + 1);
        }
    };

    std::cout << "Two threads updating DIFFERENT variables in SAME cache line" << std::endl;

    std::thread t1(thread_func, 1, var1_addr, 10);
    std::thread t2(thread_func, 2, var2_addr, 10);

    t1.join();
    t2.join();

    std::cout << "\nCache state shows excessive invalidations due to false sharing" << std::endl;
    proc.displayCacheState(1);
    proc.displayCacheState(2);
}

// 测试3：内存屏障/原子操作的影响
void testMemoryOrdering() {
    std::cout << "\n\n=== Test 3: Memory Ordering ===" << std::endl;

    MultiCoreProcessor proc;

    // 两个共享变量
    int ready_addr = 300;
    int data_addr = 301;

    // 初始化
    proc.write(0, ready_addr, 0);  // ready flag
    proc.write(0, data_addr, 0);   // data

    // 生产者-消费者模式
    std::thread producer([&]() {
        // 生产数据
        proc.write(1, data_addr, 42);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        // 发布数据（写ready flag）
        std::cout << "Producer: data ready" << std::endl;
        proc.write(1, ready_addr, 1);
    });

    std::thread consumer([&]() {
        int ready = 0;
        // 等待数据就绪（忙等待）
        while (ready == 0) {
            ready = proc.read(2, ready_addr);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // 读取数据
        int data = proc.read(2, data_addr);
        std::cout << "Consumer: got data = " << data << std::endl;
    });

    producer.join();
    consumer.join();
}

// 测试4：锁的实现（基于MESI）
class SimpleSpinLock {
private:
    MultiCoreProcessor& proc;
    int lock_addr;

public:
    SimpleSpinLock(MultiCoreProcessor& p, int addr) : proc(p), lock_addr(addr) {
        proc.write(0, lock_addr, 0);  // 0表示未锁定
    }

    void lock(int core_id) {
        while (true) {
            // 尝试原子地将0改为1
            int expected = 0;
            int desired = 1;

            // 读取当前值
            int current = proc.read(core_id, lock_addr);

            if (current == expected) {
                // 尝试获取锁
                proc.write(core_id, lock_addr, desired);

                // 验证是否成功（可能与其他核心竞争）
                int verify = proc.read(core_id, lock_addr);
                if (verify == desired) {
                    std::cout << "Core " << core_id << " acquired lock" << std::endl;
                    return;
                }
            }

            // 自旋等待
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    }

    void unlock(int core_id) {
        proc.write(core_id, lock_addr, 0);
        std::cout << "Core " << core_id << " released lock" << std::endl;
    }
};

void testSpinLock() {
    std::cout << "\n\n=== Test 4: Spin Lock Implementation ===" << std::endl;

    MultiCoreProcessor proc;
    SimpleSpinLock lock(proc, 400);
    int shared_data_addr = 401;

    proc.write(0, shared_data_addr, 0);

    auto worker = [&](int core_id) {
        for (int i = 0; i < 3; i++) {
            lock.lock(core_id);

            // 临界区开始
            int val = proc.read(core_id, shared_data_addr);
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            proc.write(core_id, shared_data_addr, val + 1);
            // 临界区结束

            lock.unlock(core_id);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    };

    std::thread threads[3];
    for (int i = 0; i < 3; i++) {
        threads[i] = std::thread(worker, i + 1);
    }

    for (auto& t : threads) {
        t.join();
    }

    int final_value = proc.read(0, shared_data_addr);
    std::cout << "Final shared data value: " << final_value << std::endl;
    std::cout << "Expected: 9" << std::endl;
}
}

void main_test_mesi() {
    using namespace mesi;
    std::cout << "MESI Protocol and Multi-threading Demo" << std::endl;
    std::cout << "======================================" << std::endl;

    testCounter();
    //testFalseSharing();
    //testMemoryOrdering();
    //testSpinLock();

    std::cout << "\nAll tests completed!" << std::endl;
}
