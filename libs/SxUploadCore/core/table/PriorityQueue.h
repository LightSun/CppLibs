#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H

#include <queue>

namespace h7 {

    template<typename T>
class __priority_queue_greater: public std::priority_queue<T,std::vector<T>,
        std::greater<T>>{
public:
    std::vector<T>& getVector(){
        return std::priority_queue<T,std::vector<T>,
                std::greater<T>>::c;
    }
};
template<typename T>
class __priority_queue_less: public std::priority_queue<T,std::vector<T>>{
public:
std::vector<T>& getVector(){
    return std::priority_queue<T,std::vector<T>>::c;
}
};
    // retain min. biggest is the last
    // AESC
    template<typename T>
    class MinPriorityQueue{
    public:
        MinPriorityQueue(unsigned int max):m_max(max){}
        MinPriorityQueue():m_max(0){}

        void push(const T& t){
            queue.push(t);
            if(m_max > 0 && queue.size() > m_max){
                queue.pop();
            }
        }
       template<typename... _Args>
       void push_ins(_Args&&... __args){
            queue.emplace(std::forward<_Args>(__args)...);
            if(m_max > 0 && queue.size() > m_max){
                queue.pop();
            }
        }
        bool pop(T& t){
            if(queue.size() > 0){
                t = queue.top();
                queue.pop();
                return true;
            }
            return false;
        }
        int size(){
            return queue.size();
        }
        std::vector<T>& getVector(){
            return queue.getVector();
        }
    public:
        __priority_queue_less<T> queue;
    private:
        unsigned int m_max;
    };
//---------------------------------
    //DESC
    template<typename T>
    class MaxPriorityQueue{
    public:
        MaxPriorityQueue(unsigned int max):m_max(max){}
        MaxPriorityQueue():m_max(0){}

        void push(const T& t){
            queue.push(t);
            if(m_max > 0 && queue.size() > m_max){
                std::vector<T>& vec = queue.getVector();
                vec.pop_back();
            }
        }
       template<typename... _Args>
       void push_ins(_Args&&... __args){
            queue.emplace(std::forward<_Args>(__args)...);
            if(m_max > 0 && queue.size() > m_max){
                std::vector<T>& vec = queue.getVector();
                vec.pop_back();
            }
        }
        bool pop(T& t){
            if(queue.size() > 0){
                std::vector<T>& vec = queue.getVector();
                t = *vec.back();
                vec.pop_back();
                return true;
            }
            return false;
        }
        int size(){
            return queue.size();
        }
        std::vector<T>& getVector(){
            return queue.getVector();
        }
    public:
        __priority_queue_greater<T> queue;
    private:
        unsigned int m_max;
    };
}
/*
class cmp
{
public:
    bool operator() ( Data &a, Data &b) {
        return a.getId() < b.getId();
    }
};
*/
#endif // PRIORITYQUEUE_H
