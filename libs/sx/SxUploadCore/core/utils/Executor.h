#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <thread>
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <vector>
#include "common/common.h"
#include "utils/h_atomic.h"
#include "common/SkRefCnt.h"

namespace h7 {

template <typename T>
class IColumn;

/**
  demos:
  ```
   Executor exe(4);
   exe.prepare();
   exe.addTask([](){
    //...
    });
    exe.waitFinish();
  ```
 or
 ```
 Executor exe(4);
 exe.setTasksAndRun(...);
 exe.waitFinish();
 ```

 * @brief The Executor class: which used for multi threads.
 */
class Executor : public SkRefCnt{
    public:
        Executor(int tc):m_threadCount(tc){
            m_threads.reserve(tc);
        }

        ~Executor(){
            if(h_atomic_cas(&m_cmd, kReqStop, kReqNormalExit)){
                for(int i = 0 ; i < (int)m_threads.size() ; ++i){
                    m_threads[i].join();
                }
                m_threads.clear();
            }
        }
        inline void waitFinish(){
            if(h_atomic_cas(&m_cmd, kNone, kReqNormalExit)){
                for(int i = 0 ; i < (int)m_threads.size() ; ++i){
                    m_threads[i].join();
                }
                m_threads.clear();
            }
//            h_atomic_set(&m_cmd, kReqNormalExit);
//            for(int i = 0 ; i < (int)m_threads.size() ; ++i){
//                m_threads[i].join();
//            }
//            m_threads.clear();
        }
        inline void finish(){
            if(h_atomic_cas(&m_cmd, kNone, kReqNormalExit)){
                for(int i = 0 ; i < (int)m_threads.size() ; ++i){
                    m_threads[i].detach();
                }
                m_threads.clear();
            }
//            h_atomic_set(&m_cmd, kReqNormalExit);
//            for(int i = 0 ; i < (int)m_threads.size() ; ++i){
//                m_threads[i].detach();
//            }
//            m_threads.clear();
        }
        /**
         * set the all tasks and run directly. unlike addTask. this no need prepare.
         */
        template<typename T>
        void setTasksAndRun(h7::IColumn<T>* list,
                      std::function<void(T&,int)> func);

        //often used before addTask
        void prepare();

        /**
         * add a task to thread-group. unlike 'setTasksAndRun' this need prepare.
         */
        template<class F, class... Args>
        void addTask(F&& f, Args&&... args);
           // -> std::future<typename std::result_of<F(Args...)>::type>;
        inline void stop(){
            h_atomic_set(&m_cmd, kReqStop);
            m_condition.notify_all();
        }
        //this can only be called if all tasks run done, or else may cause bug
        inline void reset(){
            m_reqCount = 0;
            h_atomic_set(&m_finishCount, 0);
            h_atomic_set(&m_cmd, kNone);
        }
        inline int getReqTaskCount(){
            return m_reqCount;
        }
        inline int getFinishTaskCount(){
            return h_atomic_get(&m_finishCount);
        }
        inline void setContext(void* ctx){
            m_ctx = ctx;
        }
        inline void* getContext(){
            return m_ctx;
        }
        bool isStopped(){
            return h_atomic_get(&m_cmd) == kReqStop;
        }
        //set on progress listener. should called before setTasksAndRun/addTask
        void setOnProgressUpdateListener(std::function<void(h7::Executor*)> func){
            m_listener = FUNC_MAKE_SHARED_PTR_1(void(h7::Executor*), func);
        }
    private:
        enum{
            kNone = 0,
            kReqNormalExit,
            kReqStop,
        };
        std::vector<std::thread> m_threads;
        int m_threadCount;
        int m_reqCount {0};
        volatile int m_finishCount {0};
        volatile int m_cmd {kNone};
        std::queue<std::function<void()> > m_tasks;
        std::mutex m_mutex; //the mutext of tasks
        std::condition_variable m_condition;
        void* m_ctx {nullptr};
        FUNC_SHARED_PTR(void(h7::Executor*)) m_listener;
        std::mutex m_cb_mutex;
    };
}

#include "utils/Executor.inc"

#endif // EXECUTOR_H
