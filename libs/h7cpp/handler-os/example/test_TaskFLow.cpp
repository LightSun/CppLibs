#include "handler-os/TaskFlow.h"

namespace h7_task {

struct TaskFlowTester{
    TaskFlow flow;

    void test1(){
        auto t1 = flow.createTask();
        t1->tag = "test1";
//        t1->func = [](){

//        };
    }
};

}
