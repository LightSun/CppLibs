#ifndef NUMBERS_H
#define NUMBERS_H

#include <math.h>

union __Uni_Float1{
    float f;
    int val;
};

union __Uni_Double1{
    double f;
    long long val;
};


static inline int isFloatEquals(float a, float b){
    union __Uni_Float1 uf1; uf1.f = a;
    union __Uni_Float1 uf2; uf2.f = b;
    return abs(uf1.val - uf2.val) < 10;
}

static inline int isDoubleEquals(double a, double b){
    union __Uni_Double1 uf1; uf1.f = a;
    union __Uni_Double1 uf2; uf2.f = b;
    return llabs(uf1.val - uf2.val) < 10;
}

#endif // NUMBERS_H
