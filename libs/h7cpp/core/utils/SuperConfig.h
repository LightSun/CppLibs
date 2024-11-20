#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <set>
#include <memory>
#include <functional>

/**
//cur dir
//@{CUR_DIR}: @ means from outside.
//${CUR_DIR}: means from config.
$(INCLUDE)=@{CUR_DIR}/a.lp
$(INCLUDE)=b.lp,c.lp
//include all
$(INCLUDE)=.
$(INCLUDE)=@{CUR_DIR}/.
ABC=xxx,prop

public A{
B{
  C{}
  D{}
}
}
conf{
    sub{
    }
    stu: A.B.C, A.B.D{
        name = abc,
        age = 16,
        classes = [${math}, eng, sport],
        kv{
            a = 1, b = 2, c = 3
        }
    }
}
 */

namespace h7 {

using String = std::string;
using CString = const std::string&;
using Map = std::map<String, String>;

struct ConfigItem;

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

struct IConfigResolver{
    virtual ~IConfigResolver(){}
    virtual ConfigItemHolder resolveInclude(String curDir, CString name, String& errorMsg) = 0;
    virtual ConfigItemHolder resolveSuper(String curDir, CString name, String& errorMsg) = 0;
    virtual String resolveValue(CString name, String& errorMsg) = 0;

    static bool getRealValue(String& val, IConfigResolver* reso, String& errorMsg);
};
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


using UPConfigItem = std::unique_ptr<ConfigItem>;
struct ConfigItem : public IConfigResolver{
    enum{
        kFlag_PUBLIC = 0x0001
    };
    std::set<String> includes;
    std::set<String> supers;
    String dir;
    std::map<String, String> body;
    std::unordered_map<String, UPConfigItem> children;
    //std::vector<String> array;
    int flags {0};
//no copy
    ConfigItem() = default;
    virtual ~ConfigItem() = default;
    ConfigItem(ConfigItem const& other) = delete;
    ConfigItem& operator=(ConfigItem const& other) = delete;
    ConfigItem(ConfigItem&& other) = delete;
    ConfigItem& operator=(ConfigItem&& other) = delete;

    static UPConfigItem make(){
        return std::make_unique<ConfigItem>();
    }
    static UPConfigItem make(CString buffer, CString curDir){
        auto item = make();
        item->loadFromBuffer(buffer, curDir);
        return item;
    }
    bool isPublic()const {return (flags & kFlag_PUBLIC) == kFlag_PUBLIC;}
    void putChild(CString key, UPConfigItem item){
        children[key] = std::move(item);
    }
    void loadFromBuffer(CString buffer, CString curDir);
    //return error msg.
    String resolve(const Map& prop);
    String resolve(Map* prop);
    String resolve(IConfigResolver* resolver);

    UPConfigItem copy();
    void mergeForInclude(ConfigItem* ci);
    void mergeForSuper(ConfigItem* ci);
    //==================
    ConfigItemHolder resolveInclude(String, CString, String&) override{
        return ConfigItemHolder();
    }
    ConfigItemHolder resolveSuper(String, CString, String&) override;
    String resolveValue(CString name, String& errorMsg) override;
};

class SuperConfig
{
public:
    SuperConfig(Map* env = nullptr):m_env(env){}

    bool loadFromBuffer(CString buffer, CString curDir);
    bool loadFromFile(CString file);
    String getString(CString key, CString def = "");

private:
    Map* m_env;
    ConfigItem m_item;
};

}
