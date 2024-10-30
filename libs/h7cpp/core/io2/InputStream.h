#pragma once

#include <string>
#include <vector>
#include <functional>

namespace h7 {

struct StringConvertor{
    using String = std::string;
    using CString = const std::string&;

    double atof(const char* str){
        return std::stof(str);
    }
    int atoi(const char* str){
        return std::stoi(str);
    }
    long long atoll(const char* str){
        return std::stoll(str);
    }
};

/**
    bool open(CString file, CString param);
    void close()
    //return actual read count . -1 for failed.
    int readCount(Count count, char* outArr, int arrLen);
    void seekDelta(Count offset);
    void reset();
    size_t leftSize();
    size_t totalSize();
 */
template<typename T, typename Convertor = StringConvertor>
class IInputStream{
public:
    using Count = long long;
    using String = std::string;
    using CString = const std::string&;

    bool open(CString file, CString param){
        return (static_cast<T*>(this))->open(file, param);
    }
    void close(){
        return (static_cast<T*>(this))->close();
    }
    //return actual read count . -1 for failed.
    int readCount(Count count, char* outArr, int arrLen){
        return (static_cast<T*>(this))->readCount(count, outArr, arrLen);
    }
    void seekDelta(Count offset){
        (static_cast<T*>(this))->seekDelta(offset);
    }
    void reset(){
        (static_cast<T*>(this))->reset();
    }
    Count leftSize(){
        return (static_cast<T*>(this))->leftSize();
    }
    size_t totalSize(){
        return (static_cast<T*>(this))->totalSize();
    }

    //------------------------
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
            out = m_cvt.atof(str.c_str());
            return true;
        }
        return false;
    }
    inline bool readFloat(float& out){
        String str = readString(sizeof (float));
        if(!str.empty()){
            out = (float)m_cvt.atof(str.c_str());
            return true;
        }
        return false;
    }
    inline bool readShort(short& out){
        String str = readString(sizeof (short));
        if(!str.empty()){
            out = (short)m_cvt.atoi(str.c_str());
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
            out = m_cvt.atoi(str.c_str()) != 0;
            return true;
        }
        out = false;
        return false;
    }
    inline bool readInt(int& out){
        String str = readString(sizeof (int));
        if(!str.empty()){
            out = m_cvt.atoi(str.c_str());
            return true;
        }
        return false;
    }
    inline bool readLong(long long int & out){
        String str = readString(sizeof (long long int));
        if(!str.empty()){
            out = m_cvt.atoll(str.c_str());
            return true;
        }
        return false;
    }
    inline String readString(Count count){
        std::vector<char> vec;
        vec.resize(count);
        auto actCnt = readCount(count, vec.data(), count);
        return String(vec.data(), actCnt);
    }
    inline bool readChar(char& out){
        return readCount(1, &out, 1) > 0;
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
            if(!func_filter(str)){
                vec.push_back(str);
            }
        }
    }

private:
    Convertor m_cvt;
};
}
