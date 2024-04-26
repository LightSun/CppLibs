#include "utils/Callbacks.h"

using namespace h7;

namespace test {
struct Packet0{
    void* ps;
};
struct Packet1{
    void* ps;
};
struct Callback0{
    void onCallback(std::shared_ptr<Packet0> pkg){
        printf("Callback0 >> onCallback -> Packet0\n");
    }
};

struct Invoker0{
    void operator()(Callback0* c, std::shared_ptr<Packet0> pkg){
        c->onCallback(pkg);
    }
    void operator()(Callback0* c, std::shared_ptr<Packet1> pkg){
        printf("Callback0 >> onCallback -> Packet1\n");
    }
    void handle(Callback0* c, std::shared_ptr<Packet0> pkg){
        printf("Invoker0 >> handle -> Packet0\n");
    }
};
}

using namespace test;

void test_Callbacks(){
    CallbackWrapper<Callback0, Invoker0> cb;
    cb.sendPacket(std::make_shared<Packet0>());
    cb.sendNewPacket<Packet1>();
    //
    CallbackWrapper2<Callback0, Packet0> cb2;
    Invoker0 inc;
    cb2.sendNewPacket(&inc);
}
