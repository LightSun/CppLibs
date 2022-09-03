#include "h7/array_map_n.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace std;

static struct core_allocator Allocator_Default;

static void* alloc_impl(uint32 size){
    return malloc(size);
}

static void* realloc_impl(void* ptr, uint32 /*oldSize*/, uint32 newSize){
    void* nptr = realloc(ptr, newSize);
    ASSERT(nptr != NULL);
    return nptr;
}
static void free_impl(void* ptr){
    free(ptr);
}
void test_array_map_n(){
    Allocator_Default.Alloc = alloc_impl;
    Allocator_Default.Realloc = realloc_impl;
    Allocator_Default.Free = free_impl;

    array_map_n_p ptr = array_map_n_new(&Allocator_Default, 5000, 1000);
    array_map_n_prepare_entryCount(ptr, 100);
    string keys[100];
    string values[100];
    for(int i = 0 ; i < 100 ; i ++){
        keys[i] = "I am heaven7_" + std::to_string(i);
        values[i] = "value_" + std::to_string(i);
        //itoa(i, (char*)keys[i], 10);
        array_map_n_put(ptr, (void*)keys[i].data(), keys[i].length() + 1,
                        (void*)values[i].data(), values[i].length() + 1, NULL);
    }
    ASSERT(array_map_n_size(ptr) == 100)
    core_mem mem_info;
    for(int i = 0 ; i < 100 ; i ++){
        //itoa(i, (char*)keys[i], 10);
        array_map_n_rawget(ptr, (void*)keys[i].data(), keys[i].length() + 1, &mem_info);
        ASSERT(mem_info.data != NULL);
        string val = (char*)mem_info.data;
        ASSERT(val == values[i]);
    }
    array_map_n_remove(ptr, (void*)keys[0].data(), keys[0].length() + 1, &mem_info);
    ASSERT(mem_info.data != NULL);
    string val = (char*)mem_info.data;
    ASSERT(val == values[0]);
    core_mem_free(&mem_info);
    ASSERT(array_map_n_size(ptr) == 99)
    //
    array_map_n_rawget(ptr, (void*)keys[0].data(), keys[0].length() + 1, &mem_info);
    ASSERT(mem_info.data == NULL);
    //
    array_map_n_delete(ptr);
}
