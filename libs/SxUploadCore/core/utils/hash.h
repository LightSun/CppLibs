#ifndef H7_HASH_H
#define H7_HASH_H
#include "common/c_common.h"

CPP_START

uint64 fasthash64(const void *buf, uint32 len, uint64 seed);

uint32 fasthash32(const void *buf, uint32 len, uint32 seed);

CPP_END

#endif
