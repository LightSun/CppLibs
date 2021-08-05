#ifndef STEST_C
#define STEST_C

#include <stdio.h>

int stest_parse(int** a, long long val){
    printf("stest_parse: val = %lld\r\n", val);
    *(*a) = 1024;
    return 1;
}

#endif // STEST_C
