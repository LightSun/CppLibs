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
    T& getFirst(){
        return list.front();
    }
    T& getLast(){
        return list.back();
    }
    T pollFirst(){
        auto it = list.front();
        list.pop_front();
        return it;
    }
    T pollLast(){
        auto it = list.back();
        list.pop_back();
        return it;
    }
    int size(){
        return list.size();
    }
private:
    std::list<T> list;
};

#endif // LINKEDLIST_H
