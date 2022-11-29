#ifndef H7_ALLOC_H
#define H7_ALLOC_H

#include "h7/common/c_common.h"

typedef void* (*Alloc1)(uint32 size);
typedef void* (*Realloc1)(void* ptr, uint32 old, uint32 newSize);
typedef void (*Free1)(void* ptr);

typedef struct core_allocator core_allocator;
struct core_allocator{
    Alloc1 Alloc;
    Realloc1 Realloc;
    Free1 Free;
};

//extern struct core_allocator* Allocator_Default;

struct core_mem{
    struct core_allocator* ca;
    void* data;
    uint32 size;
};

#define core_mem_free(mem)\
{  struct core_mem* _mem = mem;\
    if(_mem->data){\
        _mem->ca->Free(_mem->data);\
    }\
}

void h7_set_core_allocator(struct core_allocator* alloc);
struct core_allocator* h7_get_core_allocator();

#define ALLOC(size) h7_get_core_allocator()->Alloc(size)
#define REALLOC(ptr,nsize) h7_get_core_allocator()->Realloc(ptr, 0, nsize)
#define FREE(ptr) h7_get_core_allocator()->Free(ptr)


#endif // H7_ALLOC_H
