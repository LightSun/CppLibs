#pragma once

#include <mutex>
#include <shared_mutex>
#include <string>

namespace h7_handler_os {
    class Locker{
    public:
        using CString = const std::string&;
        Locker(CString tag, std::mutex* mtx): m_tag(tag),m_lock(mtx){
            //printf("%s >> is locked\n", tag.data());
            m_lock->lock();
        }
        ~Locker(){
            m_lock->unlock();
            //printf("%s >> is unlocked\n", m_tag.data());
        }

    private:
        std::string m_tag;
        std::mutex* m_lock;
    };
}
