#ifndef H7_COMMON_H
#define H7_COMMON_H

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#ifndef sint8
typedef signed char sint8;
#endif
#ifndef uint8
typedef unsigned char uint8;
#endif

#ifndef sint16
typedef signed short sint16;
#endif
#ifndef uint16
typedef unsigned short uint16;
#endif

#ifndef sint32
typedef signed int sint32;
#endif
#ifndef uint32
typedef unsigned int uint32;
#endif

#ifndef sint64
typedef signed long long sint64;
#endif
#ifndef uint64
//typedef unsigned long long uint64;
typedef uint64_t uint64;
#endif

#ifdef __cplusplus
#define CPP_START extern "C" {
#define CPP_END }
#else
#define CPP_START
#define CPP_END
#endif

#ifdef __ANDROID_NDK__
    #include <android/log.h>
    #ifndef LOG_TAG
    #define LOG_TAG "h7"
    #endif
    #define LOGD(fmt, ...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##__VA_ARGS__)
    #define LOGV(fmt, ...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, fmt, ##__VA_ARGS__)
    #define LOGW(fmt, ...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, fmt, ##__VA_ARGS__)
#else
    #define LOGD(fmt, ...) printf(fmt, ##__VA_ARGS__)
    #define LOGV(fmt, ...) printf(fmt, ##__VA_ARGS__)
    #define LOGW(fmt, ...) printf(fmt, ##__VA_ARGS__)
#endif

#ifndef kroundup32
#define kroundup32(x) (--(x), (x)|=(x)>>1, (x)|=(x)>>2, (x)|=(x)>>4, (x)|=(x)>>8, (x)|=(x)>>16, ++(x))
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

#define HMIN(a, b) ((a) > (b) ? (b) : (a))
#define HMAX(a, b) ((a) > (b) ? (a) : (b))
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

#define core_mem_free(mem)\
{  struct core_mem* _mem = mem;\
    if(_mem->data){\
        _mem->ca->Free(_mem->data);\
    }\
}

//inline void core_mem_free(struct core_mem* mem){
//    if(mem->data){
//        mem->ca->Free(mem->data);
//    }
//}

#endif

