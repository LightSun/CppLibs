#ifndef ASYNCS_H
#define ASYNCS_H

#include "common/common.h"
#include "common/SkRefCnt.h"
#include "utils/h_atomic.h"
#include <list>
#include <mutex>
#include "utils/Executor.h"

namespace h7 {

class _TaskCallback: public SkRefCnt{
public:
    ~_TaskCallback(){}
    virtual void onSuccess() = 0;
    virtual void onFailed(int,CString msg) = 0;
    virtual void onStopped() = 0;
    virtual bool isStopped() = 0;
};

class Asyncs : public _TaskCallback
{
public:
    using TaskPtr = sk_sp<h7::_TaskCallback>;
    struct Callback{
        std::function<void(CString)> onSuccess;
        std::function<void(CString)> onStop;
        std::function<void(CString, int, CString msg)> onFail;
    };
    Asyncs(){
        //PRINTLN("init Asyncs: %p\n", this);
        h_atomic_set(&m_stopped, 0);
        h_atomic_set(&m_started, 0);
    }
    ~Asyncs(){
        //PRINTLN("de_init ~Asyncs: %p\n", this);
        if(m_func_destroy){
            (*m_func_destroy)(m_callback, m_exe_schedule, m_exe_cb);
        }
    }
    sk_sp<Asyncs> setCallback(Callback* cb){
        std::unique_lock<std::mutex> lock(m_mtx_cb);
        this->m_callback = cb;
        return sk_ref_sp(this);
    }
    sk_sp<Asyncs> scheduleOn(Executor* exe, bool backService=false){
        this->m_exe_schedule = exe;
        if(m_exe_schedule){
            if(!backService){
                m_threads.emplace_back([this](){
                    m_exe_schedule->waitFinish();
                });
            }else{
                m_exe_schedule->finish();
            }
        }
        return sk_ref_sp(this);
    }
    sk_sp<Asyncs> observerOn(Executor* exe,bool backService=false){
        this->m_exe_cb = exe;
        if(m_exe_cb){
            if(!backService){
                m_threads.emplace_back([this](){
                    m_exe_cb->waitFinish();
                });
            }else{
                m_exe_cb->finish();
            }
        }
        return sk_ref_sp(this);
    }
    sk_sp<Asyncs> of(std::function<void(TaskPtr)> _func, CString name = ""){
        if(h_atomic_get(&m_started) == 1){
            return sk_ref_sp(this);
        }
        auto _task = FUNC_MAKE_SHARED_PTR_1(void(TaskPtr), _func);
        auto func = [this, _task](){
            if(h_atomic_get(&m_stopped) == 1){
                return;
            }
            (*_task)(sk_ref_sp(this));
        };
        Task task = {name, func};
        {
             std::unique_lock<std::mutex> lock(m_mtx_tasks);
             m_tasks.push_back(std::move(task));
        }
        return sk_ref_sp(this);
    }
    sk_sp<Asyncs> next(std::function<void(TaskPtr)> _func, CString name = ""){
        return of(_func, name);
    }
    void start(){
        if(h_atomic_get(&m_started) == 1){
            return;
        }
        startInternal();
    }
    void stop(){
        h_atomic_set(&m_stopped, 1);
    }
    /**
     * @brief finish: finish this async task manager
     * @param wait: true to wait scheduler and callback finish or run in background.
     */
    void finish(bool wait = true){
        if(wait){
            for(unsigned int i = 0 ; i < m_threads.size() ; ++i){
                m_threads[i].join();
            }
            m_threads.clear();
        }else{
            for(unsigned int i = 0 ; i < m_threads.size() ; ++i){
                m_threads[i].detach();
            }
            m_threads.clear();
        }
    }
    /** called before destroy this */
    void setOnDestroyFunc(std::function<void(Callback*,Executor*,Executor*)> func){
        m_func_destroy = FUNC_MAKE_SHARED_PTR_3(void(Callback*,Executor*,Executor*), func);
    }

    void setFinalTask(std::function<void()> func){
        m_func_final = func;
    }

public:
    void onSuccess() override {
        static int _c_os = 0;
        PRINTLN("onSuccess: %d\n", _c_os++);
        Callback* tcb = nullptr;
        m_mtx_cb.lock();
        tcb = m_callback;
        m_mtx_cb.unlock();
        if(tcb && tcb->onSuccess){
            if(m_exe_cb){
                m_exe_cb->addTask([this, tcb](){
                    if(tcb){
                        tcb->onSuccess(m_lastTask.name);
                    }
                    startInternal();
                });
                return;
            }else{
                tcb->onSuccess(m_lastTask.name);
            }
        }
        startInternal();
    }
    void onFailed(int code,CString msg) override{
        static int _c_of = 0;
        PRINTLN("onFailed: %d\n", _c_of++);
        Callback* tcb = nullptr;
        m_mtx_cb.lock();
        tcb = m_callback;
        m_mtx_cb.unlock();
        if(tcb && tcb->onFail){
            if(m_exe_cb){
                m_exe_cb->addTask([this, tcb, code, msg](){
                    if(tcb){
                        tcb->onFail(m_lastTask.name, code, msg);
                    }
                    clearUp();
                });
                return;
            }else{
                m_callback->onFail(m_lastTask.name,code, msg);
            }
        }
        clearUp();
    }
    void onStopped()override{
        static int _c_ost = 0;
        PRINTLN("onStopped: %d\n", _c_ost++);
        Callback* tcb = nullptr;
        m_mtx_cb.lock();
        tcb = m_callback;
        m_mtx_cb.unlock();
        if(tcb && tcb->onStop){
            if(m_exe_cb){
                m_exe_cb->addTask([this, tcb](){
                    if(tcb){
                        tcb->onStop(m_lastTask.name);
                    }
                    clearUp();
                });
                return;
            }else{
                m_callback->onStop(m_lastTask.name);
            }
        }
        clearUp();
    }
    bool isStopped()override{
        return h_atomic_get(&m_stopped) == 1;
    }
private:
    struct Task{
        String name;
        std::function<void()> func;
    };
    volatile int m_started;
    volatile int m_stopped;
    //FUNC_SHARED_PTR(void()) m_startFunc;
    Task m_lastTask;
    std::list<Task> m_tasks;
    std::mutex m_mtx_tasks;
    std::mutex m_mtx_cb;
    Callback* m_callback {nullptr};
    Executor* m_exe_schedule {nullptr};
    Executor* m_exe_cb {nullptr};
    std::vector<std::thread> m_threads;
    std::function<void()> m_func_final;
    FUNC_SHARED_PTR(void(Callback*,Executor*,Executor*)) m_func_destroy;

    void callFinalTask(){
        if(m_func_final){
            m_func_final();
        }
    }
    void clearUp(){
        {
            std::unique_lock<std::mutex> lock(m_mtx_tasks);
            m_tasks.clear();
        }
        h_atomic_set(&m_started, 0);
        h_atomic_set(&m_stopped, 0);
        callFinalTask();
    }
    void startInternal(){
        bool end = false;
        {
            std::unique_lock<std::mutex> lock(m_mtx_tasks);
            if(m_tasks.size() > 0){
                m_lastTask = std::move(m_tasks.front());
                m_tasks.pop_front();
            }else{
                end = true;
            }
        }
        if(end){
            callFinalTask();
            return;
        }
        if(m_exe_schedule){
            m_exe_schedule->addTask([this](){
                m_lastTask.func();
            });
        }else{
            m_lastTask.func();
        }
    }
};

}

#endif // ASYNCS_H
