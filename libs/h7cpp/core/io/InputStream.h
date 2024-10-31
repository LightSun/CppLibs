#pragma once

#include <string>
#include <vector>
#include <functional>

#define DEF_IS_API_VIRTUAL(a) virtual a = 0

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
};

class InputStream{
public:
    using Count = long long;
    using String = std::string;
    using CString = const std::string&;

    DEF_IS_API_VIRTUAL(bool open(CString file, CString param));
    DEF_IS_API_VIRTUAL(void close());
    //return actual read count . -1 for failed.
    DEF_IS_API_VIRTUAL(size_t readCount(Count count, char* buf));
    DEF_IS_API_VIRTUAL(void seekDelta(Count offset));
    //reset to first position
    DEF_IS_API_VIRTUAL(void reset());
    //return -1 for failed
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
        String str = readString(sizeof (double));
        if(!str.empty()){
            out = StringConvertor::atof(str.c_str());
            return true;
        }
        return false;
    }
    inline bool readFloat(float& out){
        String str = readString(sizeof (float));
        if(!str.empty()){
            out = (float)StringConvertor::atof(str.c_str());
            return true;
        }
        return false;
    }
    inline bool readShort(short& out){
        String str = readString(sizeof (short));
        if(!str.empty()){
            out = (short)StringConvertor::atoi(str.c_str());
            return true;
        }
        return false;
    }
    inline bool readBool(bool& out){
        auto str0 = readString(4);
        if(str0 == "TRUE" || str0 == "true"){
            out = true;
            return true;
        }
        seekDelta(-4);
        String str = readString(sizeof (bool));
        if(!str.empty()){
            out = StringConvertor::atoi(str.c_str()) != 0;
            return true;
        }
        out = false;
        return false;
    }
    inline bool readInt(int& out){
        String str = readString(sizeof (int));
        if(!str.empty()){
            out = StringConvertor::atoi(str.c_str());
            return true;
        }
        return false;
    }
    inline bool readLong(long long int & out){
        String str = readString(sizeof (long long int));
        if(!str.empty()){
            out = StringConvertor::atoll(str.c_str());
            return true;
        }
        return false;
    }
    inline String readString(Count count){
        std::vector<char> vec;
        vec.resize(count);
        auto actCnt = readCount(count, vec.data());
        return actCnt > 0 ? String(vec.data(), actCnt) : "";
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
    void readLines(std::vector<String>& vec, std::function<bool(String&)> func_filter = nullptr){
        String str;
        while (readline(str)) {
            if(!func_filter || !func_filter(str)){
                vec.push_back(str);
            }
        }
    }
};

template<typename T>
class AbsInputStream : public InputStream{
public:
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
