#pragma once

#include <memory>
#include <cstring>
#include <iostream>
#include "common/common_base.h"
#include "io/OutputStream.h"
#include "io/MappedInputStream.h"

namespace h7 {
class MappedOutputStream : public AbsOutputStream<MappedOutputStream>{
public:
    bool open(CString file, CString param){
        return MappedInputStream::open0(m_memMapped, file, param);
    }
    //start: include, end: exclude. return 0 for failed.
    size_t write(char* buf, size_t start, size_t len){
        MED_ASSERT(m_memMapped.isValid());
        if(len > leftSize()) return 0;
        auto dataPtr = (char*)m_memMapped.getData() + m_nextPos;
        std::memcpy((void*)dataPtr, buf + start, len);
        m_nextPos += len;
        return len;
    }
    bool canWrite(size_t){
        return true;
    }
    void flush(){
    }
    void close(){
    }

private:
    size_t leftSize(){
        return m_memMapped.size() - m_nextPos;
    }

private:
    MemoryMapped m_memMapped;
    size_t m_nextPos {0};
};
}
