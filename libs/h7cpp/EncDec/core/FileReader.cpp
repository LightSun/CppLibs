#include <iostream>
#include "FileReader.h"

using namespace med;

namespace utils {
static inline bool trimLastR(String& str){
    if(str.length() > 0 && str.c_str()[str.length()-1]=='\r'){
        str.erase(str.length()-1);
        return true;
    }
    return false;
}
}

String FileReader::readPemContent(){
    String ret;
    ret.reserve(1024);

    String s;
    //int len = 0;
    while (readLine(s)) {
        //len += s.length();
        String line = s + "\n";
        ret = ret.append(line);
        //printf("line_len: '%lu'\n", line.length());
    }
    //printf("total_len: %d\n", len);
    return ret;
}

bool FileReader::readLine(String& outS){
    if(getline(m_stream, outS)){
        utils::trimLastR(outS);
        return true;
    }
    return false;
}

void FileReader::readLines(std::vector<String>& out){
    String str;
    while(getline(m_stream, str)){
        utils::trimLastR(str);
        out.push_back(str);
    }
}
void FileReader::readLines(std::vector<String>& out, std::function<bool(String&)> func_anno){
    String str;
    while(getline(m_stream, str)){
        utils::trimLastR(str);
        if(!func_anno || !func_anno(str)){
            out.push_back(str);
        }
    }
}

void FileReader::readMaxLines(std::vector<String>& out, size_t max,
                  std::function<bool(String&)> func_anno){
    size_t readC = 0;
    String str;
    while(getline(m_stream, str)){
        utils::trimLastR(str);
        if(!func_anno || !func_anno(str)){
            out.push_back(str);
            readC ++;
            if(readC >= max){
                break;
            }
        }
    }
}

bool FileReader::readStr(String& outS, char delim){
    if(getline(m_stream, outS, delim)){
        return true;
    }
    return false;
}

void FileReader::readCount(std::vector<String>& out, int count, char delim, CString defval){
    String str;
    for(int i = 0 ; i < count; i ++){
        if(getline(m_stream, str, delim)){
            out.push_back(str);
        }else{
            out.push_back(defval);
        }
    }
}

