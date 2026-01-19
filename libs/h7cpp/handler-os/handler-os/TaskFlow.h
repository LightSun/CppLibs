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
};
using SpData = std::shared_ptr<void*>;

using ID = size_t;

class IScheduler{
public:
    virtual ~IScheduler(){};
    virtual void schedule(CString tag,std::function<void()> t) = 0;
};

struct Task{
    String tag;
    std::function<SpData(const std::map<int,SpData>& in)> func;
    //effect the all tasks which dep this.
    int publicNoticePeriodMs {500};
    IScheduler* scheduler {nullptr};

    void run(){
        result = func(input);
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

private:
    friend class TaskFlow_Ctx;
    //
    std::map<int,SpData> input;
    SpData result;
    std::atomic_int state {kState_IDLE};
    std::atomic_bool reqStop {false};
    ID id {0};
};
using SpTask = std::shared_ptr<Task>;
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
using SpCmdTask = std::shared_ptr<CmdTask>;

//callback, id-limit
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

private:
    TaskFlow(TaskFlow&) = delete;
    TaskFlow(const TaskFlow&) = delete;
    TaskFlow& operator=(TaskFlow&) = delete;
    TaskFlow& operator=(const TaskFlow&) = delete;
private:
    TaskFlow_Ctx* m_ctx {nullptr};

};

}
