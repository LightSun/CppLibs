#include "pmap.h"
#include <stdio.h>

union Val0{
    int val;
    char ptr[4];
};

void test_map(){
    union Val0 val;
    val.val = 10;
    printf("val.ptr = %p\n", val.ptr);

    union Val0 val2;
    val2.val = 10;
    printf("val2.ptr = %p\n", val2.ptr);
}
