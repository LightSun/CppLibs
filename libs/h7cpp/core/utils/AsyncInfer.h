#pragma once

#include "core/common/common.h"
#include "core/common/SmartPtr.h"
#include "core/utils/locks.h"
#include "core/utils/SaveQueue.h"

namespace h7 {

template<typename T>
struct IInferDelegate{
    virtual ~IInferDelegate(){}

    virtual bool build() = 0;
    virtual bool infer(T*) = 0;
};

template<typename T0>
class AsyncInferItem{
public:
    struct Task{
        SmartPtr<T0> data;
        bool success {true};
        std::atomic_int* tcnt {nullptr};
        MutexLock* mk {nullptr};

        void decrease(){
            if(tcnt){
                MED_ASSERT(mk);
                if(tcnt->fetch_add(-1) == 1){
                    mk->notify();
                }
            }
        }
    };
    using SpTask = std::shared_ptr<Task>;

    static typename AsyncInferItem<T0>::Task newSharedTask(SmartPtr<T0>& t){
        auto task = std::make_shared<Task>();
        task.data.reset(t);
        return task;
    }
    static typename AsyncInferItem<T0>::Task newSharedTask(){
        return std::make_shared<Task>();
    }

    //2^n
    AsyncInferItem(int maxConcurrent, std::function<void()> onThreadFinish){
        m_tasks = std::make_unique<SaveQueue<SpTask>>(maxConcurrent);
        auto ptr_finish = FUNC_MAKE_SHARED_PTR_0(void(), onThreadFinish);
        std::thread thd([this, ptr_finish](){
            bool module_build = false;
            while (!m_reqStop.load()) {
                if(m_det){
                    if(!module_build){
                        MED_ASSERT(m_det->build());
                        module_build = true;
                        printf("AsyncInferItem >> module loaded.\n");
                    }
                    SpTask task;
                    if(m_tasks.dequeue(task)){
                        task->success = m_det->infer(task->data.get());
                        task->decrease();
                    }
                }
            }
            if(ptr_finish){
                (*ptr_finish)();
            }
        });
        thd.detach();
    }
    void setInferDelegate(IInferDelegate<T0>* rec){
        this->m_det = rec;
    }
    bool stop(){
        bool last = false;
        return m_reqStop.compare_exchange_strong(last, true);
    }
    void addTask(SpTask task){
        while (!m_tasks.enqueue(task)) {
        }
    }
private:
    IInferDelegate<T0>* m_det {nullptr};
    std::atomic_bool m_reqStop {false};
    std::unique_ptr<SaveQueue<SpTask>> m_tasks;
};

template<typename T>
class AsyncInferManager2{
public:
    using InferItem = AsyncInferItem<T>;
    AsyncInferManager2(){
        m_async1 = std::make_unique<InferItem>(4,[this](){
            if(m_running.fetch_add(-1) == 1){
                m_lock.notify();
            }
        });
        m_async2 = std::make_unique<InferItem>(4, [this](){
            if(m_running.fetch_add(-1) == 1){
                m_lock.notify();
            }
        });
    }
    ~AsyncInferManager2(){
        stop();
    }
    InferItem* getAsync1(){
        return m_async1.get();
    }
    InferItem* getAsync2(){
        return m_async2.get();
    }
    void infer(SmartPtr<T>& p, MutexLock& mk){
        auto task = InferItem::newSharedTask(p);
        auto task2 = InferItem::newSharedTask();
        task2->data.ptr = task->data.get();
        //
        std::atomic_int tcnt = 2;
        task->mk = &mk;
        task->tcnt = &tcnt;
        task2->mk = &mk;
        task2->mk = &tcnt;
        m_async1->addTask(task);
        m_async2->addTask(task2);
        mk.wait();
    }
    void stop(){
        if(m_async1->stop()){
            m_async2->stop();
            m_lock.wait();
        }
    }

private:
    std::unique_ptr<InferItem> m_async1;
    std::unique_ptr<InferItem> m_async2;
    h7::MutexLock m_lock;
    std::atomic_int m_running {2};
};

}
