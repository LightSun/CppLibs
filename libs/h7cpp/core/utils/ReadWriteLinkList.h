#pragma once

#include <list>
#include <shared_mutex>
#include <functional>
#include <atomic>
#include <mutex>

namespace h7 {

//push to tail and pull head
template<typename T>
class ReadWriteLinkList{
public:
    ReadWriteLinkList(size_t max): m_max(max){}

    int getMax(){
        return m_max;
    }
    void setFuncDrop(void* ctx, std::function<void(void*,T&)> func){
        this->m_context = ctx;
        this->m_dropFunc = func;
    }
    //FIFO
    void push(const T& t){
        T head;
        bool droped = false;
        {
            std::unique_lock<std::shared_mutex> lck(m_mutex);
            m_list.push_back(t);
            if(m_list.size() > m_max){
                 if(m_dropFunc){
                     head = m_list.front();
                     droped = true;
                 }
                 m_list.pop_front();
            }
        }
        if(droped){
            m_dropFunc(m_context, head);
        }
    }
    void pushDirect(const T& t){
        std::unique_lock<std::shared_mutex> lck(m_mutex);
        m_list.push_back(t);
    }

    bool poll(T& t){
        std::unique_lock<std::shared_mutex> lck(m_mutex);
        if(m_list.size() > 0){
            t = m_list.front();
            m_list.pop_front();
            return true;
        }
        return false;
    }

    bool pollLast(T& t){
        std::unique_lock<std::shared_mutex> lck(m_mutex);
        if(m_list.size() > 0){
            t = m_list.back();
            m_list.pop_back();
            return true;
        }
        return false;
    }

private:
    size_t m_max;
    std::list<T> m_list;
    mutable std::shared_mutex m_mutex;
    void* m_context {nullptr};
    std::function<void(void*,T&)> m_dropFunc;
};


//Not done.
template<typename T>
class ReadWriteList{
public:
    struct Item{
        T t;
        std::atomic_bool valid {false};
    };
    ReadWriteList(size_t reserve_max,size_t max): m_max(max){
        m_list.reserve(reserve_max);
    }

    void setFuncDrop(void* ctx, std::function<void(void*,T&)> func){
        this->m_context = ctx;
        this->m_dropFunc = func;
    }
    //FIFO
    void push(const T& t){
        T head;
        bool droped = false;
        {
            std::unique_lock<std::shared_mutex> lck(m_mutex);
            m_list.push_back(t);
            if(m_list.size() > m_max){
                if(m_dropFunc){
                    head = m_list.front();
                    droped = true;
                }
                m_list.erase(m_list.begin());
            }
        }
        if(droped){
            m_dropFunc(m_context, head);
        }
    }
    void pushDirect(const T& t){
        std::unique_lock<std::shared_mutex> lck(m_mutex);
        m_list.push_back(t);
    }

    bool poll(T& t){
        std::unique_lock<std::shared_mutex> lck(m_mutex);
        if(m_list.size() > 0){
            t = m_list.front();
            m_list.erase(m_list.begin());
            return true;
        }
        return false;
    }

private:
    size_t m_max;
    std::vector<Item> m_list;
    mutable std::shared_mutex m_mutex;
    void* m_context {nullptr};
    std::function<void(void*,T&)> m_dropFunc;
};


}
