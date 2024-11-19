#pragma once

#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

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

struct IConfigResolver{
    virtual ConfigItem* resolveInclude(String curDir, CString name, String& errorMsg) = 0;
    virtual ConfigItem* resolveSuper(String curDir, CString name, String& errorMsg) = 0;
    virtual String resolveValue(CString name, String& errorMsg) = 0;

    static bool getRealValue(String& val, IConfigResolver* reso, String& errorMsg);
};
using UPConfigItem = std::unique_ptr<ConfigItem>;
struct ConfigItem{
    enum{
        kFlag_PUBLIC = 0x0001
    };
    std::vector<String> includes;
    std::vector<String> supers;
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
    String resolve(IConfigResolver* resolver);
    UPConfigItem copy();
    void mergeForInclude(ConfigItem* ci);
    void mergeForSuper(ConfigItem* ci);
};

class SuperConfig
{
public:
    SuperConfig();

    void loadFromFile(CString file);
    void loadFromBuffer(CString buffer, CString curDir);

private:
};

}
