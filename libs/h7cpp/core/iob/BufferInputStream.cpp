#include <string.h>
#include <memory.h>
#include "io/BufferInputStream.h"
#include "utils/string_utils.hpp"

using namespace h7;

BufferInputStream::~BufferInputStream(){
    if(m_buf){
        free(m_buf);
    }
    if(m_file){
        fclose(m_file);
        m_file = nullptr;
    }
}

void BufferInputStream::close(){
    if(m_file){
        fclose(m_file);
        m_file = nullptr;
    }
}

bool BufferInputStream::open(CString file){
    MED_ASSERT(!m_file);
    m_file = fopen(file.c_str(), "rb");
    if(m_file == nullptr){
        PRINTLN("open file failed: %s\n", file.c_str());
        return false;
    }
    return true;
}
bool BufferInputStream::readLine(String& out){
    const char* data = m_buf;
    int cur_pos;
    for(;;) {
        if(m_validLen == -1){
            m_validLen = (int32_t)fread(m_buf, 1, strlen(data), m_file);
        }
        if(m_validLen <= 0){
            return false;
        }
        cur_pos = m_startPos;
        for( ; cur_pos < m_validLen; ){
            if(data[cur_pos++] == '\n'){
                out = m_lastStr + String(data + m_startPos, cur_pos - 1 - m_startPos);
                utils::trimLastR(out);
                m_lastStr = "";
                m_startPos = cur_pos;
                return true;
            }
        }
        //not find line end char. save and wait latter use.
        m_lastStr += String(data + m_startPos, strlen(data) - m_startPos);
        m_startPos = 0;
        m_validLen = -1;
    }
}

String BufferInputStream::readString(int count){
    const char* data = m_buf;
    String out;
    if(m_validLen == -1){
        m_validLen = (int32_t)fread(m_buf, 1, strlen(data), m_file);
    }
    if(m_validLen <= 0){
        return "";
    }
    //over range
    if(m_startPos + count >= m_validLen - 1){
        out = String(data + m_startPos, m_validLen - 1 - m_startPos);
        m_validLen = (int32_t)fread(m_buf, 1, strlen(data), m_file);
        m_startPos = 0;
        if(m_validLen <= 0){
            return out;
        }
        out += readString(count - out.length());
    }else{
        out = String(data + m_startPos, data + m_startPos + count);
        m_startPos += count;
        if(m_startPos == m_validLen - 1){
            m_validLen = (int32_t)fread(m_buf, 1, strlen(data), m_file);
            m_startPos = 0;
        }
    }
    return out;
}

