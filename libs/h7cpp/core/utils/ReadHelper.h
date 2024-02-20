#ifndef READHELPER_H
#define READHELPER_H

#include <vector>
#include <string>
#include <memory.h>
#include "common/c_common.h"

namespace h7 {

static inline uint32 readLong(char* addr, sint64& out){
    memcpy(&out, addr, sizeof(sint64));
    return sizeof(sint64);
}
static inline uint32 readInt(char* addr, int& out){
    memcpy(&out, addr, sizeof(int));
   // out = *(int*)(addr);
    return sizeof(int);
}
static inline uint32 readShort(char* addr, short& out){
    memcpy(&out, addr, sizeof(short));
    return sizeof(int);
}
static inline uint32 readUInt32(char* addr, uint32& out){
    memcpy(&out, addr, sizeof(uint32));
    //out = *(uint32*)(addr);
    return sizeof(uint32);
}

static inline uint32 readString(char* addr, std::string& str){
    int c;
    readInt(addr, c);
    if(c == 0){
        str = "";
        return sizeof(int);
    }
    str = std::string(addr + sizeof(int), c);
    return sizeof(int) + c;
}

static inline uint32 readBytes(char* addr, std::string& str){
    int c;
    readInt(addr, c);
    if(c == 0){
        str = "";
        return sizeof(int);
    }
    str = std::string(addr + sizeof(int), c);
    return sizeof(int) + c;
}
//-------------------------
static inline uint32 readListInt(char* addr, std::vector<int>& vec){
    int c;
    readInt(addr, c);
    uint32 offset = sizeof(int);
    if(c > 0){
        int val;
        for(int i = 0 ; i < c ; ++i){
            offset += readInt(addr + offset, val);
            vec.push_back(val);
        }
    }
    return offset;
}
static inline uint32 readListString(char* addr, std::vector<std::string>& vec){
    int c;
    readInt(addr, c);
    uint32 offset = sizeof(int);
    if(c > 0){
        std::string val;
        for(int i = 0 ; i < c ; ++i){
            offset += readString(addr + offset, val);
            vec.push_back(val);
        }
    }
    return offset;
}

}

#endif // READHELPER_H
