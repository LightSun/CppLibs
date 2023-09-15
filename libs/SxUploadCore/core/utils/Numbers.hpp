#ifndef NUMBERS_HPP
#define NUMBERS_HPP

#include <memory.h>
#include <limits>
#include <inttypes.h>

namespace h7 {

inline void int_reverse_to(int val, const void* _dst){
    char* dst = (char*)_dst;
    dst[0] =  val & 0xff;
    dst[1] = (val & 0xff00) >> 8;
    dst[2] = (val & 0xff0000) >> 16;
    dst[3] = (val & 0xff000000) >> 24;
}

inline void long_reverse_to(unsigned long long val, const void* _dst){
    char* dst = (char*)_dst;
    dst[0] = (val & 0xff);
    dst[1] = (val & 0xff00) >> 8;
    dst[2] = (val & 0xff0000) >> 16;
    dst[3] = (val & 0xff000000) >> 24;
    dst[4] = (val & 0xff00000000) >> 32;
    dst[5] = (val & 0xff0000000000) >> 40;
    dst[6] = (val & 0xff000000000000) >> 48;
    dst[7] = (val & 0xff00000000000000) >> 56;
}

inline float intBitsToFloat(int i)
{
    union
    {
      int i;
      float f;
    } u;
    u.i = i;
    return u.f;
}


inline int floatToRawIntBits(float f){
    union
    {
        int i;
        float f;
    } u;
    u.f = f;
    return u.i;
}

inline long long doubleToLongBits(double value) {
    if(value != value){
        return 0x7ff8000000000000L;
    }
    long long longValue = 0;
    memcpy(&longValue, &value, sizeof(double));

    return longValue;
}

inline long long doubleToRawLongBits(double value) {
    long long longValue = 0;
    memcpy(&longValue, &value, sizeof(double));
    return longValue;
}

inline double longBitsToDouble(long long bits) {
    double doubleValue = 0;
    memcpy(&doubleValue, &bits, sizeof(int64_t));
    return doubleValue;
}

inline bool isInfinite(double value) {
    return (value == std::numeric_limits<double>::infinity()
            || value == -std::numeric_limits<double>::infinity());
}

inline bool isNaN(double value) {
    return (value != value);
}

inline int64_t unsignedShift(int64_t num, int64_t shift) {
    return (shift & 0x3f) == 0 ? num : (((uint64_t)num >> 1) & 0x7fffffffffffffffLL) >> ((shift & 0x3f) - 1);
}

inline int32_t unsignedShift(int32_t num, int32_t shift) {
    return (shift & 0x1f) == 0 ? num : (((uint32_t)num >> 1) & 0x7fffffff) >> ((shift & 0x1f) - 1);
}


}

#endif // NUMBERS_HPP
