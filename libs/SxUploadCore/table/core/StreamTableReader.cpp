#include "StreamTableReader.h"
#include "utils/FileReader.h"
#include "table/core/Table.h"

using namespace h7;

#define _INIT_ROW_RC 100000

StreamTableReader::StreamTableReader(CString file, CString lineSep)
{
    m_reader = new FileReader(file);
    m_lineSep = lineSep;
}
StreamTableReader::~StreamTableReader(){
    if(m_reader){
        delete m_reader;
        m_reader = nullptr;
    }
}

StreamTableReader::SpTab StreamTableReader::readTableByRowCount(uint64 maxRc, bool haveHead){
    sk_sp<Table> tab = sk_make_sp<Table>();
    //Regex regex(String(1, seq));
    Regex regex(m_lineSep);
    bool prepared = false;
    String line;
    if(haveHead){
        while (nextLine(line)) {
            if(m_annoPredicator && m_annoPredicator(line)){
                continue;
            }
            auto sp = regex.split(line);
            tab->setHeadLine(sp);
            tab->prepareColumnCount(sp->size());
            tab->prepareRowCount(maxRc);
            prepared = true;
            break;
        }
    }
    if(prepared){
        uint64 readC = 0;
        while(nextLine(line)){
            if(m_annoPredicator && m_annoPredicator(line)){
                continue;
            }
            tab->addRowAsFull(regex.split(line));
            readC++;
            if(readC >= maxRc){
                break;
            }
        }
    }else{
        std::vector<String> lines;
        nextMaxValidLines(lines, maxRc);
       // tab->prepareColumnCount(sp.size());
        tab->prepareRowCount(lines.size());
        for(int i = 0 ; i < (int)lines.size() ; i ++){
            tab->addRowAsFull(regex.split(lines[i]));
        }
    }
    return tab;
}

StreamTableReader::SpTab StreamTableReader::readTableByAboutBytes(
        uint64 max_bytes, bool haveHead){
    sk_sp<Table> tab = sk_make_sp<Table>();
    Regex regex(m_lineSep);
    bool prepared = false;
    uint64 readBytes = 0;
    //
    String line;
    if(haveHead){
        while (nextLine(line)) {
            readBytes += line.length();
            if(m_annoPredicator && m_annoPredicator(line)){
                if(readBytes >= max_bytes){
                    return nullptr;
                }
                continue;
            }
            auto sp = regex.split(line);
            tab->setHeadLine(sp);
            tab->prepareColumnCount(sp->size());
            tab->prepareRowCount(_INIT_ROW_RC);
            prepared = true;
            if(readBytes >= max_bytes){
                return tab;
            }else{
                break;
            }
        }
    }
    if(prepared){
        while(nextLine(line)){
            readBytes += line.length();
            if(m_annoPredicator && m_annoPredicator(line)){
                if(readBytes >= max_bytes){
                    break;
                }else{
                    continue;
                }
            }
            tab->addRowAsFull(regex.split(line));
            if(readBytes >= max_bytes){
                break;
            }
        }
    }else{
        std::vector<String> lines;
        nextMaxValidBytes(lines, max_bytes - readBytes);
       // tab->prepareColumnCount(sp.size());
        tab->prepareRowCount(lines.size());
        for(int i = 0 ; i < (int)lines.size() ; i ++){
            tab->addRowAsFull(regex.split(lines[i]));
        }
    }
    return tab;
}

bool StreamTableReader::nextLine(String& ret){
    if(m_rlValid){
        m_rlValid = false;
        ret = m_recentLine;
        return true;
    }
    return m_reader->readLine(ret);
}
size_t StreamTableReader::nextMaxValidLines(ListStr& out, size_t max){
    out.reserve(max);
    if(m_rlValid){
        m_rlValid = false;
        out.push_back(m_recentLine);
        max --;
    }
    String line;
    size_t readC = 0;
    while (m_reader->readLine(line)){
        if(m_annoPredicator && m_annoPredicator(line)){
            out.push_back(line);
            readC ++;
            if(readC >= max){
                break;
            }
        }
    }
    return readC;
}
void StreamTableReader::nextMaxValidBytes(ListStr& out, size_t max_bytes){
    out.reserve(_INIT_ROW_RC);
    uint64 readBytes = 0;
    if(m_rlValid){
        m_rlValid = false;
        out.push_back(m_recentLine);
        readBytes += m_recentLine.length();
    }
    String line;
    while (m_reader->readLine(line)){
        readBytes += line.length();
        if(m_annoPredicator && m_annoPredicator(line)){
            out.push_back(line);
        }
        if(readBytes >= max_bytes){
            break;
        }
    }
}
bool StreamTableReader::hasNext(){
    if(!m_rlValid){
        if(m_reader->readLine(m_recentLine)){
            m_rlValid = true;
        }
    }
    return m_rlValid;
}
void StreamTableReader::setDefaultAnnoPredicator(){
    //start with '##'
    m_annoPredicator = [](String& s){
        return s.data()[0] == '#' && s.data()[1] == '#';
    };
}
