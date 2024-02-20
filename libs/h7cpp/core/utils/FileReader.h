#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <unordered_map>
#include "common/common.h"
#include "common/SkRefCnt.h"

namespace h7 {

class FileReader: public SkRefCnt
{
public:
    FileReader(CString file){
        //PRINTLN("FileReader >> start read: %s\n", file.data());
        m_stream.open(file, std::ios::in);
        std::string msg = "FileReader >> open file failed:'";
        msg += file + "'";
        MED_ASSERT_X(m_stream.is_open(), msg);
    }
    ~FileReader(){
         if(m_stream.is_open()) m_stream.close();
    }

    bool readLine(String& outS);
    void readLines(std::vector<String>& out);
    void readLines(std::vector<String>& out, std::function<bool(String&)> func_anno);
    void readMaxLines(std::vector<String>& out, size_t max,
                      std::function<bool(String&)> func_anno);

    bool readStr(String& out, char delim = '\t');

    //read util count.
    void readCount(std::vector<String>& out, int count, char delim = '\t',
                   CString defval = "");

    //read range of lines. if return null. mean don't have header. or else return header line.
    String readRangeLines(std::vector<String>& out, unsigned int start,
                               unsigned int count,
                               std::function<bool(String&)> func_anno,
                               bool firstLineAsHeader);

    void readProperties(std::map<String, String>& prop);
    void readProperties(std::unordered_map<String, String>& prop);

    inline bool isEnd(){
        return m_stream.peek() == EOF;
    }
    inline bool isOpen(){
        return m_stream.is_open();
    }
    inline void close(){
        if(m_stream.is_open()) m_stream.close();
    }
private:
    std::ifstream m_stream;
};

}
