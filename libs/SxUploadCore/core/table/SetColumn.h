#ifndef SETCOLUMN_H
#define SETCOLUMN_H

#include <vector>
#include <unordered_set>
#include "common/SkRefCnt.h"

namespace h7 {

template<typename T>
class SetColumn : public SkRefCnt{
public:

    std::unordered_set<T> set;

    SetColumn(){
    }
    SetColumn(int size){
        set.reserve(size);
    }
    SetColumn(const SetColumn<T>& column):set(column.set){
    }
    SetColumn(SetColumn<T>&& column):set(column.set){
    }
    SetColumn(const std::unordered_set<T>& set):set(set){
    }
    SetColumn(const std::vector<T>& vec){
        addAll(vec);
    }
    auto begin(){
        return set.begin();
    }
    auto end(){
        return set.end();
    }

    inline bool contains(const T& t)const{
        return set.find(t) != set.end();
    }
    inline bool contains(const T& t){
        return set.find(t) != set.end();
    }
    inline bool contains(T& t){
        return set.find(t) != set.end();
    }
    inline bool containsAll(const SetColumn<T>& oth){
        if(this->size() < oth.size()){
            return false;
        }
        auto it = oth.set.begin();
        auto end = oth.set.end();
        for( ; it != end ; it++){
            if(!contains(*it)){
                return false;
            }
        }
        return true;
    }
    inline bool add(const T& t){
        set.insert(t);
        return true;
    }
    bool addAll(SetColumn<T>& oth){
        std::copy(oth.begin(), oth.end(), std::inserter(set, set.end()));
//        auto it = oth.set.begin();
//        auto end = oth.set.end();
//        for( ; it != end ; it++){
//            set.insert(*it);
//        }
        return true;
    }
    bool addAll(const SetColumn<T>& oth){
        std::copy(oth.begin(), oth.end(), std::inserter(set, set.end()));
        return true;
    }
    bool addAll(const std::vector<T>& oth){
        std::copy(oth.begin(), oth.end(), std::inserter(set, set.end()));
//        auto it = oth.begin();
//        auto end = oth.end();
//        for( ; it != end ; it++){
//            set.insert(*it);
//        }
        return true;
    }
    bool addAll(std::vector<T>& oth){
        std::copy(oth.begin(), oth.end(), std::inserter(set, set.end()));
        return true;
    }
    inline int size()const{
        return set.size();
    }
    void prepare(int size){
        set.reserve(size);
    }
    std::vector<T> toList(){
        return {set.begin(), set.end()};
    }

    SetColumn<T>& operator=(SetColumn<T>&& oth){
        this->set = oth.set;
        return *this;
    };
    SetColumn<T>& operator=(const SetColumn<T>& oth){
        this->set = oth.set;
        return *this;
    };
};

typedef SetColumn<std::string> SetS;
typedef SetColumn<int> SetI;

}

#endif // SETCOLUMN_H
