#include "handler-os/TaskFlow.h"
#include "handler-os/HandlerOS.h"

namespace h7_task {

class SchedulerImpl: public IScheduler{
public:
    h7_handler_os::HandlerOS* hos {nullptr};

    SchedulerImpl(h7_handler_os::HandlerOS* h):hos(h){}

    void schedule(CString tag,std::function<void()> t) override{
        hos->getHandler()->post(t, tag);
    }
};

struct TaskFlowTester{
    TaskFlow flow;
    h7_handler_os::HandlerOS* handlerOS {nullptr};
    std::unique_ptr<SchedulerImpl> m_scheduler;

    TaskFlowTester(){
        handlerOS = new h7_handler_os::HandlerOS();
        handlerOS->start([](h7_handler_os::Message* msg){
            //TODO
            return true;
        });
        m_scheduler = std::make_unique<SchedulerImpl>(handlerOS);
        //start
        flow.setOnTaskStateChanged([](SpTask t,int oldS,int newS){
            printf("TaskStateChanged >> %s: %d -> %d\n",
                   t->tag.data(), oldS, newS);
        });
        flow.start(m_scheduler.get());
    }
    ~TaskFlowTester(){
        flow.stop();
        if(handlerOS){
            handlerOS->quitSafely();
            handlerOS = nullptr;
        }
    }

    void test_no_dep(){
        //
        auto t1 = flow.createTask();
        t1->tag = "test_no_dep";
        t1->func = [](void*,const std::map<int,SpData>& in, SpData* out){
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            auto ptr = new String("Hello-Heaven7");
            *out = make_shared_void_ptr(ptr);
            return true;
        };
        flow.addTask(t1);
    }
    void test_dep1(){
        auto t1 = flow.createTask();
        t1->tag = "test_dep1";
        t1->func = [](void*,const std::map<int,SpData>& in, SpData* out){
            for(auto& [k,v] : in){
                printf("test_dep1 >> index = %d, data = %s\n",
                       k, ((String*)v.get())->data());
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            auto ptr = new String("Hello-Heaven7");
            *out = make_shared_void_ptr(ptr);
            return true;
        };
        auto task = CmdTask::New();
        task->depTasks.push_back(ceateSimpleTask("task_0"));
        task->curTask = t1;
        flow.addCmdTask(task);
    }

private:
    SpTask ceateSimpleTask(CString tag){
        auto t1 = flow.createTask();
        t1->tag = tag;
        t1->func = [tag](void*,const std::map<int,SpData>& in, SpData* out){
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            auto ptr = new String(tag + "__ret");
            *out = make_shared_void_ptr(ptr);
            return true;
        };
        flow.addTask(t1);
        return t1;
    }
};

}

void test_TaskFlow(){
    h7_task::TaskFlowTester tester;
   // tester.test_no_dep();
    tester.test_dep1();
    h7_handler_os::HandlerOS::loopMain();
}
