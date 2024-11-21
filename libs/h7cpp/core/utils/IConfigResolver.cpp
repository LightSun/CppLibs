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

    std::unordered_map<String, ConfigItemHolder> incMap;
    std::unordered_map<String, ConfigItemHolder> superMap;
    std::map<String, String> kv;

    WrappedResolver(IConfigResolver* base):m_base(base){}

    ConfigItemHolder resolveInclude(String curDir, CString name, String& errorMsg) override{
        auto it = incMap.find(name);
        if(it != incMap.end()){
            return it->second.ref();
        }
        return m_base->resolveInclude(curDir, name, errorMsg);
    }
    ConfigItemHolder resolveSuper(String curDir, CString name, String& errorMsg) override{
        auto it = superMap.find(name);
        if(it != superMap.end()){
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
        auto& resoIncMap = incMap;
        auto& resoSuperMap = superMap;
        for(auto& inc: parItem->includes){
            auto item = reso->resolveInclude(dir, inc, errMsg);
            if(!errMsg.empty()){
                return errMsg;
            }
            if(item.ci == nullptr){
                return "resolveInclude >> can't find " + inc;
            }
            parItem->mergeForInclude(item.ci);
            resoIncMap.emplace(inc, std::move(item));
        }
        for(auto& inc: parItem->supers){
            auto item = reso->resolveSuper(dir, inc, errMsg);
            if(!errMsg.empty()){
                return errMsg;
            }
            if(item.ci == nullptr){
                return "resolveSuper >> can't find " + inc;
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
    IConfigResolver* m_base;
};

struct RuntimeEnvConfigResolver: public IConfigResolver{

    ConfigItem* pItem {nullptr};
    std::map<String, String>* prop {nullptr};
    std::map<String, String> prop0;

    RuntimeEnvConfigResolver(ConfigItem* pItem, std::map<String, String>* prop)
        :pItem(pItem), prop(prop){
    }

    RuntimeEnvConfigResolver(ConfigItem* pItem, const std::map<String, String>& prop)
        :pItem(pItem){
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
                errorMsg = "cant find file for include: " + path;
                return ConfigItemHolder();
            }
            auto dir = FileUtils::getFileDir(path);
            auto cs = FileUtils::getFileContent(path);
            auto ci = ConfigItem::make();
            ci->loadFromBuffer(cs, dir);
            errorMsg = ci->resolve(prop);
            if(errorMsg.empty()){
                return ConfigItemHolder::newDefault(ci.release());
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
        IConfigResolver* reso1){
    return std::make_shared<WrappedResolver>(reso1);
}
std::shared_ptr<IConfigResolver> IConfigResolver::newRuntimeConfigResolver(
        ConfigItem* item, Map* prop){
    return std::make_shared<RuntimeEnvConfigResolver>(item, prop);
}
std::shared_ptr<IConfigResolver> IConfigResolver::newRuntimeConfigResolver(
        ConfigItem* item, const Map& prop){
    return std::make_shared<RuntimeEnvConfigResolver>(item, prop);
}
}
