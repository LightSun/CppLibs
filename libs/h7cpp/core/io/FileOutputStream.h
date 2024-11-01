#pragma once

#include <fstream>
#include <memory>
#include <iostream>
#include "common/common_base.h"
#include "io/OutputStream.h"

namespace h7 {

class FileOutputStream: public AbsOutputStream<FileOutputStream>{
public:
    using UPOfstream = std::unique_ptr<std::ofstream>;

    ~FileOutputStream(){
        close();
    }

    bool open(CString file, CString){
        m_outStream = std::make_unique<std::ofstream>(file, std::ostream::out);
        return m_outStream->is_open();
    }
    //start: include, end: exclude. return 0 for failed.
    size_t write(char* buf, size_t start, size_t len){
        MED_ASSERT(m_outStream);
        m_outStream->write(buf + start, len);
        return len;
    }
    bool canWrite(size_t){
        return true;
    }
    void flush(){
        if(m_outStream){
            m_outStream->flush();
        }
    }
    void close(){
        if(m_outStream){
            m_outStream->close();
            m_outStream = nullptr;
        }
    }

private:
     UPOfstream m_outStream;
};

}
