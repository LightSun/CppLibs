
#include "mem.h"
#include <stdlib.h>
#include <memory.h>

void arrays_insert(void* arr, int already_ele_count, unsigned int ele_unit_size,
                   const void* data, unsigned int target_pos){
    if(already_ele_count == 0){
         ASSERT(target_pos == 0);
         memcpy(arr, data, ele_unit_size);
         return;
    }
    char* dst = (char*)arr + target_pos * ele_unit_size;
    unsigned int moveSize = (already_ele_count - target_pos) * ele_unit_size;
    if(moveSize > 0){
        void* copy = malloc( moveSize);
        memcpy(copy, dst, moveSize);
        memcpy(dst, data, ele_unit_size);
        memcpy(dst + ele_unit_size, copy, moveSize);
        free(copy);
    }else{
        memcpy(dst, data, ele_unit_size);
    }
}

void arrays_remove(void* arr, int already_ele_count, unsigned int ele_unit_size,
                   unsigned int target_pos){
    if(already_ele_count > 0){
        char* dst = (char*)arr + target_pos * ele_unit_size;
        unsigned int moveSize = (already_ele_count - target_pos - 1) * ele_unit_size;
        if(moveSize > 0){
            void* copy = malloc( moveSize);
            memcpy(copy, dst + ele_unit_size, moveSize);
            memcpy(dst, copy, moveSize);
            free(copy);
        }
    }
}

void mem_remove(void* mem, unsigned int total_size, unsigned int offset, unsigned int size){
    ASSERT(total_size > 0 && size > 0 && size <= total_size)
    char* dst = (char*)mem + offset;
    unsigned int moveSize = total_size - offset - size;
    if(moveSize > 0){
        void* copy = malloc( moveSize);
        memcpy(copy, dst + offset, moveSize);
        memcpy(dst, copy, moveSize);
        free(copy);
    }
}

void mem_insert(void* mem, unsigned int total_size, unsigned int offset, const void* data, unsigned int size){
    ASSERT(total_size > 0 && size > 0 && size <= total_size);
    char* dst = (char*)mem + offset;
    unsigned int moveSize = total_size - offset;
    if(moveSize > 0){
        void* copy = malloc(moveSize);
        memcpy(copy, dst, moveSize);
        memcpy(dst, data, size);
        memcpy(dst + size, copy, moveSize);
        free(copy);
    }else{
        memcpy(dst, data, size);
    }

}
