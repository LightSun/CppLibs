
#include <thread>
#include "utils/SaveQueue.h"
#include "utils/SafeQueue.h"
#include "utils/CountDownLatch.h"
#include "utils/times.h"
#include "utils/Random.h"

using namespace h7;

static void test_SaveQueue0();
static void test_SafeQueue();

void test_SaveQueue(){
    //test_SaveQueue0();
    test_SafeQueue();
}

void test_SaveQueue0(){
    SaveQueue<int> q(2);
//    CountDownLatch cdt(2);
//    std::thread thd([&q, &cdt](){
//        for(int i = 0 ; i < 30 ; ++i){
//            while(!q.enqueue(i + 1)){
//                printf("enqueue:: failed. cur_size = %lu\n", q.size());
//            }
//            SLEEP(h7::Random::nextInt(0, 15));
//        }
//        cdt.countDown();
//    });
//    thd.detach();
//    std::thread thd2([&q, &cdt](){
//        for(int i = 0 ; i < 100 ; ++i){
//            int val;
//            while(!q.dequeue(val)){
//                //printf("dequeue:: failed. cur_size = %lu\n", q.size());
//                if(q.size() == 0){
//                    break;
//                }
//            }
//            SLEEP(h7::Random::nextInt(0, 15));
//        }
//        cdt.countDown();
//    });
//    thd2.detach();
 //   cdt.await();
    q.enqueue(1);
    q.enqueue(2);
    int val;
    while(q.dequeue(val)){
        printf("test_SaveQueue >> final deq = %d.\n", val);
    }
    q.enqueue(1);
    q.enqueue(2);
    printf("test_SaveQueue >> all done.\n");
}

void test_SafeQueue(){
    SafeQueue<int> q(2);
    CountDownLatch cdt(2);
    std::thread thd([&q, &cdt](){
        for(int i = 0 ; i < 1000 ; ++i){
            while(!q.enqueue(i + 1)){
                printf("enqueue:: failed. cur_size = %lu\n", q.size());
                SLEEP(h7::Random::nextInt(1, 5));
            }
            SLEEP(h7::Random::nextInt(0, 15));
        }
        cdt.countDown();
    });

    thd.detach();
    std::thread thd2([&q, &cdt](){
        for(int i = 0 ; i < 1000 ; ++i){
            int val;
            bool ok = true;
            while(!q.dequeue(val)){
                //printf("dequeue:: failed. cur_size = %lu\n", q.size());
                if(q.size() == 0){
                    ok = false;
                    break;
                }
                SLEEP(h7::Random::nextInt(1, 5));
            }
            if(ok){
                printf("dequeue >> ok, val = %d\n", val);
            }
            SLEEP(h7::Random::nextInt(0, 15));
        }
        cdt.countDown();
    });
    thd2.detach();
    cdt.await();
    printf("SafeQueue >> size = %lu. buf_size = %lu\n",
           q.size(), q.bufferSize());
    int val;
    while(q.dequeue(val)){
        printf("SafeQueue >> left.val = %d.\n", val);
    }
    printf("test_SafeQueue >> all done.\n");
}
