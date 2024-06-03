#include "core/utils/ConcurrentHashMap.h"
#include "core/utils/CountDownLatch.h"
#include <thread>

using namespace h7;

#define LOOP_C 100

void test_ConcurrentHashMap(){
    printf("---------- test test_ConcurrentHashMap start>> ...\n");
    CountDownLatch cdt(1);
    ConcurrentHashMap<int, std::string, 10> map;
    std::atomic<int> ato_count;

    for(int i = 0 ; i < LOOP_C ; ++i){
        const int key = i;
        std::thread thd([&map, &ato_count, &cdt, key](){
            map.insert(key, std::to_string(key));
            if(ato_count.fetch_add(1) == LOOP_C - 1){
                cdt.countDown();
            }
        });
        thd.detach();
    }
    cdt.await();
    std::string value;
    if (map.find(2, value)) {
        std::cout << "Found value: " << value << std::endl;
    } else {
        std::cout << "Value not found" << std::endl;
    }
    printf("---------- test test_ConcurrentHashMap end >> ...\n");
}
