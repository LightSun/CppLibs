#pragma once

//learn from go's GMP - global-scheduer-thread
//M kernel-thread -> N go-thread.
#include <list>
#include <vector>
#include <memory>
#include <shared_mutex>
#include <functional>

namespace h7 {

template<typename T>
struct Queue{
    virtual ~Queue(){}

    virtual int size() = 0;

    virtual bool next(T& out) = 0;

    virtual bool get(T& out, std::function<bool(T&)> predicate) = 0;

    virtual T* get(std::function<bool(T&)> predicate) = 0;

    virtual void add(const T& t) = 0;

    virtual void sort(std::function<bool(const T&,const T&)> func) = 0;

    virtual T* back() = 0;
};
//may need lock
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
    bool get(T& out, std::function<bool(T&)> predicate) override{
        auto it = m_vec.begin();
        for(; it != m_vec.end(); ++it){
            auto& t = *it;
            if(predicate(t)){
                out = t;
                return true;
            }
        }
        return false;
    }
    T* get(std::function<bool(T&)> predicate)override{
        for(auto& t: m_vec){
            if(predicate(t)){
                return &t;
            }
        }
        return nullptr;
    }
    void add(const T& t) override{
        m_vec.push_back(t);
    }
    void sort(std::function<bool(const T&,const T&)> func)override{
        std::sort(m_vec.begin(), m_vec.end(), func);
    }
    T* back()override{
        return !m_vec.empty() ? &m_vec.back() : nullptr;
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
    bool get(T& out, std::function<bool(T&)> predicate) override{
        auto it = m_vec.begin();
        for(; it != m_vec.end(); ++it){
            auto& t = *it;
            if(predicate(t)){
                out = t;
                return true;
            }
        }
        return false;
    }
    T* get(std::function<bool(T&)> predicate)override{
        for(auto& t: m_vec){
            if(predicate(t)){
                return &t;
            }
        }
        return nullptr;
    }
    void add(const T& t) override{
        m_vec.push_back(t);
    }
    void sort(std::function<bool(const T&,const T&)> func)override{
        m_vec.sort(func);
    }
    T* back()override{
        return !m_vec.empty() ? &m_vec.back() : nullptr;
    }
    std::list<T> m_vec;
};

template<typename T, typename Q>
struct IGlobalManager;

template<typename T, typename Q>
struct ILocalManager;

//template<typename T>
//struct IScheduler;

//think: T often be a task. we want schedule task to target work-flow(like thread.).
//template<typename T>
//struct IScheduler{


//};

template<typename T,typename Q = ArrayQueue<T>>
struct ILocalManager
{
    void attach(IGlobalManager<T,Q>* gm, bool manageLMByGlobal = false){
        m_global = gm;
        m_global->attachLocal(this, manageLMByGlobal);
    }
    void detach(){
        m_global->detachLocal(this);
    }
    void detachRightNow(){
        m_global->detachLocalRightNow(this);
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
    bool nextElementByGlobal(T& out){
        return m_global && m_global->nextElement(out, this);
    }
    bool nextElementByLocal(T& out){
        return m_queue.next(out);
    }
    //

    bool getElement(T& out, std::function<bool(T&)> predicate){
        if(m_queue.get(out, predicate)){
            return true;
        }
        return m_global && m_global->getElement(out, predicate, this);
    }
    T* getElement(std::function<bool(T&)> predicate){
        T* ptr = m_queue.get(predicate);
        if(ptr == nullptr && m_global){
            ptr = m_global->getElement(predicate, this);
        }
        return ptr;
    }
    bool getElementByGlobal(T& out, std::function<bool(T&)> predicate){
        return m_global && m_global->getElement(out, predicate, this);
    }
    T* getElementByGlobal(std::function<bool(T&)> predicate){
        if(m_global){
            return m_global->getElement(predicate, this);
        }
        return nullptr;
    }
    T* getElementByLocal(std::function<bool(T&)> predicate){
        return m_queue.get(predicate);
    }
    bool getElementByLocal(T& out, std::function<bool(T&)> predicate){
        return m_queue.get(out, predicate);
    }

    void resizeQueue(size_t size){
        m_queue.resize(size);
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

    bool nextElement(T& out, ILocalManager<T, Q>* except = nullptr){
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
                if(lm != except && lm->nextElementByLocal(out)){
                    return true;
                }
            }
            return false;
        }
    }
    bool getElement(T& out, std::function<bool(T&)> predicate, ILocalManager<T, Q>* except = nullptr){
        if(m_queue.get(out, predicate)){
            return true;
        }
        if(h_atomic_get(&m_trimed) == 0){
            trimLocal();
        }
        {
            std::shared_lock<std::shared_mutex> lck(m_mutex);
            for(auto& _lm : m_localMs){
                auto lm = _lm.lm;
                if(lm != except && lm->getElementByLocal(out, predicate)){
                    return true;
                }
            }
            return false;
        }
    }
    T* getElement(std::function<bool(T&)> predicate, ILocalManager<T, Q>* except = nullptr){
        T* findEle = m_queue.get(predicate);
        if(findEle != nullptr){
            return findEle;
        }
        if(h_atomic_get(&m_trimed) == 0){
            trimLocal();
        }
        {
            std::shared_lock<std::shared_mutex> lck(m_mutex);
            for(auto& _lm : m_localMs){
                auto lm = _lm.lm;
                if(lm != except){
                    findEle = lm->getElementByLocal(predicate);
                    if(findEle != nullptr){
                        return findEle;
                    }
                }
            }
            return nullptr;
        }
    }
    T* getElementWithoutSelf(std::function<bool(T&)> predicate, ILocalManager<T, Q>* except = nullptr){
        T* findEle = nullptr;
        if(h_atomic_get(&m_trimed) == 0){
            trimLocal();
        }
        {
            std::shared_lock<std::shared_mutex> lck(m_mutex);
            for(auto& _lm : m_localMs){
                auto lm = _lm.lm;
                if(lm != except){
                    findEle = lm->getElementByLocal(predicate);
                    if(findEle != nullptr){
                        return findEle;
                    }
                }
            }
            return nullptr;
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
    void detachLocalRightNow(ILocalManager<T, Q>* lm){
        {
            std::unique_lock<std::shared_mutex> lck(m_mutex);
            for(int i = m_localMs.size() ; i >= 0 ; --i){
                if(m_localMs[i] == lm){
                    m_localMs.erase(m_localMs.begin() + i);
                    break;
                }
            }
        }
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
