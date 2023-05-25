#pragma once

#include "common/common.h"

namespace h7 {

class StringBuffer{
public:
    StringBuffer(CString buf):m_data(buf){}

    bool readLine(String& str){
        str.clear();
        char ch;
        while (m_pos < m_data.length()) {
            ch = m_data.data()[m_pos++];
            if (ch == '\n') {
                // unix: LF
                return true;
            }
            if (ch == '\r') {
                // dos: CRLF
                // read LF
                ch = m_data.data()[m_pos++];
                if(ch != '\n'){
                    m_pos--;
                }
                return true;
            }
            str += ch;
        }
        return str.length() != 0;
    }

    void reset(){
        m_pos = 0;
    }

private:
    String m_data;
    unsigned int m_pos {0};
};
}
