#ifndef MAPPEDINPUTSTREAM_H
#define MAPPEDINPUTSTREAM_H

#include <functional>
#include "common/common.h"
#include "common/SkRefCnt.h"
#include "io/MemoryMapped.h"

namespace h7 {

class MappedInputStream: public SkRefCnt
{
public:
    ~MappedInputStream(){
        m_mm.close();
    }
    inline bool open(CString file, size_t mappedBytes = 0){
        return m_mm.open(file, mappedBytes);
    }
    inline void close(){
        m_mm.close();
    }
    bool readLine(String& out);

    String readString(int count);

    void readLines(std::function<void(int,CString)> func);
    void readLines(std::vector<String>& vec);

    inline bool readDouble(double& out){
        String str = readString(sizeof (double));
        if(!str.empty()){
            out = atof(str.c_str());
            return true;
        }
        return false;
    }
    inline bool readFloat(float& out){
        String str = readString(sizeof (float));
        if(!str.empty()){
            out = (float)atof(str.c_str());
            return true;
        }
        return false;
    }
    inline bool readShort(short& out){
        String str = readString(sizeof (short));
        if(!str.empty()){
            out = (short)atoi(str.c_str());
            return true;
        }
        return false;
    }
    inline bool readBool(bool& out){
        String str = readString(sizeof (bool));
        if(!str.empty()){
            out = atoi(str.c_str()) != 0;
            return true;
        }
        return false;
    }
    inline bool readInt32(int& out){
        String str = readString(sizeof (int));
        if(!str.empty()){
            out = atoi(str.c_str());
            return true;
        }
        return false;
    }
    inline bool readInt64(long long int & out){
        String str = readString(sizeof (long long int));
        if(!str.empty()){
            out = atoll(str.c_str());
            return true;
        }
        return false;
    }

    inline void reset(){
        m_curPos = 0;
    }
    inline int adjustCount(int count)const{
        if(m_curPos + count > m_mm.size()){
            uint64_t diff = m_curPos + count - m_mm.size();
            count -= diff;
        }
        return count;
    }
    inline uint64_t size(){
        return m_mm.size();
    }
    inline void skip(int count){
        m_curPos += count;
    }
    inline bool readChar(char& out){
        if(!isEnd()){
            out = m_mm.getData()[m_curPos++];
            return true;
        }
        return false;
    }
    inline bool isEnd() const{
        return m_curPos >= m_mm.size();
    }
private:
    MemoryMapped m_mm;
    uint64_t m_curPos {0};
};

}

#endif // MAPPEDINPUTSTREAM_H
