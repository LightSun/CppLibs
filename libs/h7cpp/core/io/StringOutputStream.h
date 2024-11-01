#pragma once

#include <memory>
#include <iostream>
#include "common/common_base.h"
#include "io/OutputStream.h"

namespace h7 {
class StringOutputStream : public AbsOutputStream<StringOutputStream>{
public:
    StringOutputStream(size_t bufferSize = 40960){
        m_buffer.reserve(bufferSize);
    }
    bool open(CString alreadyBuf, CString){
        m_buffer.append(alreadyBuf);
        return true;
    }
    //start: include, end: exclude. return 0 for failed.
    size_t write(char* buf, size_t start, size_t len){
        m_buffer.append(buf + start, len);
        return len;
    }
    bool canWrite(size_t){
        return true;
    }
    void flush(){
    }
    void close(){
    }
    String& getBuffer(){
        return m_buffer;
    }
private:
    String m_buffer;
};
}
