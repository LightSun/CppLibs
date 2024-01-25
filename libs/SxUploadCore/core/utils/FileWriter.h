#ifndef H_FILEWRITER_H
#define H_FILEWRITER_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include "common/SkRefCnt.h"
#include "common/common.h"

namespace h7 {

class FileWriter: public SkRefCnt
{
public:
    FileWriter(CString file){
        m_stream.open(file, std::ios::out);
        std::string msg = "FileWriter >> open file failed: " + file;
        MED_ASSERT_X(m_stream.is_open(), msg);
    }
    ~FileWriter(){
        if(m_stream.is_open()) m_stream.close();
    }

    void writeLine(CString str);

    template<typename T>
    void writeListAsLine(const std::vector<T>& vec, char sep = '\t'){
        int size = vec.size();
        for(int i = 0 ; i < size ; i ++){
            m_stream << vec[i];
            if(i != size - 1){
                m_stream << sep;
            }
        }
        m_stream << std::endl;
    }
    inline bool isOpen(){
        return m_stream.is_open();
    }
    inline void close(){
        if(m_stream.is_open()) m_stream.close();
    }
public:
    std::fstream m_stream;
};

}

#endif // FILEWRITER_H
