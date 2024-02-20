#ifndef LOCK_HPP
#define LOCK_HPP

#include <mutex>
#include <shared_mutex>
#include <functional>
#include "h_atomic.h"

namespace h7 {
    class Lock{
    public:
        ~Lock(){
            unlock();
        }
        void lock(std::function<void()> func, volatile int* locked = nullptr){
            if(locked){
                h_atomic_set(locked, 1);
            }
            std::unique_lock<std::mutex> lck(m_mutex);
            //got lock, so mark unlock
            if(locked){
                h_atomic_set(locked, 0);
            }
            func();
        }
        void lock(){
            if(h_atomic_cas(&m_locked, 0, 1)){
                m_mutex.lock();
            }
        }
        bool isLocked(){
            return h_atomic_get(&m_locked) == 1;
        }
        void unlock(){
            if(h_atomic_cas(&m_locked, 1, 0)){
                m_mutex.unlock();
            }
        }
    private:
        std::mutex m_mutex;
        volatile int m_locked {0};
    };

    class ReadWriteLock{
    public:
        void readLock(std::function<void()> func, volatile int* locked = nullptr){
            if(locked){
                h_atomic_set(locked, 1);
            }
            std::shared_lock<std::shared_mutex> lck(m_mutex);
            //got lock, so mark unlock
            if(locked){
                h_atomic_set(locked, 0);
            }
            func();
        }
        void writeLock(std::function<void()> func, volatile int* locked = nullptr){
            if(locked){
                h_atomic_set(locked, 1);
            }
            std::unique_lock<std::shared_mutex> lck(m_mutex);
            if(locked){
                h_atomic_set(locked, 0);
            }
            func();
        }
    private:
        mutable std::shared_mutex m_mutex;
        //volatile int m_locked {0};
        //0 means no lock or got lock. 1 means blocked by anther thread lock.
    };
}

#endif // LOCK_HPP
