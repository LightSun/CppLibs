#pragma once

#include <atomic>
#include <mutex>

namespace h7 {

#define H7_DEF_SINGLETON(T)\
public:\
static T* get_instance() {\
    T* tmp = instance_.load(std::memory_order_acquire);\
    if (tmp == nullptr) {\
        std::unique_lock<std::mutex> lk(mutex_);\
        tmp = instance_;\
        if (tmp == nullptr) {\
            tmp = new T();\
            instance_.store(tmp, std::memory_order_release);\
        }\
    }\
    return tmp;\
}\
private:\
T() = default;\
static std::atomic<T*> instance_;\
static std::mutex mutex_;
/*
class Singleton
{
public:
    static Singleton* get_instance() {
        Singleton* tmp = instance_.load(std::memory_order_acquire);
        if (tmp == nullptr) {
            std::unique_lock<std::mutex> lk(mutex_);
            tmp = instance_;
            if (tmp == nullptr) {
                tmp = new Singleton();
                instance_.store(tmp, std::memory_order_release);
            }
        }
        return tmp;
    }

private:
    Singleton() = default;
    static std::atomic<Singleton*> instance_;
    static std::mutex mutex_;
};
*/

}

