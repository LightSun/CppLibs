#ifndef VECTORS_H
#define VECTORS_H

#include <memory.h>
#include <vector>
#include <string>

namespace h7 {

static inline std::vector<char> string2vec2(const void* data, int len){
    std::vector<char> vec;
    vec.resize(len);
    memcpy(vec.data(), data, len);
    return vec;
}
static inline std::vector<char> string2vec(const std::string& str){
    std::vector<char> vec;
    vec.resize(str.length());
    memcpy(vec.data(), str.data(), str.length());
    return vec;
}
}

#endif // VECTORS_H
