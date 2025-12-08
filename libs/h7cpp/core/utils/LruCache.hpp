#pragma once
#include <algorithm>
#include <cstdint>
#include <list>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <functional>

#include "common/common.h"
#include "common/SkRefCnt.h"

namespace h7 {
class NullLock {
 public:
  void lock() {}
  void unlock() {}
  bool try_lock() { return true; }
};

template <typename K, typename V>
struct _KeyValuePair : public SkRefCnt{
 public:
  K key;
  V value;
  size_t useCount {1};
  _KeyValuePair(K k, V v):key(std::move(k)),value(std::move(v)) {}

  //DESC
  friend bool operator < (_KeyValuePair<K,V> const &a, _KeyValuePair<K,V> const &b){
      return a.useCount > b.useCount;
  }
  friend bool operator > (_KeyValuePair<K,V> const &a, _KeyValuePair<K,V> const &b){
      return a.useCount < b.useCount;
  }
};
/**
 *	The LRU Cache class templated by
 *		Key - key type
 *		Value - value type
 *		MapType - an associative container like std::unordered_map
 *		LockType - a lock type derived from the Lock class (default:
 *NullLock = no synchronization)
 *
 *	The default NullLock based template is not thread-safe, however passing
 *Lock=std::mutex will make it
 *	thread-safe
 */
template <class Key, class Value, class Lock = NullLock,
          class Map = std::unordered_map<Key, sk_sp<_KeyValuePair<Key,Value>>>
          >
class LruCache {
public:
    typedef _KeyValuePair<Key,Value> Pair_type0;
    typedef sk_sp<Pair_type0> Pair_type;
    typedef std::list<Pair_type> List_type;
    typedef Lock Lock_type;
    using Guard = std::lock_guard<Lock_type>;

    template <class Key1, class Value1>
    struct Callback{
        std::function<size_t(const Key1*, const Value1*)> func_kv
            = [](const Key1*, const Value1* ){
            return 1;
        };
        std::function<bool(const Key1*, Value1*)> func_reCreate;      //must
        std::function<void(const Key1*, const Value1*)> func_insert;  //must
        std::function<void(const Key1*, const Value1*)> func_remove;
    };

    explicit LruCache(size_t maxSize, Callback<Key,Value>* cb = nullptr)
        :m_maxSize(maxSize){
        if(cb){
            m_callback.func_kv = cb->func_kv;
            m_callback.func_reCreate = cb->func_reCreate;
            m_callback.func_insert = cb->func_insert;
            m_callback.func_remove = cb->func_remove;
        }
    }
    virtual ~LruCache() = default;

    size_t size() const {
        Guard g(lock_);
        return m_cache.size();
    }
    bool empty() const {
        Guard g(lock_);
        return m_cache.empty();
    }
    bool isEmpty() const {
        Guard g(lock_);
        return m_cache.empty();
    }
    void clear() {
        Guard g(lock_);
        m_cache.clear();
        m_keys.getList().clear();
        m_curSize = 0;
    }
    //return true means as new pair.
    bool put(const Key& k,const Value &v, Value* oldV = nullptr) {
        Guard g(lock_);

        const auto it = m_cache.find(k);
        size_t deltaVal = 0;
        size_t newSize = m_callback.func_kv(&k, &v);

        if (it != m_cache.end()) {
           Pair_type& pt = it->second;
           deltaVal = newSize - m_callback.func_kv(&k, &pt->value);
           if(oldV != nullptr){
               *oldV = pt->value;
           }
           if(m_callback.func_remove){
                m_callback.func_remove(&k, &pt->value);
           }
           pt->value = v;
           pt->useCount ++;
           //
           ensureSize(deltaVal);
           //cb
           m_callback.func_insert(&k, &v);
           return false;
        }
        //add to list and map
        Pair_type pt = sk_make_sp<Pair_type0>(k,v);
        m_keys.push_back(pt);
        m_cache[k] = pt;
        //sort
        sortList();

        ensureSize(newSize);
        //cb
        m_callback.func_insert(&k, &v);
        return true;
    }

    bool get(const Key& kIn, Value& vOut) {
        Guard g(lock_);
        return getValue0(kIn, vOut);
    }

    Value getCopy(const Key& k) {
        Guard g(lock_);
        Value val;
        getValue0(k, val);
        return val;
    }

    bool remove(const Key& k) {
        Guard g(lock_);
        size_t delta = 0;
        auto iter = m_cache.find(k);
        if (iter == m_cache.end()) {
          return false;
        }
        //cb
        if(m_callback.func_remove){
            m_callback.func_remove(&iter->second->key, &iter->second->value);
        }
        //size
        delta -= m_callback.func_kv(&k, &iter->second->value);
        //remove
        auto nit = m_keys.find(iter->second);
        if(nit != m_keys.end()){
            m_keys.erase(nit);
        }
        m_cache.erase(iter);
        //
        ensureSize(delta);
        return true;
    }
    bool contains(const Key& k) const {
        Guard g(lock_);
        return m_cache.find(k) != m_cache.end();
    }

    size_t getMaxSize() const { return m_maxSize; }
    size_t getCurrentSize() const { return m_curSize; }

    template <typename F>
    void cwalk(F& f) const {
        Guard g(lock_);
        auto& keys_ = m_keys;
        std::for_each(keys_.begin(), keys_.end(), f);
    }

protected:
    bool getValue0(const Key& k,Value& out) {
        const auto iter = m_cache.find(k);
        if (iter == m_cache.end()) {
            //try get from recreate
            if(m_callback.func_reCreate(&k, &out)){
                return put(k, out);
            }
            return false;
        }
        iter->second->useCount ++;
        out = iter->second->value;
        return true;
    }
    void ensureSize(size_t delta) {
        size_t expectSize = m_curSize + delta;
        if (expectSize <= m_maxSize) {
           m_curSize += delta;
           return;
        }
        //DESC
        size_t to_del_size = expectSize - m_maxSize;
        size_t del_size = 0;
        while (del_size < to_del_size) {
            if(m_keys.size() == 0){
                break;
            }
            auto& it = m_keys.back();
            del_size += m_callback.func_kv(&it->key, &it->value);
            if(m_callback.func_remove){
                m_callback.func_remove(&it->key, &it->value);
            }
            m_cache.erase(it->key);
            m_keys.pop_back();
        }
        m_curSize -= del_size;
    }
    //DESC
    void sortList(){
        m_keys.sort([](const Pair_type& p1, const Pair_type& p2){
            return p1->useCount > p2->useCount;
        });
    }

private:
    LruCache(const LruCache&) = delete;
    LruCache& operator=(const LruCache&) = delete;

    mutable Lock lock_;
    Map m_cache;
    List_type m_keys;
    size_t m_maxSize;
    size_t m_curSize {0};

    Callback<Key,Value> m_callback;
};

}
