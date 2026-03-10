#pragma once

#include <atomic>
#include <memory>

namespace med {

template<typename T>
class RWVersionPtr{
public:
    template<typename T1>
    struct Item{
        size_t version {1};
        std::shared_ptr<T1> ptr;

        Item(std::shared_ptr<T1> ptr):ptr(std::move(ptr)){ }
        Item(size_t version, std::shared_ptr<T1> ptr):version(version),ptr(ptr){ }

        std::shared_ptr<Item<T1>> update(){
            return std::make_shared<Item<T1>>(version + 1, ptr);
        }
        std::shared_ptr<Item<T1>> copy(){
            auto ptr0 = std::make_shared<Item<T1>>(version + 1, std::make_shared<T1>());
            *ptr0->ptr = *ptr;
            return ptr0;
        }
    };
    using Ptr = std::shared_ptr<Item<T>>;

    static Ptr makePtr(size_t version, std::shared_ptr<T> p){
        return std::make_shared<Item<T>>(version, p);
    }

    RWVersionPtr(std::shared_ptr<T> ptr){
        m_spPtr = std::make_shared<Item<T>>(ptr);
    }
    RWVersionPtr(){
        m_spPtr = std::make_shared<Item<T>>(std::make_shared<T>());
    }
    Ptr load(){
        return std::atomic_load_explicit(&m_spPtr, std::memory_order_acquire);
    }
    void store(Ptr ptr){
        std::atomic_store_explicit(&m_spPtr, ptr, std::memory_order_release);
    }
    bool isCurrentVersion(Ptr ptr)const{
        return ptr->version == m_spPtr->version;
    }
    template<typename Func>
    void opWriteData(Func&& func){
        auto ptr = load();
        if(isCurrentVersion(ptr)){
            auto nptr = ptr->copy();
            func(nptr->ptr);
            store(nptr);
        }else{
            auto nptr = ptr->update();
            func(nptr->ptr);
            store(nptr);
        }
    }

private:
    alignas(64) std::shared_ptr<Item<T>> m_spPtr;
};

}
