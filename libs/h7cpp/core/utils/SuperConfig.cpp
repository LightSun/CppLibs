#include <algorithm>
#include "core/utils/SuperConfig.h"
#include "core/utils/string_utils.hpp"
#include "core/utils/StringBuffer.h"
#include "core/utils/FileUtils.h"
#include "core/common/c_common.h"

using namespace h7;


namespace h7 {

struct MultiLineItemParser{
    struct ReadItem{
        String str;
        int id_left;
        int id_right;
        ReadItem(String str, int id_l, int id_r):
            str(str), id_left(id_l), id_right(id_r){
        }
        String subStrLeft(){
            return str.substr(0, id_left);
        }
        String subStrRight(){
            return str.substr(0, id_right);
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
    bool parse(ConfigItem* parent, ConfigItemCache* cache){

        PairItem pi;
        getBody(parent, pi);

        int idx = pi.preStr.find(":");
        if(idx > 0){
            parent->name = pi.preStr.substr(0, idx);
            String ss = pi.preStr.substr(idx + 1);
            auto sus = h7::utils::split2(",", ss);
            for(auto& s: sus){
                h7::utils::trim(s);
                parent->supers.insert(s);
            }
        }else{
            parent->name = pi.preStr;
        }
        h7::utils::trim(parent->name);
        //parent->flags = flags;

        //may have multi.  'B{C{}D{}}' -> 'C{}D{}'
        //find all '{' '}' and all "${"
        std::vector<IdxPair> bigQuotePairs;
        if(!findBigQuotePairs(pi.body, bigQuotePairs)){
            return false;
        }
        for(auto& pair: bigQuotePairs){
            // 1A012B{}C -> 1,7
            auto nbody = pi.body.substr(pair.startIdx, pair.rightIdx - pair.startIdx + 1);
            auto ci = ConfigItem::make(nbody, parent->dir, cache);
            if(!ci){
                return false;
            }
            MED_ASSERT(!ci->name.empty());
            String key = ci->name;
            parent->putChild(key, std::move(ci));
        }
        return true;
    }
    static bool hasNonVarRefBigQuote(CString body, bool incRight = false){
        const int body_len = body.size();
        for(int i = 0 ; i < body_len; ++i){
            auto& ch = body.data()[i];
            if(ch == '{'){
                if(i > 0 && body.data()[i-1] == '$'){
                    int id2 = body.find("}", i + 1);
                    if(id2 < 0){
                        fprintf(stderr, "'{' and '}' must be pairs.\n");
                        return false;
                    }
                    i = id2;
                    continue;
                }
                return true;
            }
            if(incRight && ch == '}'){
                return true;
            }
        }
        return false;
    }
private:

    bool findBigQuotePairs(CString body, std::vector<IdxPair>& ps){
        std::vector<int> beginIdxes;
        std::vector<int> endIdxes;
        int lastRightId = -1;
        const int body_len = body.size();
        for(int i = 0 ; i < body_len; ++i){
            auto& ch = body.data()[i];
            if(ch == '{'){
                if(i > 0 && body.data()[i-1] == '$'){
                    int id2 = body.find("}", i + 1);
                    if(id2 < 0){
                        fprintf(stderr, "'{' and '}' must be pairs.\n");
                        return false;
                    }
                    i = id2;
                    continue;
                }
                beginIdxes.push_back(i);
            }else if(ch == '}'){
                endIdxes.push_back(i);

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
        if(!beginIdxes.empty() || !endIdxes.empty()){
            fprintf(stderr, "'{' and '}' must be pairs.\n");
            return false;
        }
        return true;
    }
    void getBody(ConfigItem* ci, PairItem& p){
        MED_ASSERT(lines[0].id_left > 0);
        p.preStr = lines[0].subStrLeft();
        const int size = lines.size();
        int bodyStartIdx = -1;
        for(int i = 1 ; i < size ; ++i){
            auto& line = lines[i];
            //auto& str = line.str;
            if(!hasNonVarRefBigQuote(line.str, true)){
                //prop line
                int index = line.str.find("=");
                if(index < 0){
                    index = line.str.find(":");
                }
                if(index >= 0){
                    String _s = line.str.substr(0, index);
                    h7::utils::trim(_s);
                    String val;
                    if(index != (int)line.str.length() - 1){
                        val = line.str.substr(index + 1, line.str.length() - index - 1);
                    }
                    h7::utils::trim(val);
                    ci->body[_s] = val;
                }
            }else{
                bodyStartIdx = i;
                break;
            }
        }
        if(bodyStartIdx > 0){
            for(int i = bodyStartIdx ; i < size - 1 ; ++i){
                p.body.append(lines[i].str + NEW_LINE);
            }
        }
    }
};

}

bool ConfigItem::loadFromBuffer(CString _buffer, CString curDir, ConfigItemCache* cache){
    auto buffer = h7::utils::replace("\\t", "", _buffer);
    this->dir = curDir;
    h7::StringBuffer sb(buffer);
    String str;

    while (sb.readLine(str)) {
        h7::utils::trim(str);
        if(str.empty()){
            continue;
        }
        printf("line: >> '%s'\n", str.data());
        //anno
        if(str.c_str()[0] == '#' || h7::utils::startsWith(str, "//")){
            continue;
        }
        if(MultiLineItemParser::hasNonVarRefBigQuote(str)){
            MultiLineItemParser mp;
            String str2;
            mp.addLine(str);
            while (!mp.isEnd() && sb.readLine(str2)) {
                h7::utils::trim(str2);
                if(str2.empty()){
                    continue;
                }
                //ignore anno line .
                if(str2.c_str()[0] == '#' || h7::utils::startsWith(str2, "//")){
                    continue;
                }
                mp.addLine(str2);
            }
            if(!mp.isEnd()){
                fprintf(stderr, "loadFromBuffer >> [ERROR] unexpect end if line = %s.\n", str2.data());
                return false;
            }
            if(!mp.parse(this, cache)){
                fprintf(stderr, "loadFromBuffer >> [MultiLine-Parse-ERROR] wrong line: %s.\n", str.data());
                return false;
            }
            continue;
        }
        if(utils::startsWith(str, "include")){
            String name = str.substr(7);
            h7::utils::trim(name);
            this->includes.insert(name);
        }
    }
    return true;
}

//return error msg.
String ConfigItem::resolve(ConfigItemCache* cache, IConfigResolver* reso){
    //1, resolve include.
    //2, resolve super.
    //3, resolve variable
    auto wreso = IConfigResolver::newWrappedConfigResolver(cache, reso);
    String errMsg = wreso->resolves(this);
    if(!errMsg.empty()){
        return errMsg;
    }
    //
    return wreso->resolveValues(this);
}
String ConfigItem::resolve(ConfigItemCache* cache, const Map& prop){
    auto recr = IConfigResolver::newRuntimeConfigResolver(cache, this, prop);
    return resolve(cache, recr.get());
}
String ConfigItem::resolve(ConfigItemCache* cache, Map* prop){
    auto recr = IConfigResolver::newRuntimeConfigResolver(cache, this, prop);
    return resolve(cache, recr.get());
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
    if(indent > 0){
        for(auto& inc : includes){
            printf("%s%s\n", pre.data(), inc.data());
        }
        for(auto& inc : supers){
            printf("%s%s\n", pre.data(), inc.data());
        }
        for(auto& inc : body){
            printf("%s%s: %s\n", pre.data(), inc.first.data(), inc.second.data());
        }
        for(auto& inc : children){
            printf("%s%s{\n", pre.data(), inc.first.data());
            inc.second.ci->dump(indent + inc.first.length() + 1);
            printf("%s}\n", pre.data());
        }
        for(auto& inc : brothers){
            printf("%s%s{\n", pre.data(), inc.first.data());
            inc.second.ci->dump(indent + inc.first.length() + 1);
            printf("%s}\n", pre.data());
        }
    }else{
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
            printf("%s%s: %s\n", pre.data(), inc.first.data(), inc.second.data());
        }
        printf("---- Body end ------\n\n");

        printf("---- Children start ------\n");
        for(auto& inc : children){
            printf("%s%s{\n", pre.data(), inc.first.data());
            inc.second.ci->dump(indent + inc.first.length() + 1);
            printf("%s}\n", pre.data());
        }
        printf("---- Children end ------\n\n");

        printf("---- Brothers start ------\n");
        for(auto& inc : brothers){
            printf("%s%s{\n", pre.data(), inc.first.data());
            inc.second.ci->dump(indent + inc.first.length() + 1);
            printf("%s}\n", pre.data());
        }
        printf("---- Brothers end ------\n\n");
    }
}
//---------------
ConfigItemHolder ConfigItem::resolveSuper(String curDir, CString name, String& errMsg){
    int idxOfDot = name.find(".");
    if(idxOfDot >= 0){
        //find from children
        String sn1 = name.substr(0, idxOfDot);
        String sn2 = name.substr(idxOfDot+ 1);
        {
            auto it = children.find(sn1);
            if(it != children.end()){
                return it->second.ci->resolveSuper(curDir, sn2, errMsg);
            }
        }
        //find from brother
        {
            auto it = brothers.find(sn1);
            if(it != brothers.end()){
                return it->second.ci->resolveSuper(curDir, sn2, errMsg);
            }
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
        String sn2 = name.substr(idxOfDot+ 1);
        {
            auto it = children.find(sn1);
            if(it != children.end()){
                return it->second.ci->resolveValue(sn2, errorMsg);
            }
        }
        {
            auto it = brothers.find(sn1);
            if(it != brothers.end()){
                return it->second.ci->resolveValue(sn2, errorMsg);
            }
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
    if(!m_item.loadFromBuffer(buffer, curDir, &m_cache)){
        return false;
    }
    auto msg = m_item.resolve(&m_cache, m_env);
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
void SuperConfig::print(CString prefix){
     printf("========== %s start ==============%s", prefix.data(), CMD_LINE);
     dump();
     printf("========== %s end ==============%s", prefix.data(), CMD_LINE);
}
