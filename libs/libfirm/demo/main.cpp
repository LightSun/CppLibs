
#include <memory.h>
#include <stdio.h>

extern void test_array();
extern void test_deq();
extern void test_map();
extern void test_array_map_n();
extern void test_sort();
extern void test_gen_execute_ins_win();

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
    //
    test_array_map_n();
    test_sort();
    //test_gen_execute_ins_win();
    return 0;
}
