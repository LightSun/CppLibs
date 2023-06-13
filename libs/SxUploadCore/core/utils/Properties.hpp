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
        String& str = m_map[key];
        if(!str.empty()){
            out = h7::utils::split(",", str);
        }else{
            //check assign array len
            int len = getInt(key+ "-arr-len", 0);
            if(len > 0){
                out.reserve(len);
                for(int i = 0 ; i < len ; ++i){
                    out.push_back(getProperty(key + "[" + std::to_string(i) + "]"));
                }
                return;
            }
            //check sub arr
            String subs = getProperty(key + "-sub-arrs");
            if(!subs.empty()){
                 auto list = h7::utils::split(",", subs);
                 for(String& str: list){
                    getVector(str, out);
                 }
                 return;
            }
            //check format
            String fmt = getProperty(key + "-FORMAT");
            if(!fmt.empty()){
                handle_vector(fmt, out);
                return;
            }
            //handle index format
            for(int i = 0 ; ; ++i){
                String prop = getProperty(key + "[" + std::to_string(i) + "]");
                if(prop.empty()){
                    break;
                }
                out.push_back(prop);
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

private:
    /*
children_dirs-FORMAT=${MI_NUO_WA_DIR}/7月/15日/<<_names>>/<<_category>>
_names-FORMAT=<<names>>
names=01-梁彰,02-杨晓慧
_category-FORMAT=<<category>>
category=AI诊断,甲状腺
*/
    void handle_vector(CString fmt, std::vector<String>& out){
#define __H7_PROP_FMT_START "<<"
#define __H7_PROP_FMT_END ">>"
        std::map<String, std::vector<String>> repMap;
        int start_pos = 0;
        while (true) {
            int id1 = fmt.find(__H7_PROP_FMT_START, start_pos);
            if(id1 < 0){
                break;
            }
            int id2 = fmt.find(__H7_PROP_FMT_END,
                               id1 + strlen(__H7_PROP_FMT_START));
            if(id2 < 0){
                break;
            }
            String name = fmt.substr(id1 + strlen(__H7_PROP_FMT_START),
                     id2 - (id1 + strlen(__H7_PROP_FMT_START)));
            std::vector<String> _names;
            getVector(name, _names);
            if(_names.size() > 0){
                String rep_name = __H7_PROP_FMT_START + name + __H7_PROP_FMT_END;
                repMap[rep_name] = _names;
            }
            start_pos = id2 + strlen(__H7_PROP_FMT_END);
        }
        //replace
        std::vector<String> fmts = {fmt};
        auto it = repMap.begin();
        for(;it != repMap.end(); it ++){
            fmts = do_replace(fmts, it->first, it->second);
        }
        //add to ret
        out.insert(out.end(), fmts.begin(), fmts.end());
    }
    std::vector<String> do_replace(std::vector<String>& fmts,
                                        CString rep_name,
                                        std::vector<String>& _names){
        std::vector<String> newFmts;
        for(String& fmt: fmts){
            for(String& str: _names){
                newFmts.push_back(utils::replace(rep_name, str, fmt));
            }
        }
        return newFmts;
    }
public:
    std::map<String,String> m_map;
};

}

#endif // PROPERTIES_H
