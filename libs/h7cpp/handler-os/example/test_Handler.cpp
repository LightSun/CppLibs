#include "handler-os/HandlerThread.h"
#include "handler-os/Handler.h"
#include "handler-os/Object.h"
#include "handler-os/Message.h"
#include "../src/_common_pri.h"

using namespace h7_handler_os;

struct Manager{
    HandlerThread* ht {nullptr};
    std::shared_ptr<Handler> handler;

    ~Manager(){
        if(ht){
            delete ht;
            ht = nullptr;
        }
    }
};

static void testHandler0();
static void testHandler1();
static void testHandler2();

#define MSG_testHandler1 1

void testHandler(){
    testHandler0();
    testHandler1();
    testHandler2();
}

void testHandler0(){
    Manager* man = new Manager();
    auto ht = new HandlerThread();
    man->ht = ht;
    ht->setAfterQuitCallback([man](){
        delete man;
    });
    ht->start();
    man->handler = std::shared_ptr<Handler>(new Handler(ht->getLooper()));

    man->handler->post([](){
        auto str = "task runned. tid = " +  _cur_tid_tostring();
        _LOG_INFO(str);
    });
    sleep_ms(3000);
    ht->quit();
}
void testHandler2(){
    Manager* man = new Manager();
    auto ht = new HandlerThread();
    man->ht = ht;
    ht->setAfterQuitCallback([man](){
        delete man;
    });
    ht->start();
    man->handler = std::shared_ptr<Handler>(new Handler(ht->getLooper()));

    //run until callback
    man->handler->runWithScissors([](){
        auto str = "testHandler2 >> runWithScissors callback runned.";
        _LOG_INFO(str);
    }, 50);
    _LOG_INFO("testHandler2 >> start quit !");
    ht->quit();
}
//----------------------------

struct Student{
    std::string name;
    int age {0};
    Student(){}
    Student(const std::string& name):name(name){}
};

//will be called when a Message is recycle.
static void _testHandler1_free(const std::string& tag, void* ptr){
    _LOG_INFO(" func >> _testHandler1_free: ");
    Student* stu = (Student*)ptr;
    delete stu;
}

static void* _testHandler1_copy(const std::string& tag, void* ptr,
                               std::string* out_tag){
    _LOG_INFO(" func >> _testHandler1_copy");
    Student* stu = (Student*)ptr;
    _HANDLER_ASSERT(stu->name == "Heaven7");
    _HANDLER_ASSERT(tag == "stu1");
    Student* s2 = new Student();
    s2->age = stu->age;
    s2->name = stu->name;
    *out_tag = "stu2";
    return s2;
}

static bool _testHandler1_eq(const Object* o1, const Object* o2){
    Student* s1 = o1->ptrAs<Student>();
    Student* s2 = o2->ptrAs<Student>();
    return s1->name == s2->name;
}

void testHandler1(){
    Manager* man = new Manager();
    auto ht = new HandlerThread();
    man->ht = ht;
    ht->setAfterQuitCallback([man](){
        delete man;
    });
    ht->start();

    auto hc = HandlerCallback::make([](Message* m){
            switch (m->what) {
            case MSG_testHandler1:{
                _LOG_INFO("handleMessage >> rec MSG_testHandler1");
                _HANDLER_ASSERT(m->obj.ptr);
                Student* s2 = m->obj.ptrAs<Student>();
                _HANDLER_ASSERT(s2->name == "Heaven7");
                _HANDLER_ASSERT(m->obj.tag == "stu2");
            }break;
            }
        return true;
    });

    man->handler = std::shared_ptr<Handler>(new Handler(ht->getLooper(), hc));

    auto obj = Object::makeCopyEqual("stu1", new Student("Heaven7"),
                          _testHandler1_free,
                          _testHandler1_copy,
                          _testHandler1_eq);
    man->handler->obtainMessage(MSG_testHandler1, &obj)->sendToTarget();
    //wait handle message
    sleep_ms(3000);
    ht->quit();
    Message::logDebugInfo();
}
