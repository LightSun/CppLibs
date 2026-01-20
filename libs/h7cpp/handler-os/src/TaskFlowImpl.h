#pragma once

#include "handler-os/TaskFlow.h"
#include "core/utils/locks.h"
#include "core/utils/SaveQueue.h"
#include "handler-os/src/rpc/RemoteGRpc.h"

namespace h7_task {

class RpcCacheDelegateImpl : public IRpcCacheDelegate{
public:
    IRemoteGRpc* get(CString addr, size_t timeoutMs) override{
        std::unique_lock<std::mutex> lck(m_mtx);
        auto it = m_cache.find(addr);
        if(it != m_cache.end()){
            return it->second.get();
        }
        auto ptr = std::make_unique<RemoteGRpc>(addr, timeoutMs);
        auto retPtr = ptr.get();
        m_cache[addr] = std::move(ptr);
        return retPtr;
    }
private:
    std::unordered_map<String, std::unique_ptr<RemoteGRpc>> m_cache;
    std::mutex m_mtx;
};

//callback, id-limit
class TaskFlow_Ctx{
    //be-dep
    struct BeDepInfo{
        //[k,v]: k is the parent-id, v is the index of parent-dep list.
        std::unordered_map<ID,size_t> pIdMap;

        void add(ID id, size_t index){
            pIdMap[id] = index;
        }
        void remove(ID id){
            pIdMap.erase(id);
        }
        bool isEmpty()const{
            return pIdMap.empty();
        }
    };
public:
    TaskFlow_Ctx(size_t bufCnt = 128){
        m_cmds = std::make_unique<h7::SaveQueue<SpCmdTask>>(tableSizeFor(bufCnt));
        m_rpcCacheDelegate = std::make_unique<RpcCacheDelegateImpl>();
    }
    ~TaskFlow_Ctx(){
        stop();
    }

    SpTask createTask();

    void addCmdTask(SpCmdTask task){
        while (true) {
            if(m_cmds->enqueue(task)){
                break;
            }
        }
    }
    void addTask(SpTask task){
        auto cmd = CmdTask::New();
        cmd->curTask = task;
        addCmdTask(cmd);
    }
    //note: cancel task will make the all tasks cancelled, which are dep on this task.
    void cancelTask(ID id){
        auto cmd = CmdTask::New();
        cmd->cancelTaskId = id;
        addCmdTask(cmd);
    }
    bool start(IScheduler* sch);

    void stop(){
        bool stop = false;
        if(m_reqStop.compare_exchange_strong(stop, true)){
            m_stopLock.wait();
        }
    }

    void setOnTaskStateChanged(std::function<void(SpTask,int,int)> func){
        m_cb_stateChanged = func;
    }
    //calcel is trigged by the first task
    //[0] is the trigger task id. SpTask may be null, of not pending
    void setOnBatchTaskReqCancel(std::function<void(CList<ID>, SpTask)> func){
        m_cb_batchTaskReqCancel = func;
    }
    void setOnBatchTaskFailed(std::function<void(CList<ID>, SpTask)> func){
        m_cb_batchTaskFailed = func;
    }
    void setRpcCacheDelegate(std::shared_ptr<IRpcCacheDelegate> del){
        m_rpcCacheDelegate = del;
    }

private:
    void run0(IScheduler* sch);

    bool isAllDepFinished(ID id){
        for(auto& it : m_depMap[id]){
            if(m_doneIdMap.find(it) == m_doneIdMap.end()){
                return false;
            }
        }
        return true;
    }

    void processPendingTask(SpCmdTask t);
    void scheduleTasks(IScheduler* sch);
    void processRunFinish();
    void removeTasks();

private:
    void onRunDone0(SpTask t);
    void doCancel(CList<ID> ids);
    void computeRemoveIds(ID id, List<ID>& rmids);

    void setTaskState(SpTask task, int targetState){
        if(m_cb_stateChanged){
            auto old = task->getState();
            task->setState(targetState);
            m_cb_stateChanged(task, old, targetState);
        }else{
            task->setState(targetState);
        }
    }
    static inline int tableSizeFor(int cap) {
        const int MAXIMUM_CAPACITY = 1073741824;
        int n = cap - 1;
        n |= ((unsigned int)n >> 1);
        n |= ((unsigned int)n >> 2);
        n |= ((unsigned int)n >> 4);
        n |= ((unsigned int)n >> 8);
        n |= ((unsigned int)n >> 16);
        return (n < 0) ? 1 : (n >= MAXIMUM_CAPACITY) ? MAXIMUM_CAPACITY : n + 1;
    }

private:
    std::atomic_ulong m_Id {0};
    std::unique_ptr<h7::SaveQueue<SpCmdTask>> m_cmds;
    //
    std::map<ID, SpTask> m_taskMap;
    std::unordered_map<ID, long long> m_doneIdMap;
    std::unordered_map<ID, List<ID>> m_depMap;
    std::unordered_map<ID, BeDepInfo> m_beDepMap;
    //
    std::function<void(SpTask,int,int)> m_cb_stateChanged;
    //for batch cancel. SpTask may be null. if not pending
    std::function<void(CList<ID>,SpTask)> m_cb_batchTaskReqCancel;
    std::function<void(CList<ID>,SpTask)> m_cb_batchTaskFailed;
    std::shared_ptr<IRpcCacheDelegate> m_rpcCacheDelegate;
    //
    h7::MutexLock m_stopLock;
    std::atomic_bool m_started {0};
    std::atomic_bool m_reqStop {0};
};

}
