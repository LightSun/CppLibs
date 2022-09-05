#ifndef BINARY_SEARCH_H
#define BINARY_SEARCH_H

#ifdef __cplusplus
extern "C" {
#endif

#define DEF_BINARY_SEARCH(type, name) int binarySearch_##name(type* a, int start, int len, type key);
DEF_BINARY_SEARCH(int, int32)
DEF_BINARY_SEARCH(unsigned int, uint32)
DEF_BINARY_SEARCH(char, int8)
DEF_BINARY_SEARCH(unsigned char, uint8)

DEF_BINARY_SEARCH(short, int16)
DEF_BINARY_SEARCH(unsigned short, uint16)

DEF_BINARY_SEARCH(long long, int64)
DEF_BINARY_SEARCH(unsigned long long, uint64)

DEF_BINARY_SEARCH(float, f32)
DEF_BINARY_SEARCH(double, f64)

#ifdef __cplusplus
}
#endif

#endif // BINARY_SEARCH_H
