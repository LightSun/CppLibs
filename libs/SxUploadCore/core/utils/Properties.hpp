#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <map>
#include <vector>
#include "utils/Value.hpp"
#include "utils/string_utils.hpp"

#define __H7_Properties_G_LIST(func)\
    std::vector<String> vec;\
    getVector(key, vec);\
    out.resize(vec.size());\
    for(int i = 0 ; i < (int)vec.size() ; ++i){\
        out[i] = Value(vec[i]).func(def);\
    }

#define __H7_Properties_G_LIST0(func)\
    std::vector<String> vec;\
    getVector(key, vec);\
    out.resize(vec.size());\
    for(int i = 0 ; i < (int)vec.size() ; ++i){\
        out[i] = Value(vec[i]).func();\
    }

namespace h7 {

class Properties
{
public:
    using String = std::string;
    using CString = const std::string&;
    Properties(const std::map<String,String>& map):m_map(map){
    }
    Properties(){}
    Properties(const Properties& prop):m_map(prop.m_map){
    }

    int size() const{
        return m_map.size();
    }
    void clear(){
        m_map.clear();
    }
    String& getProperty(CString key){
        return m_map[key];
    }
    void setProperty(CString key, CString val){
        m_map[key] = val;
    }
    bool hasProperty(CString key){
        return m_map.find(key) != m_map.end();
    }
    String getString(CString key, CString defVal = ""){
        auto it = m_map.find(key);
        if(it != m_map.end()){
            return it->second;
        }
        return defVal;
    }
    bool getBool(CString key){
        Value v(getString(key));
        return v.getBool();
    }
    int getInt(CString key,int defVal = 0){
        Value v(getString(key));
        return v.getInt(defVal);
    }
    long long getLong(CString key, long long defVal = 0){
        Value v(getString(key));
        return v.getLong(defVal);
    }
    unsigned int getUInt(CString key, unsigned int defVal = 0){
        Value v(getString(key));
        return v.getUInt(defVal);
    }
    float getFloat(CString key, float defVal = 0){
        Value v(getString(key));
        return v.getFloat(defVal);
    }
    double getDouble(CString key, double defVal = 0){
        Value v(getString(key));
        return v.getDouble(defVal);
    }
    void getVector(CString key,std::vector<String>& out){
        String str = m_map[key];
        if(!str.empty()){
            out = h7::utils::split(",", str);
        }else{
            int len = getInt(key+ "-arr-len", 0);
            if(len > 0){
                out.reserve(len);
                for(int i = 0 ; i < len ; ++i){
                    out.push_back(getProperty(key + "[" + std::to_string(i) + "]"));
                }
            }
        }
    }
    void getVectorInt(CString key,std::vector<int>& out, int def = 0){
        __H7_Properties_G_LIST(getInt)
    }
    void getVectorUInt(CString key,std::vector<unsigned int>& out,
                         unsigned int def = 0){
        __H7_Properties_G_LIST(getUInt)
    }
    void getVectorLong(CString key,std::vector<long long>& out,
                         long long def = 0){
        __H7_Properties_G_LIST(getLong)
    }
    void getVectorULong(CString key,std::vector<unsigned long long>& out,
                         unsigned long long def = 0){
        __H7_Properties_G_LIST(getULong)
    }
    void getVectorFloat(CString key,std::vector<float>& out, float def = 0){
        __H7_Properties_G_LIST(getFloat)
    }
    void getVectorDouble(CString key,std::vector<double>& out, double def = 0){
        __H7_Properties_G_LIST(getDouble)
    }
    void getVectorBool(CString key,std::vector<bool>& out){
        __H7_Properties_G_LIST0(getBool)
    }
public:
    std::map<String,String> m_map;
};

}

#endif // PROPERTIES_H
