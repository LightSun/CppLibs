#pragma once

#include <map>
#include <unordered_map>
#include <shared_mutex>

namespace h7 {

template<typename K,typename V, typename Map = std::unordered_map<K,V>>
class ReadWriteMap{
public:
    typedef typename Map::iterator iterator;

    template<typename Func>
    void read(const K& k, Func f){
        std::shared_lock<std::shared_mutex> lck(m_mtx);
        auto it = m_map.find(k);
        if(it != m_map.end()){
            f(k, it->second);
        }
    }
    void put(const K& k, const V& v){
        std::unique_lock<std::shared_mutex> lck(m_mtx);
        m_map[k] = v;
    }

    template<typename... _Args>
    std::pair<iterator, bool>
    emplace(_Args&&... __args){
        std::unique_lock<std::shared_mutex> lck(m_mtx);
        return m_map.emplace(std::forward<_Args>(__args)...);
    }

    bool get(const K& k, V& v){
        std::shared_lock<std::shared_mutex> lck(m_mtx);
        auto it = m_map.find(k);
        if(it != m_map.end()){
            v = it->second;
            return true;
        }
        return false;
    }

private:
    Map m_map;
    mutable std::shared_mutex m_mtx;
};
}
