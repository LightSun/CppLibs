

extern void testTime();
extern void testMessage();
extern void testLooper();
extern void testHandlerThread();
extern void testHandler();

extern int test_qt_handler(int argc, char* argv[]);

static void testNormal();

int main(int argc, char* argv[]){

//  testNormal();
    //return 0;
    return test_qt_handler(argc, argv);
}

void testNormal(){
    testTime();
    testMessage();
    testLooper();
    testHandlerThread();
    testHandler();
}
