#pragma once

#include <functional>

template<typename T>
struct SmartPtr{
    T* ptr {nullptr};
    std::function<void(T*)> del;
    SmartPtr() = default;
    SmartPtr(T* ptr, std::function<void(T*)> del):ptr(ptr), del(del){}
    SmartPtr(T* ptr):ptr(ptr){} //never free

    ~SmartPtr(){delete0();}

    T *operator->() const { return ptr; }
    T* get()const{return ptr;}
    T &operator*() const {
       if(this->get() == nullptr){abort();}
       return *this->get();
    }
    T* release(){T* p = ptr; ptr = nullptr; return p;}
    void reset(T* ptr, std::function<void(T*)> del){
        delete0();
        this->ptr = ptr;
        this->del = del;
    }
    void reset(SmartPtr<T>& src){
        delete0();
        this->ptr = src.ptr;
        this->del = src.del;
        src.ptr = nullptr;
    }
private:
    void delete0(){
        if(del && ptr){del(ptr); ptr = nullptr;}
    }
    //disable copy and move
    SmartPtr<T> &operator=(SmartPtr<T> && in) = delete;
    SmartPtr<T> &operator=(const SmartPtr<T>& _in) = delete;
    SmartPtr(SmartPtr<T> && in) = delete;
    SmartPtr(const SmartPtr<T>& _in) = delete;
};

