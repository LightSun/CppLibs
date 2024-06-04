
#include <thread>
#include "utils/SaveQueue.h"
#include "utils/CountDownLatch.h"
#include "utils/times.h"
#include "utils/Random.h"

using namespace h7;

static void test();

void test_SaveQueue(){
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
