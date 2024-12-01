#pragma once

#include <map>
#include "core/src/Value.hpp"

namespace h7 {
class Properties;
}

namespace med_qa {

using String = std::string;
using CString = const String&;

typedef struct _PropCtx _PropCtx;

class Prop
{
public:
    Prop();
    ~Prop();

    void prints();
    void load(int argc, const char* argv[]);
    void load(CString propFile);
    h7::Properties* getRawProp();
    void copyFrom(std::map<String,String>& src);

    void putString(CString key, CString val);
    String getString(CString key, CString def = "");
    h7::Value getValue(CString key, CString def = ""){
        return h7::Value(getString(key, def));
    }
private:
    _PropCtx* m_ctx {nullptr};
};

}

