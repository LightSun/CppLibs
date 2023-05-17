#pragma once

#include <vector>
#include <functional>
#include <algorithm>
#include "utils/binary_search.h"
#include "utils/hash.h"

#define _FastList_HASH_SEED 11

namespace h7 {

template<typename E>
struct SortedList_Item{
    using IT = SortedList_Item<E>;
    E t;
    unsigned int hash;

    friend bool operator < (IT const &a, IT const &b){
        return a.hash < b.hash;
    }
    friend bool operator > (IT const &a, IT const &b){
        return a.hash > b.hash;
    }
    bool operator==(IT const& oth){
        return hash == oth.hash;
    }
};

template<typename T, typename _Alloc = std::allocator<SortedList_Item<T>>>
class SortedList
{
public:
    using U32 = unsigned int;
    using Item = struct SortedList_Item<T>;
    using Func_hash = std::function<unsigned int(const T& t)>;
    SortedList(Func_hash hash): m_func_hash(hash){
    }
    SortedList(){}

    void addAll(const std::vector<T>& vec, bool _sort = true){
        for(auto& t : vec){
            add(t, false);
        }
        if(_sort){
            sort();
        }
    }

    void add(const T& t, bool _sort = true){
        Item item;
        item.t = t;
        if(m_func_hash){
            item.hash = m_func_hash(t);
        }else{
            item.hash = fasthash32(&t, sizeof(T), _FastList_HASH_SEED);
        }
        m_items.push_back(std::move(item));
        if(_sort){
            sort();
        }
    }
    void addAt(U32 idx, const T& t, bool _sort = true){
        if(idx > m_items.size()){
            idx = m_items.size();
        }
        Item item;
        item.t = t;
        if(m_func_hash){
            item.hash = m_func_hash(t);
        }else{
            item.hash = fasthash32(&t, sizeof(T), _FastList_HASH_SEED);
        }
        m_items.insert(m_items.begin() + idx, std::move(item));
        if(_sort){
            sort();
        }
    }
    void sort(){
        std::sort(m_items.begin(), m_items.end());
    }
    bool remove(const T& t){
        int idx = indexOf(t);
        if(idx < 0){
            return false;
        }
        removeAt(idx);
        return true;
    }
    void removeAt(U32 idx){
        m_items.erase(m_items.begin() + idx);
    }

    void set(U32 index, const T& t, bool _sort = true){
        if(index >= m_items.size()){
            return;
        }
        Item item;
        item.t = t;
        if(m_func_hash){
            item.hash = m_func_hash(t);
        }else{
            item.hash = fasthash32(&t, sizeof(T), _FastList_HASH_SEED);
        }
        m_items[index] = std::move(item);
        if(_sort){
            sort();
        }
    }

    T& get(U32 index){
        return m_items[index].t;
    }

    int indexOf(const T& t){
        U32 _hash;
        if(m_func_hash){
            _hash = m_func_hash(t);
        }else{
            _hash = fasthash32(&t, sizeof(T), _FastList_HASH_SEED);
        }
        int offset = offsetof(Item, hash);
        //may be -2... -n
        return binarySearchOffset_u(m_items.data(), sizeof(Item), offset,
                             0, m_items.size(), _hash);
    }
    size_t size(){
        return m_items.size();
    }

private:
    std::vector<Item, _Alloc> m_items;
    Func_hash m_func_hash;
};

}

