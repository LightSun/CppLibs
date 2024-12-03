#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <set>
#include <memory>
#include <functional>
#include "core/utils/IConfigResolver.h"
#include "core/utils/IKeyValue.h"
/**
//cur dir
include ${CUR_DIR}/a.lp
include b.lp,c.lp
//include all
include .
include ${CUR_DIR}/.
ABC=xxx,prop

conf{
    public A{
    B{
      C{
        }
      D{
        }
    }
    }
    sub{
    }
    stu: A.B.C, A.B.D{
        name = abc
        age = 16
        classes = [${math}, eng, sport],
        kv{
            a = 1
            b = 2
            c = 3
        }
    }
}
 */
/*unitTest:
1, only props.
2, include a class-like.
3, include extend.
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
    String name;
    String dir;
    std::set<String> includes;
    std::set<String> supers;
    std::map<String, String> body;
    std::map<String, ConfigItemHolder> children;
    std::map<String, ConfigItemHolder> brothers;

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
    static UPConfigItem make(CString buffer, CString curDir, ConfigItemCache* cache){
        auto item = make();
        if(item->loadFromBuffer(buffer, curDir, cache)){
            return item;
        }
        return nullptr;
    }
    static ConfigItemHolder newDefaultHolder(ConfigItem* item){
        return ConfigItemHolder(item, [](ConfigItem* it){
            delete it;
        });
    }
    bool isPublic()const {return (flags & kFlag_PUBLIC) == kFlag_PUBLIC;}
    void putChild(CString key, UPConfigItem item){
        children[key] = newDefaultHolder(item.release());
    }
    void putChild(CString key, const ConfigItemHolder& ch){
        children[key] = std::move(ch);
    }
    bool loadFromBuffer(CString buffer, CString curDir, ConfigItemCache* cache);
    //return error msg.
    String resolve(ConfigItemCache* cache, const Map& prop);
    String resolve(ConfigItemCache* cache, Map* prop);
    String resolve(ConfigItemCache* cache, IConfigResolver* resolver);

    UPConfigItem copy();
    void dump(int indent = 0);
    bool addBrother(const ConfigItemHolder& ch){
        auto name = ch.ci->name;
        return brothers.emplace(name, std::move(ch)).second;
    }
    void mergeForSuper(ConfigItem* ci);
    //==================
    ConfigItemHolder resolveInclude(String, CString, String&) override{
        return ConfigItemHolder();
    }
    ConfigItemHolder resolveSuper(String, CString, String&) override;
    String resolveValue(CString name, String& errorMsg) override;
};

class SuperConfig: public IKeyValue
{
public:
    SuperConfig(Map* env = nullptr):m_env(env){}

    bool loadFromBuffer(CString buffer, CString curDir);
    bool loadFromFile(CString file);

    String getString(CString key, CString def = "") override;
    void print(CString prefix) override;

    void dump(){m_item.dump();}

private:
    Map* m_env;
    ConfigItem m_item;
    ConfigItemCache m_cache;
};

}
