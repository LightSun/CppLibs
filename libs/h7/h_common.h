#ifndef H_COMMON_H
#define H_COMMON_H

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
typedef unsigned long long uint64;
#endif

#define H_CONCAT_2(x,y) H_CONCAT_2_EXPAND(x,y)
#define H_CONCAT_2_EXPAND(x,y) x ## y

#define H_CONCAT_3(x,y,z) H_CONCAT_3_EXPAND(x,y,z)
#define H_CONCAT_3_EXPAND(x,y,z) x ## y ## z

#define H_CONCAT_4_EXPAND(x,y,z,w) x ## y ## z ## w
#define H_CONCAT_4(x,y,z,w) H_CONCAT_4_EXPAND(x,y,z,w)


typedef union h_common_union{
    sint8 _sint8;
    uint8 _uint8;
    sint16 _sint16;
    uint16 _uint16;
    sint32 _sint32;
    uint32 _uint32;
    sint64 _sint64;
    uint64 _uint64;
    float _float;
    double _double;
    int _int;
    void* ptr;
}h_common_union;

#define

#endif // H_COMMON_H
