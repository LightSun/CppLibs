#include <limits>
#include <thread>

#include "handler-os/src/TaskFlowImpl.h"
#include "handler-os/src/_time.h"
#include "handler-os/src/rpc/RemoteGRpc.h"

using namespace h7_task;

#define INFO(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) fprintf(stderr,fmt, ##__VA_ARGS__)
#define WARN(fmt, ...) fprintf(stderr,fmt, ##__VA_ARGS__)

bool Task::run(IRpcCacheDelegate* del){
    if(rpc){
        assert(del);
        for(auto& addr: rpc->addrs){
            auto rpcApi = del->get(addr, rpc->timeoutMs);
            if(rpcApi != nullptr){
                String data = rpc->cvtReq(context, rpc->type, input);
                String outData;
                if(rpcApi->invoke(rpc->type, data, &outData)){
                    if(rpc->cvtRes(context, rpc->type, outData, &result)){
                        return true;
                    }else{
                        INFO("Task(%s) >> run with rpc(%s) failed. caused by call cvtRes(...) failed.\n",
                             tag.data(), addr.data());
                    }
                }else{
                    INFO("Task(%s) >> run with rpc(%s) failed. caused by invoke rpc failed.\n",
                         tag.data(), addr.data());
                }
            }
        }
        WARN("Task(%s) >> run with rpc failed. caused by call cvtRes(...) failed.\n",
             tag.data());
        return false;
    }else{
        if(func(context, input, &result)){
            return true;
        }
        WARN("Task(%s) >> run with local func failed.\n", tag.data());
        return false;
    }
}

SpTask TaskFlow_Ctx::createTask(){
    auto old = m_Id.fetch_add(1, std::memory_order_relaxed);
    if(old >= std::numeric_limits<ID>::max() - 100){
        m_Id.store(1, std::memory_order_relaxed);
        old = 0;
    }
    SpTask task = std::shared_ptr<Task>(new Task());
    task->id = old + 1;
    return task;
}
bool TaskFlow_Ctx::start(IScheduler* sch){
    bool start = false;
    if(m_started.compare_exchange_strong(start, true)){
        std::thread thd([this, sch](){
            run0(sch);
        });
        thd.detach();
        return true;
    }
    return false;
}
void TaskFlow_Ctx::run0(IScheduler* sch){
    while (!m_reqStop.load()) {
        SpCmdTask cmd;
        if(m_cmds->dequeue(cmd)){
            processPendingTask(cmd);
        }else{
            //process finish tasks.
            processRunFinish();
            //exe
            scheduleTasks(sch);
            //rm task-info if need(public-notice-period).
            removeTasks();
        }
    }
    m_stopLock.notify();
}
void TaskFlow_Ctx::processPendingTask(SpCmdTask cmd){
    //do ins
    auto& dt = *cmd;
    if(dt.cancelTaskId == 0){
        auto& curTask = dt.curTask;
        setTaskState(curTask, kState_PENDING);
        //
        m_taskMap[dt.curTask->id] = curTask;
        for(size_t i = 0; i < dt.depTasks.size() ; ++i){
            auto& t = dt.depTasks[i];
            //
            m_taskMap[t->id] = t;
            //
            m_depMap[curTask->id].push_back(t->id);
            //be-dep
            m_beDepMap[t->id].add(curTask->id, i);
        }
    }else{
        //cancel.
        SpTask existTask;
        {
            auto it1 = m_taskMap.find(dt.cancelTaskId);
            if(it1 != m_taskMap.end()){
                existTask = it1->second;
            }
        }
        List<ID> rmids;
        rmids.push_back(dt.cancelTaskId);
        computeRemoveIds(dt.cancelTaskId, rmids);
        doCancel(rmids);
        if(m_cb_batchTaskReqCancel){
            m_cb_batchTaskReqCancel(rmids, existTask);
        }
    }
}
void TaskFlow_Ctx::scheduleTasks(IScheduler* sch){
    for(auto& [id, spt]: m_taskMap){
        if(isAllDepFinished(id) && spt->getState() == kState_PENDING){
            setTaskState(spt, kState_SCHEDULE);
            SpTask st = spt;
            //
            IScheduler* schImpl = spt->scheduler ? spt->scheduler : sch;
            schImpl->schedule(spt->tag, [this, st](){
                if(!st->isReqStop()){
                    setTaskState(st, kState_RUNING);
                    if(st->run(m_rpcCacheDelegate.get())){
                        if(!st->isReqStop()){
                            setTaskState(st, kState_DONE);
                        }else{
                            setTaskState(st, kState_CANCELED);
                        }
                    }else{
                        setTaskState(st, kState_FAILED);
                    }
                }else{
                    setTaskState(st, kState_CANCELED);
                }
                //set to null, help memory.
                st->func = nullptr;
            });
        }
    }
}
void TaskFlow_Ctx::processRunFinish(){
    std::set<ID> totalRmIds;
    for(auto& [id, spt]: m_taskMap){
        switch (spt->getState()) {
        case kState_DONE:{
            onRunDone0(spt);
        }break;

        case kState_FAILED:{
            List<ID> rmids;
            rmids.push_back(id);
            computeRemoveIds(id, rmids);
            totalRmIds.insert(totalRmIds.begin(), totalRmIds.end());
            if(m_cb_batchTaskFailed){
                m_cb_batchTaskFailed(rmids, spt);
            }
        }break;
        }
    }
    //process failed.
    for(auto& id : totalRmIds){
        m_taskMap.erase(id);
        m_doneIdMap.erase(id);
        m_depMap.erase(id);
        m_beDepMap.erase(id);
    }
}
void TaskFlow_Ctx::onRunDone0(SpTask t){
    m_doneIdMap[t->id] = h7_handler_os::getCurTime();
    //set data of dep
    {
        auto it = m_beDepMap.find(t->id);
        if(it != m_beDepMap.end()){
            for(auto& [pid,index]: it->second.pIdMap){
                auto it = m_taskMap.find(pid);
                //MED_ASSERTX(it != m_taskMap.end(), "");
                it->second->setInputAt(index, t->result);
            }
        }
    }
    //remove from m_beDepMap
    auto it = m_depMap.find(t->id);
    if(it != m_depMap.end()){
        for(auto& depId: it->second){
            m_beDepMap[depId].remove(t->id);
        }
    }
}
void TaskFlow_Ctx::removeTasks(){
    //rm empty be-dep
    {
        auto it = m_beDepMap.begin();
        for( ; it != m_beDepMap.end(); ){
            if(it->second.isEmpty()){
                it = m_beDepMap.erase(it);
            }else{
                ++it;
            }
        }
    }
    auto curT = h7_handler_os::getCurTime();
    List<ID> rmids;
    for(auto& [id, doneTime] : m_doneIdMap){
        if(m_beDepMap.find(id) == m_beDepMap.end()){
            //no task dep on this task.
            auto it = m_taskMap.find(id);
            if(it != m_taskMap.end()){
                if(curT - doneTime >= it->second->publicNoticePeriodMs){
                    rmids.push_back(id);
                }
            }else{
                WARN("can't find task. id = %lu\n", id);
            }
        }
    }
    for(auto& id : rmids){
        m_taskMap.erase(id);
        m_doneIdMap.erase(id);
        m_depMap.erase(id);
    }
}
void TaskFlow_Ctx::doCancel(CList<ID> rmids){
    //
    for(auto& id : rmids){
        //cancel
        auto tit = m_taskMap.find(id);
        if(tit != m_taskMap.end()){
            tit->second->stop();
            m_taskMap.erase(tit);
        }
        m_doneIdMap.erase(id);
        m_depMap.erase(id);
        m_beDepMap.erase(id);
    }
}
void TaskFlow_Ctx::computeRemoveIds(ID id, List<ID>& rmids){
    auto be = m_beDepMap.find(id);
    if(be != m_beDepMap.end()){
        for(auto& p: be->second.pIdMap){
            rmids.push_back(p.first);
            computeRemoveIds(p.first, rmids);
        }
    }
}
