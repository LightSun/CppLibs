#pragma once

#include <map>
#include <unordered_map>
#include <shared_mutex>

namespace h7 {

//not done
template<typename K,typename V>
class ReadWriteMap{
public:

    template<typename Func>
    void read(const K& k, Func f){
        std::shared_lock<std::shared_mutex> lck(m_mtx);
        auto it = m_map.find(k);
        if(it != m_map.end()){
            f(k, it->second);
        }
    }

    void write(const K& k, const V& v){
        std::unique_lock<std::shared_mutex> lck(m_mtx);
        m_map[k] = v;
    }

private:
    std::map<K,V> m_map;
    mutable std::shared_mutex m_mtx;
};
}
