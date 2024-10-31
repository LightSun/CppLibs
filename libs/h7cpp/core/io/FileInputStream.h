#pragma once

#include <memory>
#include <fstream>
#include <iostream>
#include "common/common_base.h"
#include "io/InputStream.h"

namespace h7 {
    class FileInputStream : public AbsInputStream<FileInputStream>{
    public:
        ~FileInputStream(){
            close();
        }
        bool open(CString file, CString){
            m_fstream = std::make_unique<std::ifstream>(file, std::ios::binary);
            if(!m_fstream->is_open()){
                return false;
            }
            m_totalSize = m_fstream->seekg(0, std::ios::end).tellg();
            m_fstream->seekg(0, std::ios::beg);
            //read(&buf[0], static_cast<std::streamsize>(buf.size()));
            return true;
        }
        void close(){
            if(m_fstream){
                m_fstream->close();
                m_fstream = nullptr;
            }
            m_totalSize = -1;
        }
        //return actual read count . -1 for failed.
        size_t readCount(Count c, char* outArr){
            MED_ASSERT(m_fstream);
            return m_fstream->readsome(outArr, static_cast<std::streamsize>(c));
        }
        void seekDelta(Count offset) {
            MED_ASSERT(m_fstream);
            Count curPos = m_fstream->tellg();
            curPos += offset;
            if(curPos < 0) curPos = 0;
            m_fstream->seekg(curPos, std::ios::beg);
        }
        void reset() {
            if(m_fstream){
                m_fstream->seekg(0, std::ios::beg);
            }
        }
        Count leftSize() {
            return m_totalSize >= 0 ? m_totalSize - m_fstream->tellg() : 0;
        }
        Count totalSize() {
            return m_totalSize;
        }
    private:
        Count m_totalSize {-1};
        std::unique_ptr<std::ifstream> m_fstream;
    };
}
