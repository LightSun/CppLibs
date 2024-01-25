#ifndef INPUTSTREAM_H
#define INPUTSTREAM_H

#include <vector>
#include "table/Column.h"
#include "common/Function.h"

namespace h7 {

class InputStream: public SkRefCnt
{
public:
    InputStream(){}

    inline void loadFile(CString file){
        if(!open(file)){
            String msg = "open file failed: " + file;
            fprintf(stderr, "%s.\n", msg.c_str());
        }
    }
    bool open(CString file);

    inline void close(){
    }

    bool readLine(String& out);
    void readLines(sk_sp<h7::function<void(int, CString)>> func);
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
    String readString(int count);

    int readCount(int count, char* outArr, int arrLen);
    inline void skip(int count){
        m_curPos += count;
    }
    inline bool readChar(char& out){
        if(!isEnd()){
            out = m_data->list[m_curPos++];
            return true;
        }
        return false;
    }
    inline bool isEnd() const{
        return m_curPos >= m_data->size();
    }
    inline void reset(){
        m_curPos = 0;
    }
    inline int adjustCount(int count)const{
        if(m_curPos + count > m_data->size()){
            int diff = m_curPos + count - m_data->size();
            count -= diff;
        }
        return count;
    }
    inline size_t size(){
        return m_data->list.size();
    }
private:
    sk_sp<ListCh> m_data;
    int m_curPos {0};
};

}

#endif // INPUTSTREAM_H
