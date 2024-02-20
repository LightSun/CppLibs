#pragma once

#include "common/SkRefCnt.h"
#include <list>

namespace h7 {

template<class T>
class LinkedList: public SkRefCnt{

public:
    ~LinkedList(){
    }
    //list
    inline void add(const T& data){
        list.push_back(data);
    }
    template<typename... _Args>
    inline void add_cons(_Args&&... __args){
        list.emplace_back(std::forward<_Args>(__args)...);
    }
    void addFirst(const T& data){
        list.push_front(data);
    }
    template<typename... _Args>
    void addFirst_cons(_Args&&... __args){
        list.emplace_front(std::forward<_Args>(__args)...);
    }
    void addLast(const T& data){
        list.push_back(data);
    }
    template<typename... _Args>
    void addLast_cons(_Args&&... __args){
        list.emplace_back(std::forward<_Args>(__args)...);
    }
    void clear(){
        list.clear();
    }
    int size(){
        return list.size();
    }
    bool removeFirst(T* out){
        if(list.size() == 0){
            return false;
        }
        if(out){
            *out = list.front();
        }
        list.pop_front();
        return true;
    }
    bool removeLast(T* out){
        if(list.size() == 0){
            return false;
        }
        if(out){
            *out = list.back();
        }
        list.pop_back();
        return true;
    }
    bool pop(T* out){
        return removeLast(out);
    }
    void push(const T& data){
        addLast(data);
    }
    bool getFirst(T* out) const{
        if(list.size() == 0){
            return false;
        }
        if(out){
            *out = list.front();
        }
        return true;
    }
    bool getLast(T* out) const{
        if(list.size() == 0){
            return false;
        }
        if(out){
            *out = list.back();
        }
        return true;
    }

public:
    std::list<T> list;

};

}
