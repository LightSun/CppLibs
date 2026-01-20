#include "handler-os/TaskFlow.h"
#include "handler-os/src/TaskFlowImpl.h"
#include "core/common/common.h"

using namespace h7_task;

TaskFlow::TaskFlow(size_t bufQueueCnt){
    m_ctx = new TaskFlow_Ctx(bufQueueCnt);
}

TaskFlow::~TaskFlow(){
    if(m_ctx){
        delete m_ctx;
        m_ctx = nullptr;
    }
}

SpTask TaskFlow::createTask(){
    return m_ctx->createTask();
}

void TaskFlow::addCmdTask(SpCmdTask task){
    //MED_ASSERT(task->curTask && task->curTask->func);
    m_ctx->addCmdTask(task);
}

void TaskFlow::addTask(SpTask task){
    MED_ASSERT(task->func);
    m_ctx->addTask(task);
}
void TaskFlow::addCancelTask(ID id){
    m_ctx->cancelTask(id);
}

bool TaskFlow::start(IScheduler* sch){
    return m_ctx->start(sch);
}

void TaskFlow::stop(){
    m_ctx->stop();
}

void TaskFlow::setOnTaskStateChanged(std::function<void(SpTask,int,int)> func){
    m_ctx->setOnTaskStateChanged(func);
}
void TaskFlow::setOnBatchTaskReqCancel(std::function<void(CList<ID>, SpTask)> func){
    m_ctx->setOnBatchTaskReqCancel(func);
}
void TaskFlow::setOnBatchTaskFailed(std::function<void(CList<ID>, SpTask)> func){
    m_ctx->setOnBatchTaskFailed(func);
}
void TaskFlow::setRpcCacheDelegate(std::shared_ptr<IRpcCacheDelegate> del){
    m_ctx->setRpcCacheDelegate(del);
}
