#pragma once

//learn from go's GMP - global-scheduer-thread
//M kernel-thread -> N go-thread.
#include <list>
#include <vector>
#include <memory>


namespace h7 {

template<typename T>
struct Queue{
    virtual int size() = 0;
    virtual bool next(T& out) = 0;
    virtual void add(const T& t) = 0;
};
//lock
template<typename T>
struct ArrayQueue: public Queue<T>
{
    int size() override{
        return m_vec.size();
    }
    bool next(T& out) override{
        if(m_vec.size() == 0) return false;
        out = m_vec.front();
        m_vec.erase(m_vec.begin());
        return true;
    }
    void add(const T& t) override{
        m_vec.push_back(t);
    }
    std::vector<T> m_vec;
};

template<typename T, typename Q>
struct IGlobalManager;

template<typename T, typename Q>
struct ILocalManager;

template<typename T>
struct IScheduler;

//think: T often be a task. we want schedule task to target work-flow(like thread.).
template<typename T>
struct IScheduler{


};

template<typename T,typename Q = ArrayQueue<T>>
struct ILocalManager
{
    void attach(IGlobalManager<T,Q>* gm){
        m_global = gm;
    }
    Queue<T>* getQueue(){
        return &m_queue;
    }

    bool nextElement(T& out){
        if(m_queue.next(out)){
            return true;
        }
        return m_global && m_global->nextElement(out, this);
    }

private:
    IGlobalManager<T, Q>* m_global {nullptr};
    Q m_queue;
};
template<typename T, typename Q = ArrayQueue<T>>
struct IGlobalManager
{
    Queue<T>* getQueue(){
        return &m_queue;
    }
    bool nextElement(T& out,ILocalManager<T, Q>* except = nullptr){
        if(m_queue.next(out)){
            return true;
        }
        for(auto& lm : m_localMs){
            if(lm != except && lm->nextElement(out)){
                return true;
            }
        }
        return false;
    }

private:
    std::vector<ILocalManager<T, Q>*> m_localMs;
    Q m_queue;
};

}
