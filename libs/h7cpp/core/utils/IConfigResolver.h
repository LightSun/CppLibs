#pragma once

#include <string>
#include <map>
#include <functional>
#include <memory>

namespace h7 {

struct ConfigItem;

using String = std::string;
using CString = const std::string&;
using Map = std::map<String, String>;

struct ConfigItemHolder{
    ConfigItem* ci;
    std::function<void(ConfigItem*)> del;

    ~ConfigItemHolder(){
        if(ci && del){
            del(ci);
            ci = nullptr;
        }
    }
    ConfigItemHolder(){ci = nullptr; del = nullptr;}
    ConfigItemHolder(ConfigItem* ci, std::function<void(ConfigItem*)> del = nullptr):
        ci(ci), del(del){
    }
    ConfigItemHolder(ConfigItemHolder&& ch){
        this->ci = ch.ci;
        this->del = ch.del;
        ch.ci = nullptr;
    }
    ConfigItemHolder(ConfigItemHolder const& _ch){
        auto& ch = const_cast<ConfigItemHolder&>(_ch);
        this->ci = ch.ci;
        this->del = ch.del;
        ch.ci = nullptr;
    }

    static inline ConfigItemHolder newDefault(ConfigItem* ci){
        return ConfigItemHolder(ci, [](ConfigItem* it){
            delete it;
        });
    }

    ConfigItemHolder& operator=(ConfigItemHolder&& ch){
        this->ci = ch.ci;
        this->del = ch.del;
        ch.ci = nullptr;
        return *this;
    }
    ConfigItemHolder& operator=(ConfigItemHolder const& _ch){
        auto& ch = const_cast<ConfigItemHolder&>(_ch);
        this->ci = ch.ci;
        this->del = ch.del;
        ch.ci = nullptr;
        return *this;
    }
    ConfigItemHolder ref(){
        return ConfigItemHolder(ci);
    }
};

struct IWrappedResolver;

struct IConfigResolver{
    virtual ~IConfigResolver(){}
    virtual ConfigItemHolder resolveInclude(String curDir, CString name, String& errorMsg) = 0;
    virtual ConfigItemHolder resolveSuper(String curDir, CString name, String& errorMsg) = 0;
    virtual String resolveValue(CString name, String& errorMsg) = 0;

    static bool getRealValue(String& val, IConfigResolver* reso, String& errorMsg);
    static std::shared_ptr<IConfigResolver> newTwoConfigResolver(
            IConfigResolver* reso1, IConfigResolver* reso2);
    static std::shared_ptr<IWrappedResolver> newWrappedConfigResolver(
            IConfigResolver* reso1);
    static std::shared_ptr<IConfigResolver> newRuntimeConfigResolver(
            ConfigItem* item, Map* prop);
    static std::shared_ptr<IConfigResolver> newRuntimeConfigResolver(
            ConfigItem* item, const Map& prop);
};

struct IWrappedResolver: public IConfigResolver{
    virtual String resolves(ConfigItem* parItem) = 0;
    virtual String resolveValues(ConfigItem* item) = 0;
};

}

