#ifndef H7_COMMON_H
#define H7_COMMON_H

#include <stdlib.h>
#include <stdio.h>

#ifndef uint8
typedef unsigned char uint8;
typedef uint8 uint8_t;
#endif

#ifndef uint16
typedef unsigned short uint16;
typedef uint16 uint16_t;
#endif

#ifndef uint32
typedef unsigned int uint32;
typedef uint32 uint32_t;
#endif

#ifndef uint64
typedef unsigned long long uint64;
typedef uint64 uint64_t;
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

typedef void* (*Alloc1)(uint32 size);
typedef void* (*Realloc1)(void* ptr, uint32 oldSize, uint32 newSize);
typedef void (*Free1)(void* ptr);
struct core_allocator{
    Alloc1 Alloc;
    Realloc1 Realloc;
    Free1 Free;
};

#endif
