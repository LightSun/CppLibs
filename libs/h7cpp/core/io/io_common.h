#pragma once

#include <string>
#include <vector>
#include <functional>

#ifndef DEF_API_VIRTUAL
#define DEF_API_VIRTUAL(a) virtual a = 0
#endif

#ifndef CMD_LINE
#ifdef _WIN32
#define CMD_LINE "\r\n"
#else
#define CMD_LINE "\n"
#endif
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
