#include "utils/Asyncs.h"

using namespace h7;

static void test1();
static void test2();
static void test3();
static void test4();

int test_main0(int argc, char* argv[]){
//   test1();
//   test2();
//   test3();
   //test4();
   return 0;
}
//TODO share executor on multi Asyncs, not done.
void test4(){
    PRINTLN("------ > test4 >> \n");
    Executor* exe = new Executor(1);
    Executor* exe2 = new Executor(1);
    exe->prepare();
    exe2->prepare();
    Asyncs::Callback cb;
    cb.onFail = [](CString name, int code, CString msg){
        PRINTLN("test4 >> onFail.\n");
    };
    sk_sp<h7::Asyncs> ins = sk_make_sp<h7::Asyncs>();
    sk_sp<h7::Asyncs> ins2 = sk_make_sp<h7::Asyncs>();
    {
        ins->scheduleOn(exe);
        ins->observerOn(exe2);
        ins->setCallback(&cb);
        ins->of([](Asyncs::TaskPtr ptr){
            PRINTLN("test4 >> (ins) start run task 1\n");
            PRINTLN("test4 >> (ins) end run task 1\n");
            ptr->onSuccess();
        })->next([](Asyncs::TaskPtr ptr){
            PRINTLN("test4 >> (ins) start run task 2\n");
            PRINTLN("test4 >> (ins) end run task 2\n");
            ptr->onFailed(1, "test failed.");
        });
        ins->setFinalTask([](){
            PRINTLN("test4 >> (ins) Final Task run. \n");
        });
        ins->start();
        ins->finish();
    }
    {
        ins2->scheduleOn(exe);
        ins2->observerOn(exe2);
        ins2->setCallback(&cb);
        ins2->setOnDestroyFunc([](Asyncs::Callback*,Executor* exe_sch,
                               Executor* exe_cb){
            //delete exe_sch;
           // delete exe_cb;
        });
        ins2->of([](Asyncs::TaskPtr ptr){
            PRINTLN("test4 >> (ins2) start run task 1\n");
            PRINTLN("test4 >> (ins2) end run task 1\n");
            ptr->onSuccess();
        })->next([](Asyncs::TaskPtr ptr){
            PRINTLN("test4 >> (ins2) start run task 2\n");
            PRINTLN("test4 >> (ins2) end run task 2\n");
            ptr->onFailed(1, "test failed.");
        });
        ins2->start();
        ins2->finish();
    }
    delete exe;
    delete exe2;
}
void test3(){
    PRINTLN("------ > test3 >> \n");
    sk_sp<h7::Asyncs> ins = sk_make_sp<h7::Asyncs>();
    int exit{0};
    Asyncs::Callback cb;
    cb.onFail = [&exit](CString name, int code, CString msg){
        exit = 1;
    };
    ins->setCallback(&cb);
    Executor* exe = new Executor(1);
    Executor* exe2 = new Executor(1);
    exe->prepare();
    exe2->prepare();
    ins->scheduleOn(exe);
    ins->observerOn(exe2);

    ins->of([](Asyncs::TaskPtr ptr){
        PRINTLN("test3 >> start run task 1\n");
        PRINTLN("test3 >> end run task 1\n");
        ptr->onSuccess();
    })->next([](Asyncs::TaskPtr ptr){
        PRINTLN("test3 >> start run task 2\n");
        PRINTLN("test3 >> end run task 2\n");
        ptr->onFailed(1, "test failed.");
    });
    ins->start();

    ins->finish(true);
    delete exe;
    delete exe2;
    PRINTLN("test2 done !\n");
}
void test2(){
    PRINTLN("------ > test2 >> \n");
    sk_sp<h7::Asyncs> ins = sk_make_sp<h7::Asyncs>();
    int exit = 0;
    Asyncs::Callback cb;
    cb.onFail = [&exit](CString name, int code, CString msg){
        PRINTLN("name = %s, code = %d, msg = %s\n",
                name.data(), code, msg.data());
        exit = 1;
    };
    ins->setCallback(&cb);
    Executor* exe = new Executor(1);
    exe->prepare();
    ins->scheduleOn(exe);
    ins->of([](Asyncs::TaskPtr ptr){
        PRINTLN("test2 >> start run task 1\n");
        PRINTLN("test2 >> end run task 1\n");
        ptr->onSuccess();
    })->next([](Asyncs::TaskPtr ptr){
        PRINTLN("test2 >> start run task 2\n");
        PRINTLN("test2 >> end run task 2\n");
        ptr->onFailed(1, "test failed.");
    });
    ins->start();
    ins->finish();
    delete exe;
    PRINTLN("test2 done !\n");
}
void test1(){
    PRINTLN("------ > test1 >> \n");
    sk_sp<h7::Asyncs> ins = sk_make_sp<h7::Asyncs>();
    ins->of([](Asyncs::TaskPtr ptr){
        PRINTLN("start run task 1\n");
        for(int i = 0 ; i < 100 ; ++i){
            PRINTLN("%d,", i);
        }
        PRINTLN("\n");
        PRINTLN("end run task 1\n");
        ptr->onSuccess();
    })->next([](Asyncs::TaskPtr ptr){
        PRINTLN("start run task 2\n");
        for(int i = 0 ; i < 100 ; ++i){
            PRINTLN("%d,", i);
        }
        PRINTLN("\n");
        PRINTLN("end run task 2\n");
        ptr->onSuccess();
    });
    ins->start();
    ins->finish();
    PRINTLN("test1 done !\n");
}
