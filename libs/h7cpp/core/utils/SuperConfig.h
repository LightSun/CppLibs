#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <set>
#include <memory>
#include <functional>
#include "core/utils/IConfigResolver.h"

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
/*unitTest:
1, only props.
2, include a class-like.
3, include extend. 找兄弟 - 包括inlcude过来的
4, include files.
5, final test
*/
namespace h7 {

using String = std::string;
using CString = const std::string&;
using Map = std::map<String, String>;

struct ConfigItem;
using UPConfigItem = std::unique_ptr<ConfigItem>;

struct ConfigItem : public IConfigResolver{
    enum{
        kFlag_PUBLIC = 0x0001
    };
    std::set<String> includes;
    std::set<String> supers;
    String dir;
    std::map<String, String> body;
    std::unordered_map<String, ConfigItemHolder> children;
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
        children[key] = ConfigItemHolder::newDefault(item.release());
    }
    void loadFromBuffer(CString buffer, CString curDir);
    //return error msg.
    String resolve(const Map& prop);
    String resolve(Map* prop);
    String resolve(IConfigResolver* resolver);

    UPConfigItem copy();
    void dump(int indent = 0);
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
    void dump(){m_item.dump();}

private:
    Map* m_env;
    ConfigItem m_item;
};

}
