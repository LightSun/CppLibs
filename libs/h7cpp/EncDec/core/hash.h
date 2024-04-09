#ifndef H7_HASH_H
#define H7_HASH_H

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
#define CPP_START extern "C" {
#define CPP_END }
#else
#define CPP_START
#define CPP_END
#endif

#ifndef uint32
typedef unsigned int uint32;
#endif

#ifndef uint64
#ifdef uint64_t
typedef uint64_t uint64;
#else
typedef size_t uint64;
#endif
#endif

CPP_START

uint64 fasthash64(const void *buf, uint32 len, uint64 seed);

uint32 fasthash32(const void *buf, uint32 len, uint32 seed);

CPP_END

#endif
