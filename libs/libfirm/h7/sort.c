#include "sort.h"
#include <stdlib.h>
#include <memory.h>

#define _SWAP_DATA(i, i2)\
{\
    char* src_ptr1 = rdata + (i) * ele_len;\
    char* src_ptr2 = rdata + (i2) * ele_len;\
    memcpy(tmp_data, src_ptr1, ele_len);\
    memcpy(src_ptr1, src_ptr2, ele_len);\
    memcpy(src_ptr2, tmp_data, ele_len);\
}

#define _SWAP_INDEX(i, i2)\
{\
    uint32 oi = outIndex[(i)];\
    outIndex[(i)] = outIndex[(i2)];\
    outIndex[(i2)] = oi;\
}

#define _BUBBLE_SORT_CORE(func)\
if(outIndex){\
    for (uint32 i = 0; i < ele_c; ++i){\
        outIndex[i] = i;\
    }\
}\
int L = 0, R = ele_c - 1;\
char* rdata = (char*)data;\
void* tmp_data = malloc(ele_len);\
while(L < R) {\
   for(int i = L; i < R; i++) {\
       void* rowi = (rdata + i * ele_len + every_offset);\
       void* rowi1 = (rdata + (i + 1) * ele_len + every_offset);\
       if(func(ud, i, i + 1, rowi, rowi1) > 0){\
            _SWAP_DATA(i, i + 1);\
            if(outIndex) _SWAP_INDEX(i, i + 1);\
       }\
   }\
   R--;\
   for(int i = R; i > L; i--) {\
       void* rowi = (rdata + i * ele_len + every_offset);\
       void* rowi1 = (rdata + (i - 1) * ele_len + every_offset);\
       if(func(ud, i, i -1, rowi, rowi1) < 0){\
           _SWAP_DATA(i, i - 1);\
           if(outIndex) _SWAP_INDEX(i, i - 1);\
       }\
   }\
   L++;\
}\
free(tmp_data);

static inline int _cmp_aesc_u32(void* ud, int i1, int i2,const void* v1, const void* v2){
    (void)(ud);
    uint32 val1 = *(uint32*)(v1);
    uint32 val2 = *(uint32*)(v2);
    if(val1 == val2){
        return i1 > i2 ? 1 : -1;
    }
    return (val1 < val2) ? -1 : 1;
}
static inline int _cmp_aesc_s32(void* ud, int i1, int i2,const void* v1, const void* v2){
    (void)(ud);
    sint32 val1 = *(sint32*)(v1);
    sint32 val2 = *(sint32*)(v2);
    if(val1 == val2){
        return i1 > i2 ? 1 : -1;
    }
    return (val1 < val2) ? -1 : 1;
}

void sortReturnIndex_cmp(const void* data, uint32 ele_c, uint32 ele_len,
                             uint32 every_offset, FUNC_CMP_SORT func, void* ud,
                             uint32* outIndex){
    _BUBBLE_SORT_CORE(func);
}

void sortReturnIndex_u32(const void* data, uint32 ele_c, uint32 ele_len, uint32 every_offset,
                     uint32* outIndex){
    void *ud = NULL;
   _BUBBLE_SORT_CORE(_cmp_aesc_u32)
}
void sortReturnIndex_s32(const void* data, uint32 ele_c, uint32 ele_len, uint32 every_offset,
                         uint32* outIndex){
    void *ud = NULL;
    _BUBBLE_SORT_CORE(_cmp_aesc_s32)
}
