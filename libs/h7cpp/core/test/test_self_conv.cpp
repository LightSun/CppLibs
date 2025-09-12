#include "utils/locks.h"


#include <iostream>
#include <thread>
#include <vector>

std::mutex mtx;
h7::ConditionVariableImpl cv;
bool data_ready = false;

void consumer(int id) {
    std::unique_lock<std::mutex> lock(mtx);
    while (!data_ready) {
        std::cout << "Consumer " << id << " waiting...\n" << std::endl;
        cv.wait(lock);
    }
    std::cout << "Consumer " << id << " processed data\n" << std::endl;
}

void producer() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        data_ready = true;
        std::cout << "Producer signals ready\n" << std::endl;
    }
    cv.notify_all();
}

void test_self_condition_variable() {
    std::vector<std::thread> consumers;
    for (int i = 0; i < 3; ++i) {
        consumers.emplace_back(consumer, i);
    }

    std::thread prod(producer);

    for (auto& c : consumers) c.join();
    prod.join();
}
