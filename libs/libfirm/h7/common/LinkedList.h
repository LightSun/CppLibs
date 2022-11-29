#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <list>

template<class T>
class LinkedList{

public:
    void add(const T& t){
        list.push_back(t);
    }
    void add(T& t){
        list.push_back(t);
    }
    void addFirst(const T& t){
        list.push_front(t);
    }
    void addLast(const T& t){
        list.push_back(t);
    }
    template<typename... _Args>
    void insToFirst(_Args&&... __args){
        list.emplace_front(std::forward<_Args>(__args)...);
    }
    template<typename... _Args>
    void insToLast(_Args&&... __args){
        list.emplace_back(std::forward<_Args>(__args)...);
    }
    bool getFirst(T& out){
        auto it = list.front();
        if(it == list.end()){
            return false;
        }
        out = *it;
        return true;
    }
    bool getLast(T& out){
        auto it = list.back();
        if(it == list.end()){
            return false;
        }
        out = *it;
        return true;
    }
    bool pollFirst(T& out){
        auto it = list.front();
        if(it == list.end()){
            return false;
        }
        out = *it;
        list.pop_front();
        return true;
    }
    bool pollLast(T& out){
        auto it = list.back();
        if(it == list.end()){
            return false;
        }
        out = *it;
        list.pop_back();
        return true;
    }
    int size(){
        return list.size();
    }
private:
    std::list<T> list;
};

#endif // LINKEDLIST_H
