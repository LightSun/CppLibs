#pragma once

#include "io/io_common.h"

namespace h7 {

struct StringConvertor{
    using String = std::string;
    using CString = const std::string&;

    static double atof(const char* str){
        return std::stof(str);
    }
    static int atoi(const char* str){
        return std::stoi(str);
    }
    static long long atoll(const char* str){
        return std::stoll(str);
    }
    static long long atoull(const char* str){
        return std::stoull(str);
    }
};

class InputStream{
public:
    using Count = long long;
    using String = std::string;
    using CString = const std::string&;

    virtual ~InputStream(){}

    DEF_IS_API_VIRTUAL(bool open(CString file, CString param));
    DEF_IS_API_VIRTUAL(void close());
    //return actual read count . -1 for failed.
    DEF_IS_API_VIRTUAL(size_t readCount(Count count, char* buf));
    DEF_IS_API_VIRTUAL(void seekDelta(Count offset));
    //reset to first position
    DEF_IS_API_VIRTUAL(void reset());
    //return 0 for failed
    DEF_IS_API_VIRTUAL(Count leftSize());
    //return > 0 for valid
    DEF_IS_API_VIRTUAL(Count totalSize());
    //------------------------
    inline bool isValid(){
        return totalSize() > 0;
    }
    inline void seekHead(){
        reset();
    }
    inline bool isEnd(){
        return leftSize() == 0;
    }
    inline void skip(Count offset){
        seekDelta(offset);
    }
    inline bool readDouble(double& out){
        using TYPE = double;
        UniDouble uni;
        if(readCount(sizeof(TYPE), uni.arr) == sizeof(TYPE)){
            out = uni.val;
            return true;
        }
        return false;
    }
    inline bool readFloat(float& out){
        using TYPE = float;
        UniFloat uni;
        if(readCount(sizeof(TYPE), uni.arr) == sizeof(TYPE)){
            out = uni.val;
            return true;
        }
        return false;
    }
    inline bool readBool(bool& out){
        char val;
        if(readChar(val)){
            out = val != 0;
            return true;
        }
        return false;
    }
    inline bool readShort(short& out){
        UniShort uni;
        if(readCount(sizeof(short), uni.arr) == sizeof(short)){
            out = uni.val;
            return true;
        }
        return false;
    }
    inline bool readInt(int& out){
        UniInt uni;
        if(readCount(sizeof(int), uni.arr) == sizeof(int)){
            out = uni.val;
            return true;
        }
        return false;
    }
    inline bool readUInt(unsigned int& out){
        using TYPE = unsigned int;
        UniUInt uni;
        if(readCount(sizeof(TYPE), uni.arr) == sizeof(TYPE)){
            out = uni.val;
            return true;
        }
        return false;
    }
    inline bool readLong(long long int & out){
        using TYPE = long long;
        UniLong uni;
        if(readCount(sizeof(TYPE), uni.arr) == sizeof(TYPE)){
            out = uni.val;
            return true;
        }
        return false;
    }
    inline bool readULong(unsigned long long & out){
        using TYPE = unsigned long long;
        UniULong uni;
        if(readCount(sizeof(TYPE), uni.arr) == sizeof(TYPE)){
            out = uni.val;
            return true;
        }
        return false;
    }
    inline String readRawString(Count count){
        std::vector<char> vec;
        vec.resize(count);
        auto actCnt = readCount(count, vec.data());
        return actCnt > 0 ? String(vec.data(), actCnt) : "";
    }
    inline String readString(){
        unsigned int size = 0;
        if(readUInt(size)){
            return readRawString(size);
        }
        return "";
    }
    inline bool readChar(char& out){
        return readCount(1, &out) > 0;
    }
    bool readline(std::string& str) {
        str.clear();
        char ch;
        while (readChar(ch)) {
            if (ch == '\n') {
                // unix: LF
                return true;
            }
            if (ch == '\r') {
                // dos: CRLF
                // read LF
                if (readChar(ch) && ch != '\n') {
                    // mac: CR
                    //fseek(m_file, -1, SEEK_CUR);
                    seekDelta(-1);
                }
                return true;
            }
            str += ch;
        }
        return str.length() != 0;
    }
    void readLines(std::function<void(int,CString)> func){
        String str;
        int lineIdx = 0;
        while (readline(str)) {
            func(lineIdx++, str);
        }
    }
    std::vector<String> readLines(std::function<bool(String&)> func_filter = nullptr){
        std::vector<String> vec;
        String str;
        while (readline(str)) {
            if(!func_filter || !func_filter(str)){
                vec.push_back(str);
            }
        }
        return vec;
    }
    template<typename T>
    std::vector<T> readList(std::function<void(String&, T&)> convertor,
                            std::function<bool(String&)> func_filter = nullptr){
        auto lines = readLines(func_filter);
        std::vector<T> ret;
        ret.resize(ret.size());
        for(int i = 0 ; i < (int)lines.size(); ++i){
            convertor(lines[i], ret[i]);
        }
        return ret;
    }
};

template<typename T>
class AbsInputStream : public InputStream{
public:
    virtual ~AbsInputStream(){}

    bool open(CString s, CString param) override{
        return (static_cast<T*>(this))->open(s, param);
    }
    void close() override{
        return (static_cast<T*>(this))->close();
    }
    //return actual read count . -1 for failed.
    size_t readCount(Count count, char* outArr) override{
        return (static_cast<T*>(this))->readCount(count, outArr);
    }
    void seekDelta(Count offset) override{
        (static_cast<T*>(this))->seekDelta(offset);
    }
    void reset() override{
        (static_cast<T*>(this))->reset();
    }
    Count leftSize() override{
        return (static_cast<T*>(this))->leftSize();
    }
    Count totalSize() override{
        return (static_cast<T*>(this))->totalSize();
    }
};
}
