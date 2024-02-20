#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <memory.h>
#include "InputStream.h"
#include "utils/string_utils.hpp"

using namespace h7;

bool InputStream::open(CString file){
    //https://www.zhihu.com/question/36675524
    //from: https://www.cnblogs.com/redips-l/p/8258306.html
    std::ifstream fin(file, std::ios::binary);
    if(!fin.is_open()){
        return false;
    }
    m_data = sk_make_sp<ListCh>(fin.seekg(0, std::ios::end).tellg(), true);
    std::vector<char>& buf = m_data->list;
    fin.seekg(0, std::ios::beg).read(&buf[0], static_cast<std::streamsize>(buf.size()));
    fin.close();
    return true;
}

void InputStream::readLines(std::function<void(int,CString)> func){
    const char* data = m_data->list.data();
    const int len = m_data->size();
    int start_pos = m_curPos;
    int cur_pos = start_pos;
    int lineIndex = 0;
    for( ; cur_pos < len; ){
        if(data[cur_pos++] == '\n'){
            String str(data + start_pos, cur_pos - 1 - start_pos);
            utils::trimLastR(str);
            (func)(lineIndex++, str);
            start_pos = cur_pos;
        }
    }
    m_curPos = len;
}
void InputStream::readLines(std::vector<String>& vec){
    const char* data = m_data->list.data();
    const int len = m_data->size();
    int start_pos = m_curPos;
    int cur_pos = start_pos;
    for( ; cur_pos < len; ){
        if(data[cur_pos++] == '\n'){
            String str(data + start_pos, cur_pos - 1 - start_pos);
            utils::trimLastR(str);
            vec.push_back(str);
            start_pos = cur_pos;
        }
    }
    m_curPos = len;
}

bool InputStream::readLine(String& out){
    const char* data = m_data->list.data();
    const int len = m_data->size();
    const int start_pos = m_curPos;
    int cur_pos = start_pos;
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

String InputStream::readString(int count){
    //check if over limit
    count = adjustCount(count);
    if(count <=  0){
        return "";
    }
    char arr[count + 1];
    memcpy(arr, m_data->list.data() + m_curPos, count);
    arr[count] = '\0';
    m_curPos += count;
    return String(arr);
}
int InputStream::readCount(int count, char* outArr, int arrLen){
    //check if over out arr
    if(arrLen < count){
        count = arrLen;
    }
    //check if over limit
    count = adjustCount(count);
    if(count <= 0){
        return -1;
    }

    memcpy(outArr, m_data->list.data() + m_curPos, count);
    m_curPos += count;
    return count;
}
