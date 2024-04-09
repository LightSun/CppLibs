#include <iostream>
#include <fstream>
#include <memory.h>

extern void test_medRSA();

//pri enc, pub text dec.
int main(int argc, char* argv[]){
    setbuf(stdout, NULL);
    test_medRSA();
    return 0;
}

