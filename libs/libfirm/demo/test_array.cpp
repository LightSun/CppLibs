#include "array.h"
#include "obstack.h"
#include "common.h"

void test_array(){

    struct obstack  obst;
    obstack_init(&obst);
    int len = 10;
    char* arr = NEW_ARR_D(char, &obst, len);
    MED_ASSERT(ARR_LEN(arr) == len);
    for(int i = 0 ; i < len ; i ++){
        arr[i] = i;
    }
    obstack_free(&obst, arr);
}
