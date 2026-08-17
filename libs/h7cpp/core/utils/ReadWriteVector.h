#pragma once

#include <vector>
#include <shared_mutex>
#include <algorithm>
#include <mutex>

namespace h7 {

template<typename T>
class ReadWriteVector{
public:
    typedef typename std::vector<T>::iterator iterator;

    void prepare(size_t size){
        std::unique_lock<std::shared_mutex> lck(m_mtx);
        m_vec.reserve(size);
    }

    template<typename... _Args>
    void emplace_back(_Args&&... __args){
        std::unique_lock<std::shared_mutex> lck(m_mtx);
        m_vec.emplace_back(std::forward<_Args>(__args)...);
    }

    void push_back(const T& t){
        std::unique_lock<std::shared_mutex> lck(m_mtx);
        m_vec.push_back(std::move(t));
    }
    void clear(){
        std::unique_lock<std::shared_mutex> lck(m_mtx);
        m_vec.clear();
    }
    void copyAndClear(std::vector<T>& out){
        std::unique_lock<std::shared_mutex> lck(m_mtx);
        out = m_vec;
        m_vec.clear();
    }
    std::vector<T> copy(){
        std::shared_lock<std::shared_mutex> lck(m_mtx);
        return m_vec;
    }
    std::vector<T> subList(int cIdx, int offset_lr){
        std::shared_lock<std::shared_mutex> lck(m_mtx);
        int start = cIdx - offset_lr;
        int end = cIdx + offset_lr;
        start = std::max(0, start);
        end = std::min((int)m_vec.size(), end);
        return std::vector<T>(m_vec.begin() + start, m_vec.begin() + end);
    }
    void addAndCopy(const T& t, int limitCnt, std::vector<T>& out){
        std::unique_lock<std::shared_mutex> lck(m_mtx);
        m_vec.push_back(std::move(t));
        if(m_vec.size() > limitCnt){
            m_vec.erase(m_vec.begin());
        }
        out = m_vec;
    }
    //note order
    void removeAll(const std::vector<int>& ids){
        std::unique_lock<std::shared_mutex> lck(m_mtx);
        for(int id: ids){
            m_vec.erase(m_vec.begin() + id);
        }
    }
    iterator begin(){
        return m_vec.begin();
    }
    iterator end(){
        return m_vec.end();
    }
    bool contains(const T& t){
        std::shared_lock<std::shared_mutex> lck(m_mtx);
        for(auto& e : m_vec){
            if(e == t){
                return true;
            }
        }
        return false;
    }
    void removeAt(int idx){
        std::unique_lock<std::shared_mutex> lck(m_mtx);
        m_vec.erase(m_vec.begin() + idx);
    }
    bool getAt(int index, T& out){
        std::shared_lock<std::shared_mutex> lck(m_mtx);
        if(index < m_vec.size()){
            out = m_vec[index];
            return true;
        }
        return false;
    }
    template<typename Func>
    void readAll(Func&& func){
        std::shared_lock<std::shared_mutex> lck(m_mtx);
        func(m_vec);
    }
    void getRange(int startIdx, int len, std::vector<T>& out){
        std::shared_lock<std::shared_mutex> lck(m_mtx);
        int actSize = std::min(len, (int)m_vec.size() - startIdx);
        for(int i = 0 ; i < actSize; ++i){
            out.push_back(m_vec[i + startIdx]);
        }
    }
    void removeRange(int startIdx, int len, std::vector<T>* out){
        std::unique_lock<std::shared_mutex> lck(m_mtx);
        int actSize = std::min(len, (int)m_vec.size() - startIdx);
        if(actSize <= 0){
            return;
        }
        if(out){
            for(int i = 0 ; i < actSize; ++i){
                out->push_back(m_vec[i + startIdx]);
            }
        }
        m_vec.erase(m_vec.begin() + startIdx, m_vec.begin() + startIdx + actSize);
    }
    //default: AESC
    void insertByTail(const T& e){
        std::unique_lock<std::shared_mutex> lck(m_mtx);
        int insPos = -1;
        const int curSize = m_vec.size();
        if(curSize > 0){
            for(int i = curSize - 1 ; i >= 0 ; --i){
                if(m_vec[i] < e){
                    insPos = i + 1;
                    break;
                }
            }
        }
        if(insPos >= 0){
            m_vec.insert(m_vec.begin() + insPos, e);
        }else{
            m_vec.push_back(e);
        }
    }
    int size(){
        std::shared_lock<std::shared_mutex> lck(m_mtx);
        return m_vec.size();
    }

private:
    std::vector<T> m_vec;
    mutable std::shared_mutex m_mtx;
};
}
