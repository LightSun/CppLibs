#ifndef STRINGREADER_H
#define STRINGREADER_H

#include "common/common.h"
#include <sstream>
#include "utils/string_utils.hpp"

namespace h7 {

class StringReader
{
public:
    StringReader(CString str){
        std::stringstream ss(str);
        String _str;
        while (getline(ss, _str)){
            utils::trimLastR(_str);
            m_lines.push_back(_str);
        }
    }
    unsigned int getLineCount(){
        return m_lines.size();
    }
    bool readLine(String& outS){
        if(m_curIndex >= m_lines.size()){
            return false;
        }
        outS = m_lines[m_curIndex++];
        return true;
    }
    void readLines(std::vector<String>& out){
        for(int i = m_curIndex ; i < (int)m_lines.size(); ++i){
            out.push_back(m_lines[i]);
        }
        m_curIndex = m_lines.size();
    }
    void readLines(std::vector<String>& out,
                   std::function<bool(String&)> func_anno){
        for(int i = m_curIndex ; i < (int)m_lines.size(); ++i){
            if(!func_anno || !func_anno(m_lines[i])){
                out.push_back(m_lines[i]);
            }
        }
        m_curIndex = m_lines.size();
    }
    std::vector<String>& getLines(){
        return m_lines;
    }
    void close(){
        //ignore
    }
    void reset(){
        m_curIndex = 0;
    }

private:
    std::vector<String> m_lines;
    int m_curIndex {0};
};

}

#endif // STRINGREADER_H
