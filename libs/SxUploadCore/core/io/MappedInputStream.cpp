#include <stdlib.h>
#include "io/MappedInputStream.h"
#include "io/MemoryMapped.h"
#include "utils/string_utils.hpp"

using namespace h7;

bool MappedInputStream::readLine(String& out){
    const char* data = m_mm.getData();
    const uint64_t len = m_mm.size();
    const uint64_t start_pos = m_curPos;
    uint64_t cur_pos = start_pos;
    for( ; cur_pos < len; ){
        if(data[cur_pos++] == '\n'){
            out = String(data + start_pos, cur_pos - 1 - start_pos);
            utils::trimLastR(out);
            m_curPos = cur_pos;
            return true;
        }
    }
    return false;
}

String MappedInputStream::readString(int count){
    //check if over limit
    count = adjustCount(count);
    if(count <=  0){
        return "";
    }
    char arr[count + 1];
    memcpy(arr, m_mm.getData() + m_curPos, count);
    arr[count] = '\0';
    m_curPos += count;
    return String(arr);
}

void MappedInputStream::readLines(sk_sp<h7::function<void(int, CString)>> func){
    String str;
    int index = 0;
    for(;readLine(str);){
        (*func)(index++, str);
    }
}
void MappedInputStream::readLines(std::function<void(int,CString)> func){
    String str;
    int index = 0;
    for(;readLine(str);){
        (func)(index++, str);
    }
}
void MappedInputStream::readLines(std::vector<String>& vec){
    String str;
    for(;readLine(str);){
        vec.push_back(str);
    }
}
