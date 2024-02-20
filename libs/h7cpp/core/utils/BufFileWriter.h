#ifndef H_BUFFILEWRITER_H
#define H_BUFFILEWRITER_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include "common/SkRefCnt.h"
#include "common/common.h"
#include "utils/convert.hpp"

namespace h7 {

class BufFileWriter: public SkRefCnt
{
public:
    BufFileWriter(CString file){
        m_buf.open(file, std::ios::out);
        std::string msg = "BufFileWriter >> open file failed: " + file;
        MED_ASSERT_X(m_buf.is_open(), msg);
    }
    ~BufFileWriter(){
        if(m_buf.is_open()) m_buf.close();
    }

    void writeLine(CString str);

    inline void writeLine(const std::vector<String>& vec, char sep = '\t'){
        int size = vec.size();
        for(int i = 0 ; i < size ; i ++){
            m_buf.sputn(vec[i].data(), vec[i].length());
            if(i != size - 1){
                m_buf.sputc(sep);
            }
        }
#ifdef WIN32
    m_buf.sputn("\r\n", 2);
#else
    m_buf.sputc('\n');
#endif
    }
    template<typename T>
    void writeListAsLine(const std::vector<T>& vec, char sep = '\t'){
        int size = vec.size();
        String str;
        for(int i = 0 ; i < size ; i ++){
            str = toStringImpl<T>(vec[i], "");
            m_buf.sputn(str.c_str(), str.length());
            if(i != size - 1){
                m_buf.sputc(sep);
            }
        }
#ifdef WIN32
    m_buf.sputn("\r\n", 2);
#else
    m_buf.sputc('\n');
#endif
    }
    inline bool isOpen(){
        return m_buf.is_open();
    }
    inline void close(){
        if(m_buf.is_open()) m_buf.close();
    }
public:
    std::filebuf m_buf;
};

}

#endif // FILEWRITER_H
