#include <stdio.h>
#include "binary_search.h"

#define DEF_BINARY_SEARCH_IMPL(type, name)\
    static int findFirstNeqPos_##name(type* a, int start, int len, type key) {\
        int high = start + len;\
        int low = start - 1;\
        int guess = -1;\
        while(high - low > 1) {\
            guess = (high + low) / 2;\
            if (a[guess] != key) {\
                low = guess;\
            } else {\
                high = guess;\
            }\
        }\
        return low;\
    }    \
    int binarySearch_##name(type* a, int start, int len, type key){\
        int high = start + len;\
        int low = start - 1;\
        while(high - low > 1) {\
            int guess = (high + low) / 2;\
            if (a[guess] < key) {\
                low = guess;\
            } else {\
                high = guess;\
            }\
        }\
        if (high == start + len) {\
            return ~(start + len);\
        } else if (a[high] == key) {\
            if(high > start){\
                return findFirstNeqPos_##name(a, start, high - start, key) + 1;\
            }\
            return high;\
        } else {\
            return ~high;\
        }\
    }

DEF_BINARY_SEARCH_IMPL(char, int8)
DEF_BINARY_SEARCH_IMPL(unsigned char, uint8)

DEF_BINARY_SEARCH_IMPL(short, int16)
DEF_BINARY_SEARCH_IMPL(unsigned short, uint16)

DEF_BINARY_SEARCH_IMPL(int, int32)
DEF_BINARY_SEARCH_IMPL(unsigned int, uint32)

DEF_BINARY_SEARCH_IMPL(long long, int64)
DEF_BINARY_SEARCH_IMPL(unsigned long long, uint64)

/*
static int findFirstNeqPos(int* a, int start, int len, int key);

int binarySearch(int* a, int start, int len, int key) {
//    for(int i = 0 ; i < len ; i ++){
//        printf("i = %d: %d\n", start + i, a[start + i]);
//    }

    int high = start + len;
    int low = start - 1;

    while(high - low > 1) {
        int guess = (high + low) / 2;
        if (a[guess] < key) {
            low = guess;
        } else {
            high = guess;
        }
    }

    if (high == start + len) {
        return ~(start + len);
    } else if (a[high] == key) {
        //may have the same element. we only want the first element index.
        if(high > start){
            return findFirstNeqPos(a, start, high - start, key) + 1;
        }
        return high;
    } else {
        return ~high;
    }
}

static int findFirstNeqPos(int* a, int start, int len, int key) {
    int high = start + len;
    int low = start - 1;
    int guess = -1;
    while(high - low > 1) {
        guess = (high + low) / 2;
        if (a[guess] != key) {
            low = guess;
        } else {
            high = guess;
        }
    }
    return low;
}
*/
