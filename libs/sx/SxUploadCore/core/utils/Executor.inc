
#include "utils/Executor.h"
#include "table/Column.h"

namespace h7{

template<typename T>
void Executor::setTasksAndRun(h7::IColumn<T>* list,
              std::function<void(T&,int)> func){
    const int tc = m_threadCount;
    int every = list->size() / tc + (list->size() % tc != 0 ? 1 : 0);
    auto tasks = list->groupByEveryCount(every);

    MED_ASSERT(tasks->size() <= tc);
    m_reqCount = tasks->size();

    for (int i = 0 ; i < tasks->size(); i ++){
        auto& ts = tasks->get(i);
        int offset = i * every;
        //if use std::function with future it will blocked.
        //same thread can only share one 'ptr'. multi thread can't
        FUNC_SHARED_PTR(void(T&,int)) ptr = FUNC_MAKE_SHARED_PTR_2(
                    void(T&,int), func);
        m_threads.emplace_back([this, &ts, ptr, i, offset](){
            PRINTLN("---> start thread: %d\n", i);
            int idx;
            for(int k = 0; k < ts->size(); ++k){
                idx = offset + k;
                (*ptr)(ts->get(k), idx);
                ptr->reset();
                if(h_atomic_get(&m_cmd) == kReqStop){
                    PRINTLN("---> end thread(req stop): %d\n", i);
                    return;
                }
            }
            if(h_atomic_get(&m_cmd) == kReqStop){
                PRINTLN("---> end thread(req stop): %d\n", i);
                return;
            }
            PRINTLN("---> end thread: %d\n", i);
            h_atomic_add(&m_finishCount, 1);
            if(m_listener){
                std::unique_lock<std::mutex> lock(m_cb_mutex);
                (*m_listener)(this);
                m_listener->reset();
            }
        });
    }
}

template<class F, class... Args>
void Executor::addTask(F&& f, Args&&... args)
   // -> std::future<typename std::result_of<F(Args...)>::type>
{
    m_reqCount ++;
    //using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared< std::packaged_task<void()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

    {
        std::unique_lock<std::mutex> lock(m_mutex);

        // don't allow enqueueing after stopping the pool
        if(h_atomic_get(&m_cmd) == kReqStop){
            return;
        }
        m_tasks.emplace([this, task](){
            if(h_atomic_get(&m_cmd) == kReqStop){
                return;
            }
            (*task)();
            if(h_atomic_get(&m_cmd) == kReqStop){
                return;
            }
            //check if need quit
            int preVal = h_atomic_add(&m_finishCount, 1);
            if(m_listener){
                std::unique_lock<std::mutex> lock(m_cb_mutex);
                (*m_listener)(this);
                m_listener->reset();
            }
            if(h_atomic_get(&m_cmd) == kReqNormalExit
                    && preVal == m_reqCount - 1){
                stop();
            }
        });
    }
    m_condition.notify_one();
}

}
