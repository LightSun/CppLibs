#include "utils/LockFreeHashMap.h"
#include "utils/CountDownLatch.h"
#include "utils/times.h"
#include "utils/Random.h"
#include <atomic>

using Map = LockFreeHashMap<int,std::string>;
using String = std::string;

static void run_remove(Map* map);
static void run_find(Map* map);
static void run_insert(Map* map);
typedef void (*Func_Run)(Map* map);

struct Runner{
    std::atomic_int c;
    h7::CountDownLatch cdt{1};
    Map map{1024};

    void addDefault(){
        for(int i = 0 ; i < 512; ++i){
            map.insert(i, std::to_string(i));
        }
    }

    void runAll(std::vector<Func_Run> funcs){
        c = funcs.size();
        for(auto& func : funcs){
            asyncRunTarget(func);
        }
        cdt.await();
        printf("runAll >> \n");
        map.traverse([](const int& k, const String& v){
            printf("  k = %d, val = %s\n", k, v.data());
        });
        printf("runAll >> ------ all done.\n");
    }
    void asyncRunTarget(Func_Run func){
        std::thread thd([this, func](){
            func(&map);
            if(c.fetch_sub(1, std::memory_order_relaxed) == 1){
                printf("asyncRunTarget:: all done!.\n");
                cdt.countDown();
            }
        });
        thd.detach();
    }
};


void test_LockFreeHashMap(){

    Runner runner;
    std::vector<Func_Run> funcs = {run_insert, run_find, run_remove};
    runner.addDefault();
    runner.runAll(funcs);
}
//---------------------
void run_remove(Map* map){
    for(int i = 5 ; i < 1024; ++i){
        if(map->remove(i)){
            printf("run_remove >> remove ok. i = %d.\n", i);
        }
        SLEEP(h7::Random::nextInt(0, 15));
    }
}
void run_find(Map* map){
    for(int i = 0 ; i < 512; ++i){
        String val;
        if(map->find(i, val)){
            printf("run_find >> i = %d, val = %s\n", i, val.data());
        }else{
            printf("run_find >> failed. i = %d\n.", i);
        }
        SLEEP(h7::Random::nextInt(0, 15));
    }
}
void run_insert(Map* map){
    for(int i = 0 ; i < 512; ++i){
        map->insert(i, std::to_string(i));
        SLEEP(h7::Random::nextInt(0, 15));
    }
}
