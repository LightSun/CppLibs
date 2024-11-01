#pragma once

#include "io/io_common.h"

namespace h7 {

class OutputStream{
public:
    template<typename T>
    union UniVal{
        char arr[sizeof(T)];
        T val;
    };

    using String = std::string;
    using CString = const std::string&;

    virtual ~OutputStream(){}

    DEF_IS_API_VIRTUAL(bool open(CString file, CString param));

    //start: include, return 0 for failed.
    DEF_IS_API_VIRTUAL(size_t write(char* buf, size_t start, size_t len));
    DEF_IS_API_VIRTUAL(bool canWrite(size_t size));

    DEF_IS_API_VIRTUAL(void flush());
    DEF_IS_API_VIRTUAL(void close());

    //-------------------

    inline bool write(char* buf, size_t buf_len){
        return write(buf, 0, buf_len) > 0;
    }
    inline bool writeBool(bool val){
        return writeChar(val ? 1 : 0);
    }
    inline bool writeChar(char val){
        return write((char*)&val, 0, sizeof(char)) > 0;
    }
    inline bool writeShort(short val){
        //UniShort kv;
        //kv.val = val;
        return write((char*)&val, 0, sizeof(short)) > 0;
    }
    inline bool writeInt(int val){
        return write((char*)&val, 0, sizeof(int)) > 0;
    }
    inline bool writeUInt(unsigned int val){
        return write((char*)&val, 0, sizeof(unsigned int)) > 0;
    }
    inline bool writeLong(long long val){
        return write((char*)&val, 0, sizeof(long long)) > 0;
    }
    inline bool writeULong(unsigned long long val){
        return write((char*)&val, 0, sizeof(unsigned long long)) > 0;
    }
    inline bool writeFloat(float val){
        return write((char*)&val, 0, sizeof(float)) > 0;
    }
    inline bool writeDouble(double val){
        return write((char*)&val, 0, sizeof(double)) > 0;
    }
    inline bool writeRawString(const std::string& str){
        return write((char*)str.data(), 0, str.length()) > 0;
    }
    inline bool writeString(const std::string& str){
        if(canWrite(str.size() + sizeof(unsigned int))){
            writeUInt(str.length());
            writeRawString(str);
            return true;
        }
        return false;
    }
    inline bool writeLine(const std::string& str){
        return writeRawString(str + CMD_LINE);
    }
    inline bool writeLines(const std::vector<String>& vec){
        const int size = vec.size();
        String str;
        str.reserve(40960);
        for(int i = 0 ; i < size ; ++i){
            str.append(vec[i] + CMD_LINE);
        }
        if(canWrite(str.length())){
            return writeRawString(str);
        }
        return false;
    }
    template<typename T>
    inline bool writeList(const std::vector<T>& vec, std::function<String(const T&)> func){
        const int size = vec.size();
        String str;
        str.reserve(40960);
        for(int i = 0 ; i < size ; ++i){
            str.append(func(vec[i]) + CMD_LINE);
        }
        if(canWrite(str.length())){
            return writeRawString(str);
        }
        return false;
    }
};

template<typename T>
class AbsOutputStream: public OutputStream{
public:

    bool open(CString s, CString param) override{
        return (static_cast<T*>(this))->open(s, param);
    }
    size_t write(char* buf, size_t start, size_t len)override{
        return (static_cast<T*>(this))->write(buf, start, len);
    }
    bool canWrite(size_t size)override{
        return (static_cast<T*>(this))->canWrite(size);
    }
    void flush()override{
       (static_cast<T*>(this))->flush();
    }
    void close()override{
       (static_cast<T*>(this))->close();
    }
};

}
