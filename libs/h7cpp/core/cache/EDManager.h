#pragma once

#include <string>
#include <vector>
#include <map>
#include "common/common.h"

namespace h7 {
    class CacheManager;
}

namespace h7 {

//using CString = const std::string &;
//using String = std::string;
template<typename T>
using List = std::vector<T>;

template<typename T>
using CList = const std::vector<T>&;

class EDManager
{
public:
    //enc
    EDManager(CString encFileDirs, CString encFileSuffixes, CString encOutDesc):
        m_encFileDirs(encFileDirs),m_encFileSuffixes(encFileSuffixes),
        m_encOutDesc(encOutDesc){}
    //for dec
    EDManager() = default;
    ~EDManager();

    List<String>& getKeys(){return m_keys;}
    //void remap(const std::map<String,String>& rmap);

    void compress(bool verify);
    void compress(CList<String> files, CList<String> keys,bool verify);

    void load(CString encOutDesc);
    String getItem(CString key);

private:
    List<String> m_keys;
    String m_encFileDirs;
    String m_encFileSuffixes;
    String m_encOutDesc;
    h7::CacheManager* m_cacheM {nullptr};
};

}
