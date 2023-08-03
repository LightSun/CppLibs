#include <stdio.h>
#include "binary_search.h"

#define MACRO_findFirstNeqPos(a, start, len, key, out) \
{\
    int _high = start + len;\
    int _low = start - 1;\
    int _guess = -1;\
    while(_high - _low > 1) {\
        _guess = (_high + _low) / 2;\
        if (a[_guess] != key) {\
            _low = _guess;\
        } else {\
            _high = _guess;\
        }\
    }\
    out = _low;\
}


namespace h7 {

static int findFirstNeqPos(int* a, int start, int len, int key);
static int findFirstNeqPos_u(unsigned int* a, int start,
                             int len, unsigned int key);
static int findFirstNeqPos_ll(long long* a, int start, int len, long long key);
static int findFirstNeqPos_offset_u(const void* a, int ev_size,int hash_offset,
                             int start,int len, unsigned int key);
//-----------------------------------------

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

int binarySearch_u(unsigned int* a, int start, int len, unsigned int key){
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
        if(high > start){
            return findFirstNeqPos_u(a, start, high - start, key) + 1;
        }
        return high;
    } else {
        return ~high;
    }
}
int binarySearch_ll(long long * a, int start, int len, long long key){
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
        if(high > start){
            return findFirstNeqPos_ll(a, start, high - start, key) + 1;
        }
        return high;
    } else {
        return ~high;
    }
}

//--------------------------------------------------------

#define _BSOU(guess) (*(unsigned int*)(ptr + ev_size * guess + hash_offset))
#define _BSOU64(guess) (*(unsigned long long*)(ptr + ev_size * guess + hash_offset))

int binarySearchOffset_u(const void* a, int ev_size,int hash_offset,
                         int start, int len, unsigned int key){
    unsigned char * ptr = (unsigned char*)a;

    int high = start + len;
    int low = start - 1;

    while(high - low > 1) {
        int guess = (high + low) / 2;
        if (_BSOU(guess) < key) {
           // if (a[guess] < key) {
            low = guess;
        } else {
            high = guess;
        }
    }

    if (high == start + len) {
        return ~(start + len);
    } else if (_BSOU(high) == key) {
        if(high > start){
            return findFirstNeqPos_offset_u(a, ev_size, hash_offset,
                                            start, high - start, key) + 1;
        }
        return high;
    } else {
        return ~high;
    }
}

//binarySearchOffset_u(m_items.data(), sizeof(Item), offset,
//                              0, m_items.size(), _hash);
int binarySearchOffset_u64(const void* a, int ev_size,int hash_offset,
                         int start, int len, unsigned long long key){
    unsigned char * ptr = (unsigned char*)a;

    int high = start + len;
    int low = start - 1;

    while(high - low > 1) {
        int guess = (high + low) / 2;
        if (_BSOU64(guess) < key) {
           // if (a[guess] < key) {
            low = guess;
        } else {
            high = guess;
        }
    }

    if (high == start + len) {
        return ~(start + len);
    } else if (_BSOU64(high) == key) {
        if(high > start){
            return findFirstNeqPos_offset_u(a, ev_size, hash_offset,
                                            start, high - start, key) + 1;
        }
        return high;
    } else {
        return ~high;
    }
}

//-----------------------

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

static int findFirstNeqPos_u(unsigned int* a, int start, int len, unsigned int key) {
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

static int findFirstNeqPos_ll(long long* a, int start, int len, long long key) {
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

int findFirstNeqPos_offset_u(const void* a, int ev_size,int hash_offset,
                             int start,int len, unsigned int key){
    unsigned char * ptr = (unsigned char*)a;
    int high = start + len;
    int low = start - 1;
    int guess = -1;
    while(high - low > 1) {
        guess = (high + low) / 2;
        if (_BSOU(guess) != key) {
            low = guess;
        } else {
            high = guess;
        }
    }
    return low;
}

int findFirstNeqPos_offset_u64(const void* a, int ev_size,int hash_offset,
                             int start,int len, unsigned long long key){
    unsigned char * ptr = (unsigned char*)a;
    int high = start + len;
    int low = start - 1;
    int guess = -1;
    while(high - low > 1) {
        guess = (high + low) / 2;
        if (_BSOU64(guess) != key) {
            low = guess;
        } else {
            high = guess;
        }
    }
    return low;
}

}
