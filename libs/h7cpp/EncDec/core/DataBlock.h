#pragma once

#include <string>
#include <vector>
#include <iostream>

#ifndef MED_ASSERT
#define MED_ASSERT(condition)                                                   \
    do                                                                      \
    {                                                                       \
        if (!(condition))                                                   \
        {                                                                   \
            std::cout << "Assertion failure: " << __FILE__ << "::" << __FUNCTION__  \
                                     << __LINE__ \
                                     << " >> " << #condition << std::endl;  \
            abort();                                                        \
        }                                                                   \
    } while (0)
#endif

namespace med {

using CString = const std::string&;
using String = std::string;
template<typename T>
using List = std::vector<T>;
using uint32 = unsigned int;

#define DB_VERSION 1
#define DEF_HASH_SEED 11

struct DataBlock{
    int version;
    int blockCount;
    List<uint32> blockSizes;
    List<size_t> blockHashes;
    List<String> blockDatas;
    List<int> verifyStates;

    void setBlockCount(int c){
        this->blockCount = c;
        blockSizes.resize(c);
        blockDatas.resize(c);
        blockHashes.resize(c);
        verifyStates.resize(c);
    }
    bool isVerifyOk(){
        for(auto& r: verifyStates){
            if(r == 0) return false;
        }
        return true;
    }
    String getDecodeResult();
    void setEncodeData(int i, CString data);
    void setDecodeData(int i, CString data, bool verify);
    String toString();
    String fromString(CString str, bool strict = true);
};

}
