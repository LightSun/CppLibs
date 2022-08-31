
#include <memory.h>
#include <stdio.h>

extern void test_array();
extern void test_deq();
extern void test_map();

int main(int argc, char* argv[]){
    test_array();
    test_deq();
    test_map();

    int a = 10;
    int* p = &a;
    char arr[sizeof (int)];
    memcpy(arr, p, sizeof (int));

    int b = *(int*)arr;
    printf("a = %d, b = %d\n", a, b); //a = 10, b = 10
    return 0;
}
