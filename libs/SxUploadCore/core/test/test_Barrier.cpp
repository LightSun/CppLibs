#include <thread>
#include "utils/Barrier.h"

using namespace h7;

void test_Barrier(){
    int c = 10;
    Barrier bar(c);

    for(int i = 0 ; i < c ; i ++){
        int idx = i;
        std::thread thd([idx, &bar](){
            printf("idx = %d\n", idx);
            bar.await();
            printf("test_Barrier >> done. idx = %d\n", idx);
        });
        thd.detach();
    }
    //wait the all thread done.
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
}
