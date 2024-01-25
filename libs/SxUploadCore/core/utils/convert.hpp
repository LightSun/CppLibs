#pragma once

#include <limits.h>
#include <string>
#include <vector>
#include <cmath>
#include "common/common.h"

namespace h7 {
template <typename T>
inline std::string toStringImpl(const T& val, const std::string&) {
  return std::to_string(val);
}

template <>
inline std::string toStringImpl(const int& val, const std::string& defVal) {
    if(val == INT_MIN){
        return defVal;
    }
    return std::to_string(val);
}
template <>
inline std::string toStringImpl(const std::string& val, const std::string& defVal) {
    if(val == "NA"){
        return defVal;
    }
    return val;
}

//int
template <typename T>
inline int getIntImpl(const T&) {
  return INT_MIN; //int_MIN
}
template <>
inline int getIntImpl(const std::string& val) {
  int ret = atoi(val.c_str());
  //if failed: return INT_MAX or INT_MIN
  if(ret == INT_MAX){
     ret = INT_MIN;
  }
  return ret;
}
template <>
inline int getIntImpl(const float& val) {
  return (int)val;
}
template <>
inline int getIntImpl(const int& val) {
  return val;
}

//
template <typename T>
inline int getIntImpl(const T&, int defVal) {
  return defVal; //int_MIN
}
template <>
inline int getIntImpl(const std::string& val, int defVal) {
  int ret = atoi(val.c_str());
  //if failed: return INT_MAX or INT_MIN
  if(ret == INT_MAX || ret == INT_MIN){
      return defVal;
  }
  return ret;
}
template <>
inline int getIntImpl(const float& val, int defVal) {
  return (int)val;
}
template <>
inline int getIntImpl(const int& val, int defVal) {
  return val;
}

//float
template <typename T>
inline float getFloatImpl(const T&) {
  return NAN; //int_MIN
}
template <>
inline float getFloatImpl(const std::string& val) {
    try {
        return std::stof(val.c_str());
    } catch (std::invalid_argument& e) {
        return NAN;
    }
}
template <>
inline float getFloatImpl(const float& val) {
  return val;
}
template <>
inline float getFloatImpl(const int& val) {
  return val;
}
//hash
#ifndef _intF
typedef union {
  int i;
  float f;
}_intF;
#endif
template <typename T>
inline int hashImpl(const T&){
    return 0;
}
template <>
inline int hashImpl(const int& val){
    return val;
}
template <>
inline int hashImpl(const float& val){
    _intF u1;
    u1.f = val;
    return u1.i;
}
template <>
inline int hashImpl(const std::string& val){
//    if(val == "NA"){
//        return 0;
//    }
    const char* strs = val.c_str();
    int hash = 0;
    if(val.length() > 0){
        for(size_t i = 0 ; i < val.length(); ++i){
            hash = 31 * hash + strs[i];
        }
    }
    return hash;
}

// parse
template <typename T>
inline T parseString(const std::string& str){
    std::string _str = "parseString error: " + str;
    MED_ASSERT_X(false, _str);
}
template <>
inline int parseString(const std::string& str){
    return getIntImpl(str);
}
template <>
inline float parseString(const std::string& str){
    return getFloatImpl(str);
}
template <>
inline std::string parseString(const std::string& str){
    return str;
}

#define toIntImpl getIntImpl
#define toFloatImpl getFloatImpl
#define toHashImpl hashImpl
}
