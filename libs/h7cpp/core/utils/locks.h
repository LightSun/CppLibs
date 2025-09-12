#pragma once

#include <atomic>
#include <mutex>
#include <list>
#include <algorithm>
#include <condition_variable>

namespace h7 {

class ConditionVariableImpl {
public:
    ConditionVariableImpl() = default;
    ConditionVariableImpl(const ConditionVariableImpl&) = delete;
    ConditionVariableImpl& operator=(const ConditionVariableImpl&) = delete;

    void wait(std::unique_lock<std::mutex>& lock) {
        waiter w;
        {
            std::lock_guard<std::mutex> internal_lock(internal_mutex_);
            waiters_.push_back(&w);
            wait_count_++;
        }
        lock.unlock();
        try {
            w.mtx.lock();
        } catch (...) {
            remove_waiter(&w);
            throw;
        }
        lock.lock();
    }
    template<typename _Predicate>
    void wait(std::unique_lock<std::mutex>& __lock, _Predicate __p){
        while (!__p()){
            wait(__lock);
        }
    }

    void notify_one() {
        std::lock_guard<std::mutex> lock(internal_mutex_);
        if (wait_count_ == 0) return;

        waiter* w = waiters_.front();
        waiters_.pop_front();
        wait_count_--;
        w->mtx.unlock();
    }

    void notify_all() {
        std::lock_guard<std::mutex> lock(internal_mutex_);
        for (auto* w : waiters_) {
            w->mtx.unlock();
        }
        waiters_.clear();
        wait_count_ = 0;
    }

private:
    struct waiter {
        std::mutex mtx;
        waiter(){
            mtx.lock();
        }
        waiter(const waiter&) = delete;
        waiter& operator=(const waiter&) = delete;
    };

    void remove_waiter(waiter* w) {
        std::lock_guard<std::mutex> lock(internal_mutex_);
        auto it = std::find(waiters_.begin(), waiters_.end(), w);
        if (it != waiters_.end()) {
            waiters_.erase(it);
            wait_count_--;
        }
    }

    std::mutex internal_mutex_;
    std::list<waiter*> waiters_;
    uint32_t wait_count_ {0};
};

class CppLock {
 public:
  void lock() {
    while (flag.test_and_set(std::memory_order_acquire)) {}
  }

  void unlock() {
    flag.clear(std::memory_order_release);
  }
 private:
  std::atomic_flag flag {0};
};

class MutexLock {
 public:
  void lock() {
     mutex.lock();
  }
  void unlock() {
    mutex.unlock();
  }
  void wait(){
    std::unique_lock<std::mutex> lck(mutex);
    conv.wait(lck);
  }
  void notify(bool all = true){
    std::unique_lock<std::mutex> lck(mutex);
    if(all){
        conv.notify_all();
    }else{
        conv.notify_one();
    }
  }
 private:
  std::mutex mutex;
  std::condition_variable conv;
};

//------------------------
class MutexLockHolder{
public:
    MutexLockHolder(MutexLock& lock):m_lock(lock){
        lock.lock();
    }
    ~MutexLockHolder(){
        m_lock.unlock();
    }

private:
    MutexLock& m_lock;
};

class CppLockHolder{
public:
    CppLockHolder(CppLock& lock):m_lock(lock){
        lock.lock();
    }
    ~CppLockHolder(){
        m_lock.unlock();
    }

private:
    CppLock& m_lock;
};

}
