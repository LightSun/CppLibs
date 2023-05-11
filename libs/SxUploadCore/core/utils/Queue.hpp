#ifndef QUEUE_HPP
#define QUEUE_HPP

#include "common/common.h"
#include <list>
#include <mutex>

namespace h7 {
    template<typename T>
    class Queue{
    public:
        template<typename R>
        struct Msg
        {
            String tag;
            R data;
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
    private:
        std::list<Msg<T>> m_pendingMsgs;
        std::mutex m_mutex_msg;
    };

}

#endif // QUEUE_HPP
