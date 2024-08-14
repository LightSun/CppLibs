#include "EDManager.h"
#include "utils/FileUtils.h"
#include "utils/string_utils.hpp"
#include "utils/PerformanceHelper.h"
#include "CacheManager.h"

using namespace h7;

EDManager::~EDManager(){
    if(m_cacheM){
        delete m_cacheM;
        m_cacheM = nullptr;
    }
}
void EDManager::compress(bool verify){
    MED_ASSERT(!m_encFileDirs.empty());
    MED_ASSERT(!m_encFileSuffixes.empty());
    MED_ASSERT(!m_encOutDesc.empty());
    //
    h7::PerformanceHelper ph;
    ph.begin();
    //encFileDirs.
    using namespace h7;
    List<String> files;
    List<String> keys;
    {
    auto dirs = h7::utils::split(",", m_encFileDirs);
    auto suffixes = h7::utils::split(",", m_encFileSuffixes);
    for(auto& dir : dirs){
        List<String> l;
        for(auto& suf : suffixes){
            auto list = FileUtils::getFiles(dir, true, suf);
            l.insert(l.end(), list->list.begin(), list->list.end());
        }
        files.insert(files.end(), l.begin(), l.end());
        //
        for(auto& s : l){
            keys.push_back(s.substr(dir.length()));
        }
    }
    }
    compress(files, keys, verify);
}
void EDManager::compress(CList<String> files, CList<String> keys,bool verify){
    using namespace h7;
    auto desc = h7::utils::split(",", m_encOutDesc);
    MED_ASSERT(desc.size() == 3);
    //
    m_keys = keys;
    h7::PerformanceHelper ph;
    ph.begin();
    //dir,recordName,dataName
    //
    {
        CacheManager cm(2 << 30); //max 2G
        int size = files.size();
        for(int i = 0 ; i < size ; ++i){
            cm.addFileItem(m_keys[i], files[i]);
        }
        cm.compressTo(desc[0], desc[1], desc[2]);
        ph.print("compress");
    }
    //
    if(verify){
        ph.begin();
        CacheManager cm(2 << 30);
        cm.load(desc[0], desc[1], desc[2]);
        int size = files.size();
        for(int i = 0 ; i < size ; ++i){
            String rawContent = FileUtils::getFileContent(files[i]);
            String out;
            cm.getItemData(m_keys[i], out);
            if(rawContent != out){
                fprintf(stderr, " verify >> enc-dec failed, key = '%s'\n", m_keys[i].data());
            }
        }
        ph.print("verify");
    }
}
void EDManager::load(CString encOutDesc){
    h7::PerformanceHelper ph;
    ph.begin();
    auto desc = h7::utils::split(",", encOutDesc);
    MED_ASSERT(desc.size() == 3);
    //
    using namespace h7;
    if(m_cacheM){
        delete m_cacheM;
    }
    m_cacheM = new CacheManager(2 << 30);
    m_cacheM->load(desc[0], desc[1], desc[2]);
    ph.print("loadED");
}
String EDManager::getItem(CString key){
    if(!m_cacheM){
        return "";
    }
    String out;
    m_cacheM->getItemData(key, out);
    return out;
}
