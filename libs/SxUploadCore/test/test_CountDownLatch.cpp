
#include <thread>
#include "utils/CountDownLatch.h"

using namespace h7;

void test_CountDownLatch(){
    int c = 10;
    CountDownLatch lt(c);
    for(int i = 0 ; i < c ; ++i){
        int idx = i;
        std::thread thd([idx, &lt](){
            printf("idx = %d\n", idx);
            lt.countDown();
        });
        thd.detach();
    }
    bool ret = lt.await();
    printf("test_CountDownLatch >> end. ret = %d\n", ret);
}
