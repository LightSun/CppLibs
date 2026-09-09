#include "experimental/component/component.h"
#include "common/common.h"

namespace h7_component {

struct TestStruct{
    int version {1}; //often, we may want a version.
    String name;
    String group;
};

//a demo only support kFormat_POINTER
class TestService: public IService{

public:
    bool init(CSpContext data) override{
        MED_ASSERT(data.format == kFormat_POINTER);
        TestStruct* ts = (TestStruct*)data.data.get();
        m_info.name = ts->name;
        m_info.group = ts->group;
        return true;
    }
    bool load(CSpContext ctx) override{
        return true;
    }
    bool unload(CSpContext ctx) override{
        return true;
    }
    bool run(CSpContext ctx, CSpContext saveState) override{
        printf("[%s] run >> start infer...\n", m_info.name.data());
        int s = 2;
        for(int i = 1 ; i < 1000 ; ++i){
            s += i;
        }
        printf("[%s] run >> end infer... s = %d\n", m_info.name.data(), s);
        return true;
    }
    //----------------
    bool saveState(SpContext&)override{
        return false;
    }
    bool restoreState(const SpContext&)override{
        return false;
    }
};

class TestListener: public IServiceListener{
public:
    void onEvent(System*, CServiceEnvent event)override{
        auto estr = int2str_event(event.event);
        printf("onEvent >> (name, event, state) = (%s, %s, %d)\n",
             event.name.data(), estr.data(), event.state);
    }
    void onGroupEvent(System*, CServiceEnvent event)override{
        //in group event. name is empty.
        auto estr = int2str_event(event.event);
        auto ss = event.state ? "open" : "close";
        printf("onGroupEvent >> (name, event, state) = (%s, %s, %s)\n",
               event.name.data(), estr.data(), ss);
    };
};
}

void test_component(){
    using namespace h7_component;
    System sys;
    sys.setServiceListener(std::make_unique<TestListener>());
    auto s1 = std::make_shared<TestService>();
    auto s2 = std::make_shared<TestService>();
    TestStruct ts1; ts1.name = "TestService_1"; ts1.group = "heaven7";
    TestStruct ts2; ts2.name = "TestService_2"; ts2.group = "heaven7";
    SpContext env1;
    env1.format = kFormat_POINTER;
    env1.data = std::shared_ptr<void>(&ts1, [](void*){
        //ignore
    });
    SpContext env2;
    env2.format = kFormat_POINTER;
    env2.data = std::shared_ptr<void>(&ts2, [](void*){
        //ignore
    });
    auto r1 = sys.registerService(s1, env1);
    auto r2 = sys.registerService(s2, env2);
    MED_ASSERT(r1.first);
    MED_ASSERT(r2.first);
    sys.group().runAll(SpContext(), SpContext());
}
