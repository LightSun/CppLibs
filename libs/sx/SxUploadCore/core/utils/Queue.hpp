#pragma once

#include <list>
#include <mutex>
#include <string>
#include <condition_variable>

namespace h7 {
    template<typename T>
    class Queue{
    public:
        using String = std::string;
        using CString = const std::string&;

        template<typename E>
        struct Msg
        {
            String tag;
            E data;
        };
        void clear(){
            std::unique_lock<std::mutex> lock(m_mutex_msg);
            m_pendingMsgs.clear();
        }

        void push(CString tag, const T& data){
            {
            std::unique_lock<std::mutex> lock(m_mutex_msg);
            Msg<T> msg;
            msg.tag = tag;
            msg.data = data;
            m_pendingMsgs.push_back(std::move(msg));
            }
        }
        bool pop(Msg<T>* msg){
            std::unique_lock<std::mutex> lock(m_mutex_msg);
            if(m_pendingMsgs.size() == 0){
                return false;
            }
            *msg = m_pendingMsgs.front();
            m_pendingMsgs.pop_front();
            return true;
        }
        int size(){
            std::unique_lock<std::mutex> lock(m_mutex_msg);
            return m_pendingMsgs.size();
        }
        void wait(){
            std::unique_lock<std::mutex> lock(m_mutex_msg);
            m_case.wait(lock);
        }
        void notify(){
            std::unique_lock<std::mutex> lock(m_mutex_msg);
            m_case.notify_one();
        }
        void notify_all(){
            std::unique_lock<std::mutex> lock(m_mutex_msg);
            m_case.notify_all();
        }
    private:
        std::list<Msg<T>> m_pendingMsgs;
        std::mutex m_mutex_msg;
        std::condition_variable m_case;
    };

}
