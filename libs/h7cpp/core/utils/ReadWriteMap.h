#pragma once

#include <map>
#include <unordered_map>
#include <shared_mutex>
#include <vector>
#include <mutex>

namespace h7 {

template<typename K,typename V, typename Map = std::unordered_map<K,V>>
class ReadWriteMap{
public:
    typedef typename Map::iterator iterator;

    template<typename Func>
    bool read(const K& k, Func f){
        std::shared_lock<std::shared_mutex> lck(m_mtx);
        iterator it = m_map.find(k);
        if(it != m_map.end()){
            f(k, it->second);
            return true;
        }
        return false;
    }
    template<typename Func>
    void readAll(Func f){
        std::shared_lock<std::shared_mutex> lck(m_mtx);
        for(auto& [k,v]: m_map){
            f(k, v);
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
    bool remove(const K& k, V& out){
        std::unique_lock<std::shared_mutex> lck(m_mtx);
        auto it = m_map.find(k);
        if(it != m_map.end()){
            out = it->second;
            m_map.erase(it);
            return true;
        }
        return false;
    }
    void remove2(const K& k){
        std::unique_lock<std::shared_mutex> lck(m_mtx);
        m_map.erase(k);
    }
    void erase(const K& k){
        std::unique_lock<std::shared_mutex> lck(m_mtx);
        m_map.erase(k);
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
    template<typename Func>
    auto getOrNull(const K& k, Func&& f){
        std::shared_lock<std::shared_mutex> lck(m_mtx);
        auto it = m_map.find(k);
        if(it != m_map.end()){
            return f(it->second);
        }
        return nullptr;
    }
    bool contains(const K& k){
        std::shared_lock<std::shared_mutex> lck(m_mtx);
        auto it = m_map.find(k);
        return it != m_map.end();
    }
    void clear(){
        std::unique_lock<std::shared_mutex> lck(m_mtx);
        m_map.clear();
    }
    Map copy(){
        std::shared_lock<std::shared_mutex> lck(m_mtx);
        Map out = m_map;
        return out;
    }
    void removeAll(const std::vector<K>& vec){
        std::unique_lock<std::shared_mutex> lck(m_mtx);
        for(auto& k : vec){
            m_map.erase(k);
        }
    }
    size_t size(){
        std::shared_lock<std::shared_mutex> lck(m_mtx);
        return m_map.size();
    }

private:
    Map m_map;
    mutable std::shared_mutex m_mtx;
};
}
