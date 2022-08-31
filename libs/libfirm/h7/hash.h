#ifndef H7_HASH_H
#define H7_HASH_H
#include "h7_common.h"

uint64 fasthash64(const void *buf, uint32 len, uint64 seed);

inline uint32 fasthash32(const void *buf, uint32 len, uint32 seed){
    // the following trick converts the 64-bit hashcode to Fermat
    // residue, which shall retain information from both the higher
    // and lower parts of hashcode.
    uint64_t h = fasthash64(buf, len, seed);
    return h - (h >> 32);
}

#endif
