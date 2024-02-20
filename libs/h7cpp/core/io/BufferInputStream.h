#ifndef BUFFERINPUTSTREAM_H
#define BUFFERINPUTSTREAM_H

#include <stdlib.h>
#include <functional>
#include "common/common.h"

namespace h7 {

class BufferInputStream
{
public:
    static constexpr int DEFAULT_BUF_LEN = 8192;
    BufferInputStream(int len){
        m_buf = (char*)malloc(len);
    }
    BufferInputStream():BufferInputStream(DEFAULT_BUF_LEN){
    }
    virtual ~BufferInputStream();

    bool open(CString file);
    void close();
    bool readLine(String& out);

    inline void readLines(std::function<void(int,CString)> func){
        String str;
        int index = 0;
        while (readLine(str)) {
            func(index++, str);
        }
    }
    inline void readLines(std::vector<String>& vec){
        String str;
        while (readLine(str)) {
            vec.push_back(str);
        }
    }
    String readString(int count);

    //--------------------------------------
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
    //--------------------------------------
private:
    char* m_buf;
    int m_validLen {-1}; //valid buffer len
    int m_startPos {0};
    String m_lastStr;

    FILE* m_file {nullptr};
};

}

#endif // BUFFERINPUTSTREAM_H
