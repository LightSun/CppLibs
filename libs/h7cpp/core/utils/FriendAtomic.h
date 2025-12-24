#pragma once


#include <atomic>
#include <memory>

namespace h7 {

template<typename T>
class FriendAtomic{
public:
    FriendAtomic(const T& t){
        store(t);
    }
    T load(){
        return m_ato.load(std::memory_order_acquire);
    }
    void store(const T& t){
        m_ato.store(std::memory_order_release, t);
    }
    void add(int delta){
        auto v = load() + delta;
        store(v);
    }
    void operator=(const T& t){
        store(t);
    }
    friend bool operator!=(FriendAtomic<T>& t1, FriendAtomic<T>& t2){
        return t1.load() != t2.load();
    }
    friend bool operator!=(FriendAtomic<T>& t1, const T& t2){
        return t1.load() != t2;
    }
    friend bool operator==(FriendAtomic<T>& t1, FriendAtomic<T>& t2){
        return t1.load() == t2.load();
    }
    friend bool operator==(FriendAtomic<T>& t1, const T& t2){
        return t1.load() == t2;
    }
private:
    std::atomic<T> m_ato;
};

template<typename T>
class SharedFriendAtomic{
public:
    SharedFriendAtomic(std::shared_ptr<T> t){
        store(t);
    }
    std::shared_ptr<T> load(){
        return m_ato.load(std::memory_order_acquire);
    }
    void store(std::shared_ptr<T> t){
        m_ato.store(std::memory_order_release, t);
    }
    void operator=(std::shared_ptr<T> t){
        store(t);
    }
    friend bool operator!=(SharedFriendAtomic<T>& t1, SharedFriendAtomic<T>& t2){
        return !(operator==(t1, t2));
    }
    friend bool operator!=(SharedFriendAtomic<T>& t1, const T& t2){
        return !(operator==(t1, t2));
    }
    friend bool operator==(SharedFriendAtomic<T>& t1, SharedFriendAtomic<T>& t2){
        auto v1 = t1.load();
        auto v2 = t2.load();
        if(v1 == nullptr){
            return v2 == nullptr;
        }else{
            if(v2 != nullptr){
                return *v1 == *v2;
            }
            return false;
        }
    }
    friend bool operator==(SharedFriendAtomic<T>& t1, const T& t2){
        auto v1 = t1.load();
        return v1 && *v1 == t2;
    }
private:
    std::atomic<std::shared_ptr<T>> m_ato;
};

typedef FriendAtomic<int> FriendAtomicI32;
typedef FriendAtomic<unsigned int> FriendAtomicU32;

}
