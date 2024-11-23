#include "SuperConfig.h"
#include "core/utils/FileUtils.h"
#include "core/utils/string_utils.hpp"

namespace h7 {

struct TwoResolver: public IConfigResolver{
    IConfigResolver* reso1;
    IConfigResolver* reso2;
    TwoResolver(IConfigResolver* reso1, IConfigResolver* reso2):
        reso1(reso1), reso2(reso2){}

    ConfigItemHolder resolveInclude(String curDir, CString name, String& errorMsg) override{
        auto h = reso1->resolveInclude(curDir, name, errorMsg);
        if(h.ci == nullptr){
            return reso2->resolveInclude(curDir, name, errorMsg);
        }
        return h;
    }
    ConfigItemHolder resolveSuper(String curDir, CString name, String& errorMsg) override{
        auto h = reso1->resolveSuper(curDir, name, errorMsg);
        if(h.ci == nullptr){
            return reso2->resolveSuper(curDir, name, errorMsg);
        }
        return h;
    }
    String resolveValue(CString name, String& errorMsg) override{
        auto h = reso1->resolveValue(name, errorMsg);
        if(h.empty()){
            return reso2->resolveValue(name, errorMsg);
        }
        return h;
    }
};

struct WrappedResolver: public IWrappedResolver{

    std::map<String, String> kv;
    IConfigResolver* m_base;
    ConfigItemCache* m_cache;

    WrappedResolver(ConfigItemCache* cache, IConfigResolver* base):m_base(base), m_cache(cache){}

    ConfigItemHolder resolveInclude(String curDir, CString name, String& errorMsg) override{
        auto it = m_cache->incMap.find(name);
        if(it != m_cache->incMap.end()){
            return it->second.ref();
        }
        return m_base->resolveInclude(curDir, name, errorMsg);
    }
    ConfigItemHolder resolveSuper(String curDir, CString name, String& errorMsg) override{
        auto it = m_cache->superMap.find(name);
        if(it != m_cache->superMap.end()){
            return it->second.ref();
        }
        return m_base->resolveSuper(curDir, name, errorMsg);
    }
    String resolveValue(CString name, String& errorMsg) override{
        auto it = kv.find(name);
        if(it != kv.end()){
            return it->second;
        }
        return m_base->resolveValue(name, errorMsg);
    }

    String resolves(ConfigItem* parItem) override{
        String errMsg;
        auto reso = this;
        auto& dir = parItem->dir;
        auto& resoSuperMap = m_cache->superMap;
        for(auto& _inc: parItem->includes){
            String inc = _inc;
            h7::utils::trim(inc);
            if(!getRealValue(inc, this, errMsg)){
                return errMsg;
            }
            {
                int pos = inc.find("./");
                if(pos == 0){
                    inc = dir + inc.substr(pos + 1);
                }
            }
            //handle '../../<...>'
            {
                int parCount = 0;
                while(utils::startsWith(inc, "../")){
                    parCount ++;
                }
                if(parCount > 0){
                    int id = inc.rfind("../");
                    auto suffix = inc.substr(id + 2);
                    String pdir = dir;
                    for(int i = 0 ; i < parCount ; ++i){
                        pdir = FileUtils::getFileDir(pdir);
                    }
                    inc = pdir + suffix;
                }
            }
            //check all files include subs.
            if(utils::endsWith(inc, "/**")){
                auto tdir = inc.substr(0, inc.length() - 3);
                if(tdir == dir){
                    errMsg = "can't include with curDir: " + dir;
                    return errMsg;
                }
                if(FileUtils::isRelativePath(tdir)){
                    tdir = dir + "/" + tdir;
                }
                auto files = FileUtils::getFiles(tdir, true, "");
                for(auto& f1 : files){
                    auto dir1 = FileUtils::getFileDir(f1);
                    if(!resolveIncludeImpl(dir1, f1, errMsg, parItem)){
                        return errMsg;
                    }
                }
            }
             //check all files , exclude sub dirs
            else if(utils::endsWith(inc, "/*")){
                auto tdir = inc.substr(0, inc.length() - 2);
                if(tdir == dir){
                    errMsg = "can't include with curDir: " + dir;
                    return errMsg;
                }
                if(FileUtils::isRelativePath(tdir)){
                    tdir = dir + "/" + tdir;
                }
                auto files = FileUtils::getFiles(tdir, false, "");
                for(auto& f1 : files){
                    auto dir1 = FileUtils::getFileDir(f1);
                    if(!resolveIncludeImpl(dir1, f1, errMsg, parItem)){
                        return errMsg;
                    }
                }
            }
            else{
                if(FileUtils::isRelativePath(inc)){
                    inc = dir + "/" + inc;
                }
                auto dir1 = FileUtils::getFileDir(inc);
                if(!resolveIncludeImpl(dir1, inc, errMsg, parItem)){
                    return errMsg;
                }
            }
        }
        for(auto& inc: parItem->supers){
            auto item = reso->resolveSuper(dir, inc, errMsg);
            if(item.ci == nullptr){
                if(errMsg.empty()){
                    errMsg = "resolveSuper >> can't find " + inc;
                }
                return errMsg;
            }
            parItem->mergeForSuper(item.ci);
            resoSuperMap.emplace(inc, std::move(item));
        }
        {//children can ref prop of parent, but reverse not.
            auto it = parItem->children.begin();
            for(; it != parItem->children.end() ; ++it){
                auto ci = it->second.ci;
                errMsg = this->resolves(ci);
                if(!errMsg.empty()){
                    return errMsg;
                }
            }
        }
        return "";
    }
    String resolveValues(ConfigItem* item) override{
        TwoResolver reso(item, this);
        String errMsg;
        {
            auto it = item->body.begin();
            for(; it != item->body.end() ; ++it){
                if(!getRealValue(it->second, &reso, errMsg)){
                    return errMsg;
                }
                kv[it->first] = it->second;
            }
        }
        {//children can ref prop of parent, but reverse not.
            auto it = item->children.begin();
            for(; it != item->children.end() ; ++it){
                auto ci = it->second.ci;
                errMsg = resolveValues(ci);
                if(!errMsg.empty()){
                    return errMsg;
                }
            }
        }
        return "";
    }

private:
    bool resolveIncludeImpl(CString dir, CString name, String& errMsg, ConfigItem* parItem){
        auto item = resolveInclude(dir, name, errMsg);
        if(item.ci == nullptr){
            if(errMsg.empty()){
                errMsg = "resolveInclude >> can't find " + name;
            }
            return false;
        }       
        if(!parItem->addBrother(item.ref())){
            if(errMsg.empty()){
                errMsg = "addBrother failed >> " + item.ci->name;
            }
            return false;
        }
        m_cache->incMap.emplace(name, std::move(item));
        return true;
    }
};

struct RuntimeEnvConfigResolver: public IConfigResolver{

    ConfigItemCache* pCache;
    ConfigItem* pItem {nullptr};
    std::map<String, String>* prop {nullptr};
    std::map<String, String> prop0;

    RuntimeEnvConfigResolver(ConfigItemCache* cache, ConfigItem* pItem, std::map<String, String>* prop)
        :pCache(cache),pItem(pItem), prop(prop){
    }

    RuntimeEnvConfigResolver(ConfigItemCache* cache, ConfigItem* pItem, const std::map<String, String>& prop)
        :pCache(cache), pItem(pItem){
        this->prop0 = prop;
        this->prop = &prop0;
    }

    ConfigItemHolder resolveInclude(String curDir, CString name, String& errorMsg) override{
        String path = name;
        String errMsg;
        if(IConfigResolver::getRealValue(path, this, errMsg)){
            if(FileUtils::isRelativePath(path)){
                path = curDir + "/" + path;
            }
            if(!FileUtils::isFileExists(path)){
                errorMsg = "can't find file for include: " + path;
                return ConfigItemHolder();
            }
            auto dir = FileUtils::getFileDir(path);
            auto cs = FileUtils::getFileContent(path);
            auto ci = ConfigItem::make(cs, dir, pCache);
            if(!ci){
                return ConfigItemHolder();
            }
            errorMsg = ci->resolve(pCache, prop);
            if(errorMsg.empty()){
                return ConfigItem::newDefaultHolder(ci.release());
            }
        }else{
            errorMsg = "getRealValue failed. val = " + path;
        }
        return ConfigItemHolder();
    }
    ConfigItemHolder resolveSuper(String curDir, CString name, String& errorMsg)override{
        return pItem->resolveSuper(curDir, name, errorMsg);
    }
    String resolveValue(CString n, String&) override{
        return getVal(n);
    }
private:
    String getVal(CString n){
        if(prop){
            auto it = prop->find(n);
            if(it != prop->end()){
                return it->second;
            }
        }
        auto val = getenv(n.data());
        return val ? val : "";
    }
};
bool getRealValueImpl(CString pre, String& val, IConfigResolver* reso, String& errorMsg){
    if(!val.empty()){
        const String first = pre + "{";
        String _key;
        int s1, s2;
        String newVal;
        while (true) {
            s1 = val.find(first);
            if(s1 >=0 ){
                s2 = val.find("}", s1 + 2);
                _key = utils::subString(val, s1 + 2, s2);
                newVal = reso->resolveValue(_key, errorMsg);
                if(newVal.empty()){
                    char buf[1024];
                    snprintf(buf, 1024, "can't find value for key = %s, "
                           "val = %s\n", _key.c_str(), val.data());
                    if(!errorMsg.empty()){
                        errorMsg += "; " + String(buf);
                    }else{
                        errorMsg = String(buf);
                    }
                    return false;
                }else{
                    //std::cout << "start replace: " << _key << " --> "
                    //          << newVal << std::endl;
                    val = utils::replace("\\$\\{"+ _key + "\\}", newVal, val);
                }
            }else{
                break;
            }
        }
    }
    return true;
}
bool IConfigResolver::getRealValue(String& val, IConfigResolver* reso, String& errorMsg){
    return getRealValueImpl("$", val, reso, errorMsg);
}

std::shared_ptr<IConfigResolver> IConfigResolver::newTwoConfigResolver(
        IConfigResolver* reso1, IConfigResolver* reso2){
    return std::make_shared<TwoResolver>(reso1, reso2);
}
std::shared_ptr<IWrappedResolver> IConfigResolver::newWrappedConfigResolver(
        ConfigItemCache* cache, IConfigResolver* reso1){
    return std::make_shared<WrappedResolver>(cache, reso1);
}
std::shared_ptr<IConfigResolver> IConfigResolver::newRuntimeConfigResolver(
        ConfigItemCache* cache, ConfigItem* item, Map* prop){
    return std::make_shared<RuntimeEnvConfigResolver>(cache, item, prop);
}
std::shared_ptr<IConfigResolver> IConfigResolver::newRuntimeConfigResolver(
        ConfigItemCache* cache, ConfigItem* item, const Map& prop){
    return std::make_shared<RuntimeEnvConfigResolver>(cache, item, prop);
}
}
