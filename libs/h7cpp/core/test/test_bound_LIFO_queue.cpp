#include <thread>
#include <iostream>
#include "core/common/common.h"
#include "fastqueue/BoundedLifoQueue.h"
#include "core/utils/CountDownLatch.h"

#define QUEUE_SIZE 16
#define PRODUCER_CNT 30

namespace h7_test {
struct Object{
    int value {-1};

    Object(int v):value(v){}
    ~Object(){
        printf("--> Rlease >>  %d\n", value);
    }
};
}

void test_BoundedLifoQueue(){
    using Object = h7_test::Object;
    using Queue = h7::BoundedLifoQueue<Object, QUEUE_SIZE>;
    h7::CountDownLatch cdt(2);
    bool producerDone = false;
    Queue queue;
    // 生产者
    std::thread producer([&] {
        for (int i = 1; i <= PRODUCER_CNT; ++i) {
            queue.push(new Object(i));
            //printf("> Pushed >>  %d\n", i);
        }
        producerDone = true;
        cdt.countDown(1);
    });

    // 消费者（LIFO）
    std::thread consumer([&] {
        int popCnt = 0;
        while (popCnt < PRODUCER_CNT) {
            Object* obj = nullptr;
            auto ok = queue.pop(obj);
            //even if ok
            if(ok){
                popCnt ++;
                MED_ASSERT_X(obj, "queue.pop");
                auto val = obj->value;
                printf("> Popped >>  %d\n", val);
                delete obj;
            }else{
                if(producerDone){
                    break;
                }
            }
        }
        printf("popCnt = %d\n", popCnt);
        cdt.countDown(1);
    });

    producer.join();
    consumer.join();

    cdt.await();
    // 遍历剩余元素
    queue.visit([](const Object* x) {
        std::cout << "Remaining: " << x->value << std::endl;
    });
}
