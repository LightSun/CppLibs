#include <algorithm>
#include "core/utils/SuperConfig.h"
#include "core/utils/string_utils.hpp"
#include "core/utils/StringBuffer.h"
#include "core/utils/FileUtils.h"

using namespace h7;

#define DEF_INC_KEY "$(INCLUDE)"

namespace h7 {

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
    struct IdxPair{
        int startIdx;
        int leftIdx;
        int rightIdx;
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

            int start_pos = id_l + 1;
            int id = -1;
            while ( (id = str.find("{", start_pos)) >= 0) {
                leftCount ++;
                start_pos = id + 1;
            }
        }
        if(id_r >= 0){
            int start_pos = 0;
            int id = -1;
            while ( (id = str.find("}", start_pos)) >= 0) {
                start_pos = id + 1;
                rightCount ++;
            }
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
            auto supers = h7::utils::split(",", p.preStr.substr(id_m + 1));
            for(auto& su : supers){
                h7::utils::trim(su);
            }
            for(auto& su : supers){
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
        parent->flags = flags;
        //may have multi.  'B{C{}D{}}' -> 'C{}D{}'
        //find all "{" and all "${"
        std::vector<IdxPair> bigQuotePairs;
        if(!findBigQuotePairs(p.body, bigQuotePairs)){
            return false;
        }
        for(auto& pair: bigQuotePairs){
            // 1A012B{}C -> 1,7
            auto nbody = p.body.substr(pair.startIdx, pair.rightIdx - pair.startIdx + 1);
            auto ci = ConfigItem::make(nbody, parent->dir);
            parent->putChild(key, std::move(ci));
        }
        return true;
    }
private:
    bool findBigQuotePairs(CString body, std::vector<IdxPair>& ps){
        int offset = 0;
        //bool doubleQuoteClosed = true;
        std::vector<int> beginIdxes;
        std::vector<int> endIdxes;
        int lastRightId = -1;
        while (true) {
            //int idx_dq = body.find("\"");
            int idx = body.find("{", offset);
            if(idx < 0){
                break;
            }
            beginIdxes.push_back(idx);
            if(body.data()[idx - 1] == '$'){
                //var-ref
                int id2 = body.find("}", idx + 1);
                offset = id2 + 1;
            }else{
                //non var-ref
                int idx2 = body.find("}", idx + 1);
                if(idx2 < 0){
                    fprintf(stderr, "Parse error: while parse '%s'\n", body.data());
                    return false;
                }
                offset = idx2 + 1;
                endIdxes.push_back(idx2);
                //find inside.
                if(endIdxes.size() == beginIdxes.size()){
                    //pair of first begin and last of end.
                    IdxPair pair;
                    pair.startIdx = lastRightId >= 0 ? lastRightId + 1 : 0;
                    pair.leftIdx = beginIdxes[0];
                    pair.rightIdx = endIdxes[endIdxes.size()-1];
                    ps.push_back(pair);
                    //
                    lastRightId = pair.rightIdx;
                    beginIdxes.clear();
                    endIdxes.clear();
                }
            }
        }
        return true;
    }
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
                //h7::utils::trim(body);
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
            //h7::utils::trim(ri.str);
            body.append(ri.str);
        }
        if(id_r_last > id_l_first){
            auto s = lines[id_r_last].str.substr(0, lines[id_r_last].id_right);
            //h7::utils::trim(s);
            body.append(s);
        }else if(id_r_last == id_l_first){
            auto& ri = lines[id_r_last];
            //012{334}6
            //3,7
            body = ri.str.substr(ri.id_left + 1, ri.id_right - ri.id_left - 1);
        }
        //
        p.preStr = preStr;
        p.body = body;
    }
};

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
                if(!mp.parse(this)){
                    break;
                }
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
    auto wreso = IConfigResolver::newWrappedConfigResolver(reso);
    String errMsg = wreso->resolves(this);
    if(!errMsg.empty()){
        return errMsg;
    }
    //
    return wreso->resolveValues(this);
}
String ConfigItem::resolve(const Map& prop){
    auto recr = IConfigResolver::newRuntimeConfigResolver(this, prop);
    return resolve(recr.get());
}
String ConfigItem::resolve(Map* prop){
    auto recr = IConfigResolver::newRuntimeConfigResolver(this, prop);
    return resolve(recr.get());
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
        auto ret = children.emplace(it->first, it->second.ref());
        if(!ret.second){
            MED_ASSERT_X(false, "redfined: " << it->first);
        }
    }
    //super.
    this->supers.merge(ci->supers);
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
        auto next = it->second.ci->copy();
        item->putChild(it->first, std::move(next));
    }
    return item;
}
void ConfigItem::dump(int indent){
    String pre;
    for(int i = 0 ; i < indent ; ++i){
        pre += " ";
    }
    printf("---- Includes start ------\n");
    for(auto& inc : includes){
        printf("%s%s\n", pre.data(), inc.data());
    }
    printf("---- Includes end ------\n\n");

    printf("---- Supers start ------\n");
    for(auto& inc : supers){
        printf("%s%s\n", pre.data(), inc.data());
    }
    printf("---- Supers end ------\n\n");

    printf("---- Body start ------\n");
    for(auto& inc : body){
        printf("%s%s = %s\n", pre.data(), inc.first.data(), inc.second.data());
    }
    printf("---- Body end ------\n\n");

    printf("---- Children start ------\n");
    for(auto& inc : children){
        printf("%s%s{\n", pre.data(), inc.first.data());
        inc.second.ci->dump(indent + inc.first.length() + 1);
        printf("%s  }\n", pre.data());
    }
    printf("---- Children end ------\n\n");
}
//---------------
ConfigItemHolder ConfigItem::resolveSuper(String curDir, CString name, String& errMsg){
    int idxOfDot = name.find(".");
    if(idxOfDot >= 0){
        String sn1 = name.substr(0, idxOfDot);
        auto it = children.find(sn1);
        if(it != children.end()){
            String sn2 = name.substr(idxOfDot+ 1);
            return it->second.ci->resolveSuper(curDir, sn2, errMsg);
        }
        return ConfigItemHolder();
    }
    auto it = children.find(name);
    if(it != children.end()){
        return ConfigItemHolder(it->second.ci);
    }
    return ConfigItemHolder();
}
String ConfigItem::resolveValue(CString name, String& errorMsg){
    int idxOfDot = name.find(".");
    if(idxOfDot >= 0){
        String sn1 = name.substr(0, idxOfDot);
        auto it = children.find(sn1);
        if(it != children.end()){
            String sn2 = name.substr(idxOfDot+ 1);
            return it->second.ci->resolveValue(sn2, errorMsg);
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
    String path = file;
    if(FileUtils::isRelativePath(path)){
        path = FileUtils::getCurrentDir() + "/" + path;
    }
    if(!FileUtils::isFileExists(path)){
        return false;
    }
    auto dir = FileUtils::getFileDir(path);
    return loadFromBuffer(FileUtils::getFileContent(path), dir);
}
bool SuperConfig::loadFromBuffer(CString buffer, CString curDir){
    m_item.loadFromBuffer(buffer, curDir);
    auto msg = m_item.resolve(m_env);
    if(!msg.empty()){
        fprintf(stderr, "%s\n", msg.data());
        return false;
    }
    return true;
}
String SuperConfig::getString(CString key, CString def){
    String eMsg;
    auto val = m_item.resolveValue(key, eMsg);
    if(val.empty()){
        val = def;
    }
    return val;
}
