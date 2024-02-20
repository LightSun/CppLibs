#ifndef NUMBERS_HPP
#define NUMBERS_HPP

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
}

#endif // NUMBERS_HPP
