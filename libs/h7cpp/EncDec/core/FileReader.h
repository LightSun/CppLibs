#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <functional>
#include <unordered_map>
#include "DataBlock.h"

namespace med{

class FileReader
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

    String readPemContent();

    String readLine(){
        String s;
        readLine(s);
        return s;
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
