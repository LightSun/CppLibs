#pragma once

//learn from go's GMP - global-scheduer-thread
//M kernel-thread -> N go-thread.
#include <list>
#include <vector>
#include <memory>
#include <shared_mutex>

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

template<typename T>
struct LinkListQueue: public Queue<T>
{
    int size() override{
        return m_vec.size();
    }
    bool next(T& out) override{
        if(m_vec.size() == 0) return false;
        out = m_vec.front();
        m_vec.pop_front();
        return true;
    }
    void add(const T& t) override{
        m_vec.push_back(t);
    }
    std::list<T> m_vec;
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
        m_global->attachLocal(this);
    }
    void detach(){
        m_global->detachLocal(this);
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
struct LocalManagerItem{
    ILocalManager<T, Q>* lm {nullptr};
    std::unique_ptr<ILocalManager<T, Q>> splm;
    int volatile valid {1};

    void markInvalid(){
        h_atomic_cas(&valid, 1, 0);
    }
    bool isValid(){
        return h_atomic_get(&valid) == 1;
    }
    void setLocalManager(ILocalManager<T, Q>* klm, bool manage){
        if(manage){
            splm = std::unique_ptr<ILocalManager<T, Q>>(klm);
        }
        lm = klm;
    }
};

template<typename T, typename Q = ArrayQueue<T>>
struct IGlobalManager
{
public:
    Queue<T>* getQueue(){
        return &m_queue;
    }

    bool nextElement(T& out,ILocalManager<T, Q>* except = nullptr){
        if(m_queue.next(out)){
            return true;
        }
        if(h_atomic_get(&m_trimed) == 0){
            trimLocal();
        }
        {
            std::shared_lock<std::shared_mutex> lck(m_mutex);
            for(auto& _lm : m_localMs){
                auto lm = _lm.lm;
                if(lm != except && lm->nextElement(out)){
                    return true;
                }
            }
            return false;
        }
    }
private:
    void detachLocal(ILocalManager<T, Q>* lm){
        {
            std::shared_lock<std::shared_mutex> lck(m_mutex);
            for(auto& li : m_localMs){
                if(li.lm == lm){
                    li.markInvalid();
                    break;
                }
            }
        }
        h_atomic_cas(&m_trimed, 1, 0);
    }
    void attachLocal(ILocalManager<T, Q>* lm, bool manageLM){
        {
            std::unique_lock<std::shared_mutex> lck(m_mutex);
            LocalManagerItem<T,Q> lmi;
            lmi.setLocalManager(lm, manageLM);
            m_localMs.push_back(lmi);
        }
    }
    void trimLocal(){
        {
            std::unique_lock<std::shared_mutex> lck(m_mutex);
            int size = m_localMs.size();
            for(int i = size - 1 ; i >=0 ; --i){
                if(!m_localMs[i].isValid){
                    m_localMs.erase(m_localMs.begin() + i);
                }
            }
        }
        h_atomic_cas(&m_trimed, 0, 1);
    }
private:
    friend ILocalManager<T, Q>;
    std::vector<LocalManagerItem<T,Q>> m_localMs;
    Q m_queue;
    mutable std::shared_mutex m_mutex;
    volatile int m_trimed {1};
};

}
