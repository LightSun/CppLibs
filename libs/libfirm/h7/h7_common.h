#ifndef H7_COMMON_H
#define H7_COMMON_H

#include <stdlib.h>
#include <stdio.h>

#ifndef uint8
typedef unsigned char uint8;
#endif

#ifndef uint16
typedef unsigned short uint16;
#endif

#ifndef uint32
typedef unsigned int uint32;
#endif

#ifndef uint64
typedef unsigned long long uint64;
#endif

#ifdef __cplusplus
#define CPP_START extern "C" {
#define CPP_END }
#else
#define CPP_START
#define CPP_END
#endif

#define ASSERT(con) \
    if(!(con)){\
        fprintf(stderr, "assert failed. file = %s, func = %s, line = %d, condition = %s\n", \
            __FILE__, __FUNCTION__,__LINE__, #con);\
        abort();\
    }

#define ASSERT_X(con, msg) \
    if(!(con)){\
        fprintf(stderr, "assert failed. file = %s, func = %s, line = %d, condition = %s\n", \
            __FILE__, __FUNCTION__,__LINE__, #con);\
        fprintf(stderr, "%s\n", msg);\
        abort();\
    }

#define HMIN(a, b) (a > b ? b : a)
#define HMAX(a, b) (a > b ? a : b)
#define GROWUP_HALF(c) (c % 4 == 0 ? (c * 3 / 2) : (c * 3 / 2 + 1))

typedef void* (*Alloc1)(uint32 size);
typedef void* (*Realloc1)(void* ptr, uint32 oldSize, uint32 newSize);
typedef void (*Free1)(void* ptr);
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

inline void core_mem_free(struct core_mem* mem){
    if(mem->data){
        mem->ca->Free(mem->data);
    }
}

#endif
