#include "h7/sort.h"
#include <string>
#include <stdlib.h>
#include <stddef.h>
#include "h7/h7_common.h"

using namespace std;

struct Stu{
    float val;
    uint32 age;
    char* a;
};

//(TYPE *)0是指向TYPE类型的指针，其指针值是0，其作用是将从地址0开始的一段内存映射为TYPE类型的结构对象。
#define OFFSETOF(struct_name,member_name) (size_t)&( ((struct_name*)0)->member_name)

void test_sort(){
    printf("--- test_sort >> start ...\n");
    Stu* data = (Stu*)malloc(sizeof(struct Stu) * 3);
    data[0] = {5.5, 2};
    data[1] = {6.5, 3};
    data[2] = {4.5, 1};
    //
    fprintf(stderr, "offset age: %u\n",OFFSETOF(struct Stu, age));
    fprintf(stderr, "offset a: %u\n",OFFSETOF(struct Stu, a));
    //
    sortReturnIndex_u32(data, 3, sizeof(struct Stu), OFFSETOF(struct Stu, age), NULL);
    for(int i = 0 ; i < 3 ; i ++){
        fprintf(stderr, "i = %d: age = %d\n", i, data[i].age);
    }
    ASSERT(data[0].age == 1);
    ASSERT(data[0].val == 4.5);
    ASSERT(data[1].age == 2);
    ASSERT(data[1].val == 5.5);
    ASSERT(data[2].age == 3);
    ASSERT(data[2].val == 6.5);
    free(data);
    printf("--- test_sort >> end ...\n");
}
