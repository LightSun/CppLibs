#pragma once

#include <memory>
#include <cstring>
#include <iostream>
#include "common/common_base.h"
#include "io/InputStream.h"
#include "io/MemoryMapped.h"

namespace h7 {

class MappedInputStream : public AbsInputStream<MappedInputStream>{
public:
    bool open(CString file, CString hint){
        Count mapSize;
        MemoryMapped::CacheHint cacheHint;
        int pos = hint.find(":");
        if(pos < 0){
            mapSize = std::stoll(hint);
            cacheHint = MemoryMapped::Normal;
        }else{
            mapSize = std::stoll(hint.substr(0, pos));
            auto actHint = hint.substr(pos + 1);
            if(actHint == "Normal"){
                cacheHint = MemoryMapped::Normal;
            }else if(actHint == "SequentialScan"){
                cacheHint = MemoryMapped::SequentialScan;
            }else if(actHint == "RandomAccess"){
                cacheHint = MemoryMapped::RandomAccess;
            }else{
                //default
                cacheHint = MemoryMapped::Normal;
            }
        }
        MED_ASSERT(mapSize > 0);
        m_nextPos = 0;
        return m_memMapped.open(file, mapSize, cacheHint);
    }
    void close(){
        m_memMapped.close();
    }
    //return actual read count . -1 for failed.
    size_t readCount(Count c, char* outArr){
        const auto left = leftSize();
        if(c > left) c = left;
        std::memcpy(outArr, m_memMapped.getData() + m_nextPos, c);
        m_nextPos += c;
        return c;
    }
    void seekDelta(Count offset){
        m_nextPos += offset;
    }
    void reset(){
        m_nextPos = 0;
    }
    Count leftSize(){
        return totalSize() - m_nextPos;
    }
    Count totalSize(){
        return m_memMapped.mappedSize();
    }

private:
    MemoryMapped m_memMapped;
    Count m_nextPos {0};
};

}
