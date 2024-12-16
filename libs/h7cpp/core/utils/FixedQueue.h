#pragma once

#include <cstring>
#include <vector>
#include "core/common/common.h"

namespace h7 {

//h7::NullLock, h7::mutex
template <typename T>
class FixedQueue{
public:
    FixedQueue(size_t cap){
        m_buffer.resize(cap);
    }
    int capacity(){
        return m_buffer.size();
    }
    bool push(const T& t){
        std::unique_lock<std::mutex> lck(m_lock);
        if(m_nextPushPos == m_buffer.size() - 1){
            m_buffer[m_nextPushPos++] = t;
            if(m_nextPullPos > 0){
                T* data = m_buffer.data();
                std::memmove(data, data + m_nextPullPos, sizeof(T) * validSize());
                m_nextPushPos -= m_nextPullPos;
                m_nextPullPos = 0;
            }
        }else if(m_nextPushPos >= m_buffer.size()){
            MED_ASSERT(m_nextPullPos == 0);
            T* data = m_buffer.data();
            std::memmove(data, data + 1, sizeof(T) * (validSize()-1));
            m_buffer[m_buffer.size() - 1] = t;
        }
        else{
            m_buffer[m_nextPushPos ++ ] = t;
        }
        return true;
    }
    bool pollFirst(T& out){
        std::unique_lock<std::mutex> lck(m_lock);
        if(validSize() > 0){
            out = m_buffer[m_nextPullPos ++];
            if(m_nextPullPos == m_nextPushPos){
                m_nextPullPos = 0;
                m_nextPushPos = 0;
            }
            return true;
        }
        return false;
    }
    bool pollLast(T& out){
        std::unique_lock<std::mutex> lck(m_lock);
        if(validSize() > 0){
            --m_nextPushPos;
            out = m_buffer[m_nextPushPos];
            if(m_nextPullPos == m_nextPushPos){
                m_nextPullPos = 0;
                m_nextPushPos = 0;
            }
            return true;
        }
        return false;
    }
    std::vector<T> drainOut(){
        std::vector<T> ret;
        std::unique_lock<std::mutex> lck(m_lock);
        int s = validSize();
        for(int i = 0 ; i < s ; ++i){
            ret.push_back(m_buffer[i + m_nextPullPos]);
        }
        return ret;
    }
private:
    size_t validSize(){
        return m_nextPushPos - m_nextPullPos;
    }

private:
    std::vector<T> m_buffer;
    std::mutex m_lock;
    int m_nextPushPos {0};
    int m_nextPullPos {0};
};

}
