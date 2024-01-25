#pragma once

#include "table/Column.h"
#include "utils/string_utils.hpp"

class Tab_Group: public SkRefCnt{
public:
    using GroupS = h7::GroupS;
    using ListUI = h7::ListUI;
    using ListF = h7::ListF;
    String name;
    sk_sp<GroupS> data;
    sk_sp<ListUI> tempList;
    //uint32 hash;
    //h7::IColumn<sk_sp<Tab_Group>> subs;
    Tab_Group();
    Tab_Group(String name, sk_sp<GroupS> g):name(name), data(g){
    }
    Tab_Group(const Tab_Group& src){
        this->data = src.data;
        this->name = src.name;
        this->tempList = src.tempList;
    }
    void cvtInt(int idx){
        tempList = data->map<uint32>([idx](sk_sp<h7::ListS>& l,int){
            auto& str = l->get(idx);
            return h7::utils::getInt(str, 0);
        });
    }
    Tab_Group& operator=(Tab_Group& g1){
        this->name = g1.name;
        this->data = g1.data;
        this->tempList = g1.tempList;
        return *this;
    }
    Tab_Group& operator=(const Tab_Group& g1){
        this->name = g1.name;
        this->data = g1.data;
        this->tempList = g1.tempList;
        return *this;
    }
};
inline bool operator ==(const Tab_Group& t1,const Tab_Group& t2){
    if(t1.name != t2.name){
        return false;
    }
    return t1.data == t2.data;
}
inline bool operator !=(const Tab_Group& t1,const Tab_Group& t2){
    if(t1.name != t2.name){
        return true;
    }
    return t1.data != t2.data;
}
inline bool operator <(const Tab_Group& t1,const Tab_Group& t2){
    if(t1.name < t2.name){
        return true;
    }
    return t1.data < t2.data;
}
inline bool operator >(const Tab_Group& t1,const Tab_Group& t2){
    if(t1.name > t2.name){
        return true;
    }
    return t1.data > t2.data;
}
inline void swap(Tab_Group& t1,Tab_Group& t2){
    std::swap(t1.name, t2.name);
    {
    auto d1 = t1.data;
    t1.data = t2.data;
    t2.data = d1;
    }
    {
    auto l1 = t1.tempList;
    t1.tempList = t2.tempList;
    t2.tempList = l1;
    }
}
inline bool operator ==(const sk_sp<Tab_Group>& t1,const sk_sp<Tab_Group>& t2){
    if(t1->name != t2->name){
        return false;
    }
    return t1->data == t2->data;
}

inline bool operator !=(const sk_sp<Tab_Group>& t1,const sk_sp<Tab_Group>& t2){
    if(t1->name != t2->name){
        return true;
    }
    return t1->data != t2->data;
}
