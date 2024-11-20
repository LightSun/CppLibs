#include "core/utils/SuperConfig.h"
#include "core/utils/string_utils.hpp"
#include "core/utils/StringBuffer.h"

using namespace h7;

#define DEF_INC_KEY "$(INCLUDE)"

namespace h7 {

bool IConfigResolver::getRealValue(String& val, IConfigResolver* reso, String& errorMsg){
    if(!val.empty()){
        String _key;
        int s1, s2;
        String newVal;
        while (true) {
            s1 = val.find("${");
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
struct WrappedResolver: public IConfigResolver{

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
    String resolves(ConfigItem* parItem){
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
                auto ci = it->second.get();
                errMsg = this->resolves(ci);
                if(!errMsg.empty()){
                    return errMsg;
                }
            }
        }
        return "";
    }
    String resolveValues(ConfigItem* item){
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
                auto ci = it->second.get();
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

struct MultiLineItemParser{
    struct ReadItem{
        String str;
        int id_left;
        int id_right;
        ReadItem(String str, int id_l, int id_r):
            str(str), id_left(id_l), id_right(id_r){
        }
    };
    struct PairItem{
        String preStr;
        String body;
    };
    std::vector<ReadItem> lines;
    int leftCount {0};
    int rightCount {0};

    void addLine(CString str){
        int id1 = str.find("{");
        int id2 = str.rfind("}");
        addItem(str, id1, id2);
    }
    void addItem(CString str, int id_l, int id_r){
        lines.emplace_back(str, id_l, id_r);
        if(id_l >= 0){
            leftCount ++;
        }
        if(id_r >= 0){
            rightCount ++;
        }
    }
    bool isEnd()const{
        return leftCount > 0 && leftCount == rightCount;
    }
    bool parse(ConfigItem* parent){
        PairItem p;
        getOutPair(p);
        //
        String key;
        String preStr;
        auto id_m = p.preStr.find(":");
        if(id_m > 0){
            preStr = p.preStr.substr(0, id_m);
            auto list = h7::utils::split(",", p.preStr.substr(id_m + 1));
            for(auto& su : list){
                h7::utils::trim(su);
            }
            for(auto& su : list){
                parent->supers.insert(su);
            }
        }else{
            preStr = p.preStr;
        }
        h7::utils::trim(preStr);
        auto l = h7::utils::split("\\s+", preStr);
        int flags = 0;
        if(l.size() > 1){
            if(l[0] == "public"){
                flags = ConfigItem::kFlag_PUBLIC;
                key = l[1];
            }else{
                MED_ASSERT_X(false, "unrec str: " << l[0]);
            }
        }else if(l.size() == 1){
            key = l[0];
        }else{
            MED_ASSERT_X(false, "wrong state of config error.");
        }
        auto ci = ConfigItem::make(p.body, parent->dir);
        ci->flags = flags;
        parent->putChild(key, std::move(ci));
        return true;
    }
private:
    void getOutPair(PairItem& p){
        int id_l_first = -1;
        int id_r_last = -1;
        String preStr;
        String body;
        for(int i = 0 ; i < (int)lines.size() ; ++i){
            auto& ri = lines[i];
            if(ri.id_left >= 0){
                preStr.append(ri.str.substr(0, ri.id_left));
                body = ri.str.substr(ri.id_left + 1);
                h7::utils::trim(body);
                id_l_first = i;
                break;
            }else{
                preStr += ri.str;
            }
        }
        for(int i = lines.size() - 1 ; i >=0  ; --i){
            auto& ri = lines[i];
            if(ri.id_right >= 0){
                id_r_last = i;
                break;
            }
        }
        MED_ASSERT_X(id_l_first >=0 && id_r_last >= 0, "getOutPair failed.");
        for(int i = id_l_first + 1 ; i < id_r_last ; ++i){
            auto& ri = lines[i];
            //space
            h7::utils::trim(ri.str);
            body.append(ri.str);
        }
        auto s = lines[id_r_last].str.substr(0, lines[id_r_last].id_right);
        h7::utils::trim(s);
        body.append(s);
        //
        p.preStr = preStr;
        p.body = body;
    }
};

}

UPConfigItem ConfigItem::copy(){
    auto item = make();
    item->includes = includes;
    item->supers = supers;
    item->dir = dir;
    item->body = body;
    item->flags = flags;
    auto it = children.begin();
    for(; it != children.end(); ++it){
        auto next = it->second->copy();
        item->putChild(it->first, std::move(next));
    }
    return item;
}
void ConfigItem::loadFromBuffer(CString buffer, CString curDir){
    this->dir = curDir;
    h7::StringBuffer sb(buffer);
    String str;
    int index;

    while (sb.readLine(str)) {
        h7::utils::trim(str);
        if(str.empty()){
            continue;
        }
        //anno
        if(str.c_str()[0] == '#' || h7::utils::startsWith(str, "//")){
           continue;
        }
        index = str.find("=");
        if(index >= 0){
            String _s = str.substr(0, index);
            h7::utils::trim(_s);
            String val;
            if(index != (int)str.length() - 1){
                val = str.substr(index + 1, str.length() - index - 1);
            }
            h7::utils::trim(val);
            if(_s == DEF_INC_KEY){
                if(!val.empty()){
                    includes.insert(val);
                }
            }else{
                body[_s] = val;
                //if(val.data()[0] == '[' && val.data()[val.length()-1] == ']'){
            }
        }else{
            int idx1_l = str.find("{");
            int idx1_r = str.rfind("}");
            if(idx1_l > 0){
                MultiLineItemParser mp;
                mp.addItem(str, idx1_l, idx1_r);
                String str2;
                while (!mp.isEnd() && sb.readLine(str2)) {
                    mp.addLine(str2);
                }
                MED_ASSERT_X(mp.isEnd(), "[ERROR] unexpect end if line: " << str2);
                mp.parse(this);
            }else{
                MED_ASSERT_X(false, "[ERROR] wrong line: " << str);
            }
        }
    }
}
//return error msg.
String ConfigItem::resolve(IConfigResolver* reso){
    //1, resolve include.
    //2, resolve super.
    //3, resolve variable
    WrappedResolver wreso(reso);
    String errMsg = wreso.resolves(this);
    if(!errMsg.empty()){
        return errMsg;
    }
    //
    return wreso.resolveValues(this);
}
void ConfigItem::mergeForInclude(ConfigItem* ci){
    mergeForSuper(ci);
}
void ConfigItem::mergeForSuper(ConfigItem* ci){
    //ci->includes
    includes.merge(ci->includes);
    //body
    Map newBody = ci->body;
    for(auto it = body.begin(); it != body.end() ; ++it){
        newBody[it->first] = it->second;
    }
    this->body = newBody;
    //children
    for(auto it = ci->children.begin(); it != ci->children.end() ; ++it){
        auto ret = children.emplace(it->first, it->second);
        if(!ret.second){
            MED_ASSERT_X(false, "redfined: " << it->first);
        }
    }
    //super.
    this->supers.merge(ci->supers);
}
String ConfigItem::resolveValue(CString name, String& errorMsg){
    int idxOfDot = name.find(".");
    if(idxOfDot >= 0){
        String sn1 = name.substr(0, idxOfDot);
        auto it = children.find(sn1);
        if(it != children.end()){
            String sn2 = name.substr(idxOfDot+ 1);
            return it->second->resolveValue(sn2, errorMsg);
        }
        return "";
    }
    auto it = body.find(name);
    if(it != body.end()){
        return it->second;
    }
    return "";
}
//==================================================
bool SuperConfig::loadFromFile(CString file){

}
bool SuperConfig::loadFromBuffer(CString buffer, CString curDir){
    m_item->loadFromBuffer(buffer, curDir);
    //m_item->resolve()
}
String SuperConfig::getString(CString key, CString def){

}
