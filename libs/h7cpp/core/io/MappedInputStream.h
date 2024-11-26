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
    using UPMemoryMapped = std::unique_ptr<MemoryMapped>;

    MappedInputStream(){
        m_memMapped = std::make_unique<MemoryMapped>(MemoryMapped::kOP_READ);
    }

    static bool open0(MemoryMapped& memMap,CString file, CString hint){
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
        auto pgSize = MemoryMapped::getpagesize();
        bool hasLeft = (mapSize % pgSize) != 0;
        mapSize = (mapSize / pgSize + (hasLeft ? 1 : 0)) * pgSize;
        return memMap.open(file, mapSize, cacheHint);
    }
    bool open(CString file, CString hint){
        m_nextPos = 0;
        return open0(*m_memMapped, file, hint);
    }
    void close(){
        m_memMapped->close();
    }
    //return actual read count . -1 for failed.
    size_t readCount(Count c, char* outArr){
        const auto left = leftSize();
        if(c > left) c = left;
        std::memcpy(outArr, m_memMapped->getData() + m_nextPos, c);
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
        return m_memMapped->mappedSize();
    }

private:
    UPMemoryMapped m_memMapped;
    Count m_nextPos {0};
};

}
