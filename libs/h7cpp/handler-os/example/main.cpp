
#include <iostream>
#include "handler-os/QTApplication.h"

extern int test_qt_handler(int argc, char* argv[]);

extern void testTime();
extern void testMessage();
extern void testLooper();
extern void testHandlerThread();
extern void testHandler();
extern void test_TaskFlow();

static void testNormal();

int main(int argc, char* argv[]){
    setbuf(stdout, NULL);
#ifdef BUILD_WITH_QT
    return test_qt_handler(argc, argv);
#else
    //testNormal()
    test_TaskFlow();
    return 0;
#endif
}

void testNormal(){
    testTime();
    testMessage();
    testLooper();
    testHandlerThread();
    testHandler();
}
