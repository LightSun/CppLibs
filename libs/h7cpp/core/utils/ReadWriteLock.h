#pragma once

#include <functional>
#include <atomic>
#include <mutex>
#include <shared_mutex>

namespace h7 {

class ReadWriteLock{
public:

    ReadWriteLock(bool reqLockFlag):m_reqLockFlag(reqLockFlag){}

    bool readLock(std::function<void()> func){
        if(m_reqLockFlag){
            bool old = false;
            bool runOk = false;
            if(m_lockFlag.compare_exchange_strong(old, true, std::memory_order_acquire)){
                 std::shared_lock<std::shared_mutex> lck(m_mutex);
                 func();
                 runOk = true;
            }
            if(runOk){
                m_lockFlag.store(false, std::memory_order_relaxed);
            }
            return runOk;
        }else{
            std::shared_lock<std::shared_mutex> lck(m_mutex);
            func();
            return true;
        }
    }
    bool writeLock(std::function<void()> func){
        if(m_reqLockFlag){
            bool old = false;
            bool runOk = false;
            if(m_lockFlag.compare_exchange_strong(old, true, std::memory_order_acquire)){
                std::unique_lock<std::shared_mutex> lck(m_mutex);
                func();
                runOk = true;
            }
            if(runOk){
                m_lockFlag.store(false, std::memory_order_relaxed);
            }
            return runOk;
        }else{
            std::unique_lock<std::shared_mutex> lck(m_mutex);
            func();
            return true;
        }
    }
    bool isLocked(){
        return m_lockFlag.load(std::memory_order_acquire);
    }

private:
    mutable std::shared_mutex m_mutex;
    std::atomic_bool m_lockFlag {false};
    bool m_reqLockFlag {false};
};
}
