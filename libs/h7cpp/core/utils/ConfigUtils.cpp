#include <fstream>
#include "utils/ConfigUtils.h"
#include "utils/string_utils.hpp"
#include "utils/FileUtils.h"
#include "utils/StringBuffer.h"

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#ifdef WIN32
#include <windows.h>                 //Windows API   FindFirstFile
#include <shlwapi.h>
#endif


namespace h7 {

static void _readProperties(std::ifstream& m_stream, std::map<String, String>& prop){
    String str;
    int index;
    while (getline(m_stream, str)) {
        utils::trimLastR(str);
        h7::utils::trim(str);
        if(str.empty()){
            continue;
        }
        //anno
        if(str.c_str()[0] == '#' || h7::utils::startsWith(str, "//")){
            continue;
        }
        //printf("prop_line: %s\n", str.c_str());
        index = str.find("=");
        MED_ASSERT(index >= 0);
        String _s = str.substr(0, index);
        h7::utils::trim(_s);
        if(index == (int)str.length() - 1){
            prop[_s] = "";
        }else{// abc=1235 : 3
            prop[_s] = str.substr(index + 1, str.length() - index - 1);
        }
    }
}
static String _getRealValue(std::map<String,String>& map, CString key){
    String val = map[key];
    if(!val.empty()){
        String _key;
        int s1, s2;
        String newVal;
        while (true) {
            s1 = val.find("${");
            if(s1 >=0 ){
                s2 = val.find("}", s1 + 2);
                _key = utils::subString(val, s1 + 2, s2);
                newVal = map[_key];
                if(newVal.empty()){
                    printf("Properties >> can't find replacement, for key = %s.\n", _key.c_str());
                    break;
                }else{
                    std::cout << "start replace: " << _key << " --> "
                              << newVal << std::endl;
                    val = h7::utils::replace("\\$\\{"+ _key + "\\}", newVal, val);
                }
            }else{
                break;
            }
        }
    }
    return val;
}

static void resolveInclude(const std::vector<String>& dirs,
                                    std::map<String, String>& m_map){
    bool config_changed = false;;
    std::map<String,String> _map(m_map);
    auto end = _map.end();
    for(auto it = _map.begin(); it != end ; it ++){
        String key = it->first;
        if(key == "$$INCLUDES"){
            //remove it
            m_map.erase(m_map.find(key));
            //find file
            auto vec = h7::utils::split(",", it->second);
            for(String& s: vec){
                String path = FileUtils::getFilePath(s, dirs);
                if(path.empty()){
                    String msg = "can't find include file. " + s;
                    fprintf(stderr, "%s, please check your dirs\n", msg.data());
                    return;
                }
                ConfigUtils::loadProperties(path, m_map);
            }
            config_changed = true;
        }else if(key == "$$INCLUDE"){
            m_map.erase(m_map.find(key));
            //find file
            String path = FileUtils::getFilePath(it->second, dirs);
            if(path.empty()){
                String msg = "can't find include file. " + it->second;
                fprintf(stderr, "%s, please check your dirs\n", msg.data());
                return;
            }
            ConfigUtils::loadProperties(path, m_map);
            config_changed = true;
        }
    }
    //include may nested
    if(config_changed){
        resolveInclude(dirs, m_map);
    }
}

void ConfigUtils::loadPropertiesFromBuffer(CString buffer,
                                           std::map<String, String>& prop){
    h7::StringBuffer sb(buffer);
    String str;
    int index;
    while (sb.readLine(str)) {
        //utils::trimLastR(str);
        h7::utils::trim(str);
        if(str.empty()){
            continue;
        }
        //anno
        if(str.c_str()[0] == '#' || h7::utils::startsWith(str, "//")){
            continue;
        }
        //printf("prop_line: %s\n", str.c_str());
        index = str.find("=");
        MED_ASSERT(index >= 0);
        String _s = str.substr(0, index);
        h7::utils::trim(_s);
        if(index == (int)str.length() - 1){
            prop[_s] = "";
        }else{// abc=1235 : 3
            prop[_s] = str.substr(index + 1, str.length() - index - 1);
        }
    }
}

void ConfigUtils::loadProperties(CString prop_file,
                                                    std::map<String,String>& m_map){
    std::ifstream m_stream;
    m_stream.open(prop_file, std::ios::in);
    std::string msg = "FileReader >> open file failed: ";
    msg += prop_file;
    MED_ASSERT_X(m_stream.is_open(), msg);
    //
    _readProperties(m_stream, m_map);
    m_stream.close();
}

void ConfigUtils::resolveProperties(const std::vector<String>& dirs,
                                    std::map<String, String>& m_map){
    //first handle includes
    {
        resolveInclude(dirs, m_map);
    }
    //do resolve
    {
        auto end = m_map.end();
        for(auto it = m_map.begin(); it != end ; it ++){
            m_map[it->first] = _getRealValue(m_map, it->first);
        }
    }
}

}
