#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include <functional>

namespace h7_task {

template<typename T>
using List = std::vector<T>;

using String = std::string;
using CString = const std::string&;

struct Task{
    String tag;
    std::function<void()> func;
};

class TaskFlow{
public:
    using ID = size_t;

    void start(){

    }

private:
    void run0(){
        std::set<ID> doneIds;
        while (true) {
            //for(auto& [k,v]: )
        }
    }
    bool isAllDepFinished(ID id, const std::set<ID>& doneIds){
        for(auto& it : m_depMap[id]){
            if(doneIds.find(it) == doneIds.end()){
                return false;
            }
        }
        return true;
    }

private:
    std::map<ID,Task> m_taskMap;
    std::unordered_map<ID,std::set<ID>> m_depMap;
};
}
