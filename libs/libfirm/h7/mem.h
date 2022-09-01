#ifndef H7_MEM_H
#define H7_MEM_H

#include <stdlib.h>
#include <memory.h>
#include "h7_common.h"

inline void arrays_insert(void* arr, int already_ele_count, unsigned int ele_unit_size,
                   void* data, unsigned int target_pos){
    if(already_ele_count == 0){
         ASSERT(target_pos == 0);
         memcpy(arr, data, ele_unit_size);
         return;
    }
    char* dst = (char*)arr + target_pos * ele_unit_size;
    unsigned int moveSize = (already_ele_count - target_pos) * ele_unit_size;
    void* copy = malloc( moveSize);
    memcpy(copy, dst, moveSize);
    memcpy(dst, data, ele_unit_size);
    memcpy(dst + ele_unit_size, copy, moveSize);
    free(copy);
}

#endif
