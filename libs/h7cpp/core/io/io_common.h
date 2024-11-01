#pragma once

#include <string>
#include <vector>
#include <functional>

#ifndef DEF_IS_API_VIRTUAL
#define DEF_IS_API_VIRTUAL(a) virtual a = 0
#endif

namespace h7 {

template<typename T>
union UniVal{
    char arr[sizeof(T)];
    T val;
};

typedef UniVal<short> UniShort;
typedef UniVal<int> UniInt;
typedef UniVal<unsigned int> UniUInt;
typedef UniVal<long long> UniLong;
typedef UniVal<unsigned long long> UniULong;
typedef UniVal<float> UniFloat;
typedef UniVal<double> UniDouble;

}
