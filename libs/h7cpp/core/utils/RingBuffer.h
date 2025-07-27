#pragma once

#include <vector>
//#include <functional>

namespace h7 {

template<typename T>
class RingBufferQueue{
public:
    RingBufferQueue(int buf_Size){
        m_buffer.resize(buf_Size);
    }

    void push(const T& t){
        m_buffer[m_nextPushPos++] = t;
        if(m_nextPushPos == m_buffer.size()){
            m_isFull = true;
            m_nextPushPos = 0;
        }
    }
    bool pool(T& t){
        if(validSize() > 0){
            t = m_buffer[m_nextPollPos++];
            if(m_nextPollPos == m_buffer.size()){
                m_isFull = false;
                m_nextPollPos = 0;
            }
            return true;
        }
        return false;
    }

    bool isFull(){
        return m_isFull && m_nextPushPos >= m_nextPollPos;
    }
    bool isEmpty(){
        return !m_isFull && m_nextPushPos == m_nextPollPos;
    }

private:
    int validSize(){
        if(m_isFull){
            return m_buffer.size() - m_nextPollPos + m_nextPushPos;
        }
        return m_nextPushPos - m_nextPollPos;
    }

private:
    std::vector<T> m_buffer;
    int m_nextPushPos {0};
    int m_nextPollPos {0};
    bool m_isFull {false};
};
}
