#include <iostream>
#include "FileReader.h"
#include "utils/string_utils.hpp"

namespace h7 {

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

String FileReader::readRangeLines(std::vector<String>& out, unsigned int start,
                                unsigned int count,
                  std::function<bool(String&)> func_anno, bool firstLineAsHeader){
    if(count == 0){
        count = INT_MAX;
    }
    String header;
    String str;
    unsigned int read_count = 0;
    unsigned int end = start + count;//11 + 10
    if(firstLineAsHeader){
        end += 1;
    }
    for(;;){
        if(getline(m_stream, str)){
            if(!func_anno || !func_anno(str)){
                read_count ++;
                if(firstLineAsHeader && read_count == 1){
                    header = str;
                    continue;
                }
                if(read_count >= start){
                    if(read_count < end){
                        out.push_back(str);
                    }else{
                        break;
                    }
                }
            }
        }else{
            break;
        }
    }
    return header;
}

void FileReader::readProperties(std::map<String, String>& prop){
    String str;
    int index;
    while (getline(m_stream, str)) {
        utils::trimLastR(str);
        h7::utils::trim(str);
        if(str.empty()){
            continue;
        }
        //anno
        if(str.c_str()[0] == '#' || h7::utils::startsWith(str, "//")){
            continue;
        }
        //printf("prop_line: %s\n", str.c_str());
        index = str.find("=");
        MED_ASSERT(index >= 0);
        String _s = str.substr(0, index);
        h7::utils::trim(_s);
        if(index == (int)str.length() - 1){
            prop[_s] = "";
        }else{// abc=1235 : 3
            prop[_s] = str.substr(index + 1, str.length() - index - 1);
        }
    }
}
void FileReader::readProperties(std::unordered_map<String, String>& prop){
    String str;
    int index;
    while (getline(m_stream, str)) {
        utils::trimLastR(str);
        h7::utils::trim(str);
        if(str.empty()){
            continue;
        }
        //anno
        if(str.c_str()[0] == '#' || h7::utils::startsWith(str, "//")){
            continue;
        }
        //printf("prop_line: %s\n", str.c_str());
        index = str.find("=");
        MED_ASSERT(index >= 0);
        String _s = str.substr(0, index);
        h7::utils::trim(_s);
        if(index == (int)str.length() - 1){
            prop[_s] = "";
        }else{// abc=1235 : 3
            prop[_s] = str.substr(index + 1, str.length() - index - 1);
        }
    }
}
}
