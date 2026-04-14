#include <thread>
#include <iostream>
#include "core/common/common.h"
#include "fastqueue/BoundedLIFOQueue.h"
#include "fastqueue/BoundedFIFOQueue.h"
#include "core/utils/CountDownLatch.h"

#define QUEUE_SIZE 16
#define PRODUCER_CNT 30

namespace h7_test {
struct Object{
    int value {-1};

    Object(int v):value(v){}
    ~Object(){
       // printf("--> Rlease >>  %d\n", value);
    }
};
}

void test_BoundedLifoQueue(){
    //TODO changed need modify demo.
    using Object = h7_test::Object;
    using Queue = h7::BoundedLIFOQueue<Object>;
    h7::CountDownLatch cdt(2);
    bool producerDone = false;
    Queue queue(QUEUE_SIZE);
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

void test_BoundedFifoQueue(){
    using Object = h7_test::Object;
    using Queue = h7::BoundedFIFOQueue<Object>;
    h7::CountDownLatch cdt(3);

    std::atomic_int dropCnt = 0;
    Queue queue(QUEUE_SIZE);
    queue.setDropCallback(nullptr, [&dropCnt](void*, Object* p){
        printf("drop >> %d\n", p->value);
        delete p;
        dropCnt.fetch_add(1, std::memory_order_release);
        return true;
    });

    std::thread p1([&] {
        for (int i = 1; i <= PRODUCER_CNT; ++i) {
            queue.push(new Object(i));
            printf("> Pushed >>  %d\n", i);
        }
        cdt.countDown(1);
    });
    p1.detach();

    std::thread p2([&] {
        for (int i = 1; i <= PRODUCER_CNT; ++i) {
            queue.push(new Object(i + PRODUCER_CNT));
            printf("> Pushed >> %d\n", i + PRODUCER_CNT);
        }
        cdt.countDown(1);
    });
    p2.detach();

    std::thread consumer([&] {
        int total = PRODUCER_CNT * 2;
        int popCnt = 0;
        while (popCnt < total - dropCnt.load(std::memory_order_acquire)) {
            Object* obj = nullptr;
            auto ok = queue.pop(obj);
            //even if ok
            if(ok){
                popCnt ++;
                MED_ASSERT_X(obj, "queue.pop");
                auto val = obj->value;
                printf("> Popped >> %d\n", val);
                delete obj;
            }
        }
        printf("---> popCnt = %d\n", popCnt);
        cdt.countDown(1);
    });
    consumer.join();
    cdt.await();
    printf("----> dropCnt = %d\n", dropCnt.load(std::memory_order_relaxed));
    // 遍历剩余元素
    queue.visit([](const Object* x) {
        std::cout << "Remaining: " << x->value << std::endl;
    });
}
