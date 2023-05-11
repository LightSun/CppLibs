

extern void testTime();
extern void testMessage();
extern void testLooper();
extern void testHandlerThread();
extern void testHandler();

int main(int argc, char* argv[]){

    testTime();
    testMessage();
    testLooper();
    testHandlerThread();
    testHandler();
    return 0;
}
