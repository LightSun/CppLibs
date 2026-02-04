#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include <functional>
#include <memory>
#include <atomic>
#include <shared_mutex>
#include "handler-os/rpc_api.h"

namespace h7_task {

template<typename T>
using List = std::vector<T>;

template<typename T>
using CList = const std::vector<T>&;

using String = std::string;
using CString = const std::string&;

//---------------------------------
enum TaskState{
    kState_IDLE,
    kState_PENDING, //pending means id is prepared, but not scheduled.
    kState_SCHEDULE,
    kState_RUNING,
    kState_DONE,
    kState_CANCELED,
    kState_FAILED,
};
enum TDataType{
    kTDataType_DEFAULT,
};
struct TData{
    std::shared_ptr<void> data;
    int type {kTDataType_DEFAULT};

    void* getPtr()const{
        return data.get();
    }
};

class RemoteGRpc;
struct Task;
struct CmdTask;
//
using SpData = TData;
using ID = size_t;
using SpTask = std::shared_ptr<Task>;
using SpCmdTask = std::shared_ptr<CmdTask>;

template<typename T>
std::shared_ptr<void> make_shared_void_ptr(T* ptr) {
    return std::shared_ptr<void>(ptr, [](void* p) {
        delete static_cast<T*>(p);
    });
}

class IScheduler{
public:
    virtual ~IScheduler(){};
    virtual void schedule(CString tag,std::function<void()> t) = 0;
};

struct Task{
    struct GrpcInfo{
        //if one failed, will req next addr. until success or all failed.
        List<String> addrs; //with port xxx:1009, permit multi hosts.
        int type;           //req type
        size_t timeoutMs {500};
        //[0] is context,[1] is req-type
        std::function<String(void*,int,const std::map<int,SpData>& in)> cvtReq;
        std::function<bool(void*,int,CString in, SpData* out)> cvtRes;
    };

    String tag;
    //[0] is context
    std::function<bool(void*,const std::map<int,SpData>& in, SpData* out)> func;
    //effect the all tasks which dep this.
    int publicNoticePeriodMs {500};
    void* context {nullptr};
    IScheduler* scheduler {nullptr};
    std::unique_ptr<GrpcInfo> rpc;

    static inline std::unique_ptr<GrpcInfo> makeGrpcInfo(){
        return std::make_unique<GrpcInfo>();
    }

    bool isDone(){
        return getState() == kState_DONE;
    }
    void stop(){
        bool stop = false;
        reqStop.compare_exchange_strong(stop, true);
    }
    bool isReqStop(){
        return reqStop.load();
    }
    ID getId()const{
        return id;
    }
    bool isTaskIdle(){
        return getState() == kState_IDLE;
    }
    int getState(){
        return state.load();
    }
private:
    Task(){}
    void setState(int s){
        state.store(s);
    }
    void setInputAt(int index, SpData data){
        input[index] = data;
    }
    bool run(IRpcCacheDelegate* del);
private:
    friend class TaskFlow_Ctx;
    //
    std::map<int,SpData> input;
    SpData result;
    std::atomic_int state {kState_IDLE};
    std::atomic_bool reqStop {false};
    ID id {0};
};
struct CmdTask{
    List<SpTask> depTasks;
    SpTask curTask;
    ID cancelTaskId {0};

    static std::shared_ptr<CmdTask> New(){
        return std::shared_ptr<CmdTask>(new CmdTask());
    }
private:
    CmdTask(){}
};

typedef class TaskFlow_Ctx TaskFlow_Ctx;
class TaskFlow{
public:
    TaskFlow(size_t bufQueueCnt = 128);

    ~TaskFlow();

    SpTask createTask();

    void addCmdTask(SpCmdTask task);

    void addTask(SpTask task);

    //note: cancel task will make the all tasks cancelled, which are dep on this task.
    void addCancelTask(ID id);

    bool start(IScheduler* sch);

    void stop();

    void setOnTaskStateChanged(std::function<void(SpTask,int,int)> func);

    //calcel is trigged by the first task
    //[0] is the trigger task id. SpTask may be null, of not pending
    void setOnBatchTaskReqCancel(std::function<void(CList<ID>, SpTask)> func);

    void setOnBatchTaskFailed(std::function<void(CList<ID>, SpTask)> func);

    void setRpcCacheDelegate(std::shared_ptr<IRpcCacheDelegate> del);

private:
    TaskFlow(TaskFlow&) = delete;
    TaskFlow(const TaskFlow&) = delete;
    TaskFlow& operator=(TaskFlow&) = delete;
    TaskFlow& operator=(const TaskFlow&) = delete;
private:
    TaskFlow_Ctx* m_ctx {nullptr};

};

//used for simple task builder
class TaskBuilder{
    struct Node{
        List<SpTask> tasks;
        Node* next {nullptr};

        void release(){
            tasks.clear();
            if(next){
                next->release();
                delete next;
                next = nullptr;
            }
        }
        void addTask(SpTask t){
            tasks.push_back(t);
        }
    };
public:
    TaskBuilder(TaskFlow* tf):m_flow(tf){
    }
    ~TaskBuilder(){
        release();
    }
    template<typename Func>
    TaskBuilder& root(Func f){
        if(m_root != nullptr){
            abort();
        }
        m_root = m_cur = new Node();
        m_root->addTask(f(m_flow));
        return *this;
    }
    template<typename Func>
    TaskBuilder& child(Func f){
        auto nNode = new Node();
        nNode->addTask(f(m_flow));
        m_cur->next = nNode;
        m_cur = nNode;
        return *this;
    }

    template<typename FuncList>
    TaskBuilder& children(FuncList f){
        auto nNode = new Node();
        nNode->tasks = f(m_flow);
        //
        m_cur->next = nNode;
        m_cur = nNode;
        return *this;
    }
    void build(){
        Node* cur = m_root;
        while (cur != nullptr) {
            List<SpTask> nextTs;
            if(cur->next){
                nextTs = cur->next->tasks;
            }
            for(auto& t: cur->tasks){
                auto ct = CmdTask::New();
                ct->curTask = t;
                ct->depTasks = nextTs;
                m_flow->addCmdTask(ct);
            }
            cur = cur->next;
        }
        release();
    }
private:
    void release(){
        if(m_root){
            m_root->release();
            delete m_root;
            m_root = nullptr;
            m_cur = nullptr;
        }
    }

private:
    TaskFlow* m_flow {nullptr};
    Node* m_root {nullptr};
    Node* m_cur {nullptr};
};

}
