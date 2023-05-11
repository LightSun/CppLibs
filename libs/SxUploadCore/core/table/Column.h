#ifndef COLUMN_H
#define COLUMN_H

#include <string>
#include <vector>
#include <initializer_list>
#include <set>
#include <unordered_set>
#include <limits.h>
#include <sstream>
#include <random>

#include "common/SkRefCnt.h"
#include "common/common.h"
#include "utils/convert.hpp"

namespace h7 {

template <typename T>
class IColumn;

typedef IColumn<std::string> ListS;
typedef IColumn<bool> ListB;
typedef IColumn<char> ListCh;
typedef IColumn<unsigned char> ListUCh;
typedef IColumn<short> ListSh;
typedef IColumn<unsigned short> ListUSh;
typedef IColumn<int> ListI;
typedef IColumn<unsigned int> ListUI;
typedef IColumn<long long> ListL;
typedef IColumn<unsigned long long> ListUL;
typedef IColumn<float> ListF;
typedef IColumn<double> ListD;

typedef IColumn<sk_sp<ListS>> GroupS;
typedef IColumn<sk_sp<ListI>> MatrixI;

template <typename T>
class IColumn : public SkRefCnt{
public:
    std::vector<T> list;

    IColumn(){
        prepareSize(10);
    }
    IColumn(std::initializer_list<T> _list):list(_list){
    }
    IColumn(const std::vector<T>& _list):list(_list){
    }
    IColumn(const IColumn<T>& _list): list(_list.list){

    }
    IColumn(int initCount){
        list.reserve(initCount);
    }
    IColumn(int count, bool fixed){
        if (fixed) {
            list.resize(count);
        }else{
            list.reserve(count);
        }
    }
    IColumn(IColumn<T>&& c): list(c.list){

    };

    inline std::string getString(int index, const std::string& defVal) const{
        return toStringImpl<T>(list[index], defVal);
    }
    inline int getInt(int index, int defVal){
        int val = getIntImpl<T>(list[index]);
        return val != INT_MIN ? val : defVal;
    }
    inline float getFloat(int index, float defVal){
        float val = getFloatImpl<T>(list[index]);
        return !std::isnan(val) ? val : defVal;
    }

    inline T& get(int index){
        MED_ASSERT(index < (int)list.size());
        return list[index];
    }

    inline T const_get(int index)const{
        MED_ASSERT(index < (int)list.size());
        return list[index];
    }
    inline int size() const{
        return list.size();
    }
    inline bool isEmpty() const{
        return list.size() == 0;
    }
    inline bool isNotEmpty() const{
        return list.size() > 0;
    }
    inline void clear(){
        list.clear();
    }
    inline void remove(const T& t, bool once = true){
        size_t size = list.size();
        for (int i = size - 1; i >=0 ; i--) {
            if (list[i] == t) {
                removeAt0(i);
                if(once) break;
            }
        }
    }
    inline void add(const T& obj){
        list.push_back(obj);
    }
    template<typename... _Args>
    inline void add_cons(_Args&&... __args){
        list.emplace_back(std::forward<_Args>(__args)...);
    }
    inline void add(int index, const T& t){
        MED_ASSERT(index < list.size());
        list.insert(list.begin() + index, t);
    }
    inline void addAll(int index, IColumn<T>* oth){
        MED_ASSERT(index < list.size());
        list.insert(list.begin() + index, oth->list.begin(), oth->list.end());
    }
    inline void addAll(int index, std::initializer_list<T> oth){
        MED_ASSERT(index < list.size());
        list.insert(list.begin() + index, oth.begin(), oth.end());
    }
    inline void addAll(std::initializer_list<T> oth){
        list.insert(list.end(), oth.begin(), oth.end());
    }
    inline void addAll(IColumn<T>* oth){
        list.insert(list.end(), oth->list.begin(), oth->list.end());
    }
    inline void addAll(sk_sp<IColumn<T>> oth){
        list.insert(list.end(), oth->list.begin(), oth->list.end());
    }
    inline void addAll(const IColumn<T>& oth){
        list.insert(list.end(), oth.list.begin(), oth.list.end());
    }
    inline void setAll(std::initializer_list<T> oth){
        list.clear();
        list.insert(list.begin(), oth.begin(), oth.end());
    }
    inline void setAll(const std::vector<T>& oth){
        list.clear();
        list.insert(list.begin(), oth.begin(), oth.end());
    }
    inline void setAll(sk_sp<IColumn<T>> oth){
        list.clear();
        list.insert(list.begin(), oth->list.begin(), oth->list.end());
    }
    inline void setAll(IColumn<T>* oth){
        list.clear();
        list.insert(list.begin(), oth->list.begin(), oth->list.end());
    }
    inline void set0(int index, const T& val){
        MED_ASSERT(index < (int)list.size());
        list[index] = val;
    }
    inline void removeAt0(int index){
        MED_ASSERT(index < (int)list.size());
        list.erase(list.begin() + index);
    }
    inline T removeAt(int index) const{
        MED_ASSERT(index < (int)list.size());
        T t = list[index];
        list.erase(list.begin() + index);
        return t;
    }
    int indexOf(const T& t)const{
        int size = list.size();
        for (int i = 0; i < size; i++) {
            //need overload ==
            if (list[i] == t) {
                return i;
            }
        }
        return -1;
    }
    int indexOf(T& t)const{
        int size = list.size();
        for (int i = 0; i < size; i++) {
            //need overload ==
            if (list[i] == t) {
                return i;
            }
        }
        return -1;
    }
    bool contains(const T& t) const{
        int size = list.size();
        for (int i = 0; i < size; i++) {
            //need overload ==
            if (list[i] == t) {
                return true;
            }
        }
        return false;
    }
    bool contains(T& t) const{
        int size = list.size();
        for (int i = 0; i < size; i++) {
            //need overload ==
            if (list[i] == t) {
                return true;
            }
        }
        return false;
    }
    inline void resize(int size){
        list.resize(size);
    }
    inline void prepareSize(int size){
        list.reserve(size);
    }
    inline void swap(int id1, int id2){
//        T t = list[id1];
//        list[id1] = list[id2];
//        list[id2] = t;
        std::iter_swap(list.begin()+id1, list.begin()+id2);
    }
    sk_sp<IColumn<int>> asInt(int defVal){
        sk_sp<IColumn<int>> sp = sk_make_sp<IColumn<int>>();
        int size = list.size();
        sp->prepareSize(size);
        for (int i = 0; i < size; i++) {
           sp->add(getInt(i, defVal));
        }
        return sp;
    }
    inline sk_sp<IColumn<int>> asInt(){
        return asInt(INT_MIN);
    }
    sk_sp<IColumn<float>> asFloat(float defVal){
        sk_sp<IColumn<float>> sp = sk_make_sp<IColumn<float>>();
        size_t size = list.size();
        sp->prepareSize(size);
        for (size_t i = 0; i < size; i++) {
           sp->add(getFloat(i, defVal));
        }
        return sp;
    }
    inline sk_sp<IColumn<float>> asFloat(){
        return asFloat(NAN);
    }

    T& last(){
       if(size() == 0){
           T* t = nullptr;
           return *t;
       }
       return list[size()-1];
    }

    T max(){
        T t = list[0];
        int size = list.size();
        for (int i = 1; i < size; i++) {
            if(list[i] > t){
                t = list[i];
            }
        }
        return t;
    }

    inline IColumn<T>& sort(std::function<int(T&,T&)> func){
        std::sort<T>(list.begin(), list.end(), [func](T& t1, T& t2){
             return func(t1, t2) < 0;
        });
    }

    sk_sp<IColumn<T>> copy(){
        sk_sp<IColumn<T>> sp = sk_make_sp<IColumn<T>>(list.size());
        sp->list.insert(sp->list.end(), list.begin(), list.end());
        return sp;
    }

    void copyTo(IColumn<T>* col){
        col->prepareSize(list.size());
        col->list.insert(col->list.end(), list.begin(), list.end());
    }

    //start: include,  end: exclude
    sk_sp<IColumn<T>> sub(int start, int end){
        MED_ASSERT(start >=0 && start < size());
        MED_ASSERT(end > start && end <= size());
        sk_sp<IColumn<T>> sp = sk_make_sp<IColumn<T>>();
        sp->prepareSize(end - start);
        sp->list.insert(sp->list.end(), list.begin() + start, list.begin() + end);
        return sp;
    }
    inline int hashCode(int index){
        MED_ASSERT(index >=0 && index < size());
        return hashImpl<T>(list[index]);
    }
    int hashCode()const{
         int result = 1;
         int size = list.size();
         for (int i = 0; i < size; ++i ) {
              result = 31 * result + hashImpl<T>(list[i]);
         }
         return result;
    }
    //conpute rge hash of target index elements.
    int hashCode(ListI* idxes)const{
        MED_ASSERT(idxes != nullptr);
        int result = 1;
        int size = idxes->size();
        for (int i = 0; i < size; ++i ) {
             result = 31 * result + hashImpl<T>(list[idxes->get(i)]);
        }
        return result;
    }
    sk_sp<ListI> toHash(){
        int size = list.size();
        sk_sp<ListI> sp = sk_make_sp<ListI>();
        sp->resize(size);
        for (int i = 0; i < size; i++) {
            sp->set0(i, hashImpl<T>(list[i]));
        }
        return sp;
    }
    sk_sp<ListS> toStrs(){
        int size = list.size();
        sk_sp<ListS> sp = sk_make_sp<ListS>();
        sp->resize(size);
        for (int i = 0; i < size; i++) {
            sp->set0(i, toStringImpl<T>(list[i], ""));
        }
        return sp;
    }
    inline String toString(char seq)const{
        String str(1, seq);
        return toString(str);
    }
    inline String toString() const{
        return toString(",");
    }
    String toString(CString seq)const{
        std::stringstream out;
        size_t size = list.size();
        for (size_t i = 0; i < size; i++) {
            out << toStringImpl<T>(list[i], "");
            if(i != size - 1){
                out << seq;
            }
        }
        return out.str();
    }
    /**
     * @brief include "[" and "]"
     * @return the string
     */
    inline String toStringFullly()const{
        return "[" + toString(", ") + "]";
    }
    //----------------------------------
    inline void toSet(std::set<T>& set){
        std::copy(list.begin(), list.end(), std::inserter(set, set.end()));
    }
    inline void toSet(std::unordered_set<T>& set){
        //set.reserve(list.size());
        std::copy(list.begin(), list.end(), std::inserter(set, set.end()));
    }
    void tileToLength(int expectLen){
        if(list.empty()){
            list.resize(expectLen);
        }else{
            const int rawSize = list.size();
            int diff = expectLen - rawSize;
            int nextIndex = 0;
            for (int i = diff ; i > 0 ; --i){
                add(get(nextIndex));
                if(nextIndex == rawSize - 1){
                    nextIndex = 0;
                }else{
                    nextIndex ++;
                }
            }
        }
    }
    void alignSize(int expectSize, const T& defVal){
        int oldSize = size();
        list.resize(expectSize);
        if(oldSize < expectSize){
            int diff = expectSize - oldSize;
            for (int i = 0 ; i < diff ; ++i){
               set0(i + oldSize, defVal);
            }
        }
    }
    //-----------------------
    sk_sp<IColumn<T>> filter(std::function<bool(T&,int)> func){
        sk_sp<IColumn<T>> ret = sk_make_sp<IColumn<T>>();
        int size = list.size();
        for (int i = 0; i < size; ++i) {
            auto& t = get(i);
            if(func(t, i)){
                ret->add(t);
            }
        }
        return ret;
    }
    template<typename E>
    sk_sp<IColumn<E>> map(std::function<E(T&,int)> func){
        int size = list.size();
        sk_sp<IColumn<E>> ret = sk_make_sp<IColumn<E>>();
        ret->prepareSize(size);
        for (int i = 0; i < size; ++i) {
            ret->add(func(get(i), i));
        }
        return ret;
    }
    inline void travel(std::function<void(T&,int)> func){
        int size = list.size();
        for (int i = 0; i < size; ++i) {
            func(get(i), i);
        }
    }
    template<typename E>
    sk_sp<IColumn<E>> filterMap(std::function<bool(T&,int)> pre,
                          std::function<E(T&,int)> func){
        sk_sp<IColumn<E>> ret = sk_make_sp<IColumn<E>>();
        int size = list.size();
        for (int i = 0; i < size; ++i) {
            auto& t = get(i);
            if(pre && pre(t, i)){
                ret->add(func(t, i));
            }
        }
        return ret;
    }
    template<typename E>
    sk_sp<IColumn<E>> mapFilter(std::function<bool(E&,int)> pre,
                          std::function<E(T&,int)> func){
        sk_sp<IColumn<E>> ret = sk_make_sp<IColumn<E>>();
        int size = list.size();
        for (int i = 0; i < size; ++i) {
            E e = func(get(i), i);
            if(pre && pre(e, i)){
                ret->add(e);
            }
        }
        return ret;
    }
    bool isAll(const T& t){
        int size = list.size();
        for (int i = 0; i < size; ++i) {
           if(list[i] != t){
               return false;
           }
        }
        return true;
    }
    bool isAll(std::function<bool(T&,int)> func){
        int size = list.size();
        for (int i = 0; i < size; ++i) {
           if(!func(get(i), i)){
               return false;
           }
        }
        return true;
    }
    bool isAny(std::function<bool(T&,int)> func){
        int size = list.size();
        for (int i = 0; i < size; ++i) {
           if(func(get(i), i)){
               return true;
           }
        }
        return false;
    }
    int sum(std::function<int(T&,int)> func){
        int sum = 0;
        int size = list.size();
        for (int i = 0; i < size; ++i) {
           sum += func(get(i),i);
        }
        return sum;
    }
    int sum(){
        int sum = 0;
        int size = list.size();
        for (int i = 0; i < size; ++i) {
           sum += getIntImpl(get(i));
        }
        return sum;
    }
    sk_sp<IColumn<T>> shuffle(){
        std::mt19937 g(0); // seed
        std::shuffle(list.begin(), list.end(), g);
        return sk_ref_sp(this);
    }
    //
    sk_sp<IColumn<T>> applyIndexResult(sk_sp<ListI> idxes){
        sk_sp<IColumn<T>> ret = sk_make_sp<IColumn<T>>();
        for(int i = 0 ; i < idxes->size() ; ++i){
            ret->add(get(idxes->get(i)));
        }
        return ret;
    }
    sk_sp<IColumn<sk_sp<IColumn<T>>>> groupByEveryCount(int count){
        auto ret = sk_make_sp<IColumn<sk_sp<IColumn<T>>>>();
        int _size = size();
        int st = 0;
        for(;st < _size; st += count){
            ret->add(sub(st, HMIN(st + count, _size)));
        }
        return ret;
    }
    inline sk_sp<IColumn<sk_sp<IColumn<T>>>> groupByCount(int count){
        return groupByEveryCount(size() / count + (size() % count != 0 ? 1 : 0));
    }

    //------------------------
    IColumn<T>& operator=(IColumn<T>&& oth){
        this->list = oth.list;
        return *this;
    };
    IColumn<T>& operator=(const IColumn<T>& oth){
        this->list = oth.list;
        return *this;
    };
    T& operator[](int i){
        return list[i];
    }

    //------------- R impl ------------------
    sk_sp<ListI> which(std::function<bool(T&,int)> func, ListI* dropIndexes = nullptr){
        sk_sp<ListI> ret = sk_make_sp<ListI>();
        size_t size = list.size();
        for (size_t i = 0; i < size; ++i) {
            if(func(get(i), i)){
                ret->add(i);
            }else if(dropIndexes){
                dropIndexes->add(i);
            }
        }
        return ret;
    }
};

template <>
inline String toStringImpl(const sk_sp<ListS>& l, CString){
    return l->toStringFullly();
}
template <>
inline String toStringImpl(const sk_sp<ListI>& l, CString){
    return l->toStringFullly();
}
template <>
inline String toStringImpl(const sk_sp<IColumn<long>>& l, CString){
    return l->toStringFullly();
}
template <>
inline String toStringImpl(const sk_sp<IColumn<float>>& l, CString){
    return l->toStringFullly();
}
template <>
inline String toStringImpl(const sk_sp<IColumn<double>>& l, CString){
    return l->toStringFullly();
}
template <>
inline String toStringImpl(const sk_sp<IColumn<char>>& l, CString){
    return l->toStringFullly();
}
}

template<typename T>
inline bool operator!=(const sk_sp<h7::IColumn<T>> &a, const sk_sp<h7::IColumn<T>> &b) {
    if(a->size() != b->size()){
        return true;
    }
    int size = a->size();
    for(int i = 0 ; i < size ; i ++){
        if(a->get(i) != b->get(i)){
            return true;
        }
    }
    return false;
}
template<typename T>
inline bool operator==(const sk_sp<h7::IColumn<T>> &a, const sk_sp<h7::IColumn<T>> &b) {
    if(a->size() != b->size()){
        return false;
    }
    int size = a->size();
    for(int i = 0 ; i < size ; i ++){
        if(a->get(i) != b->get(i)){
            return false;
        }
    }
    return true;
}

template<typename T>
inline bool operator<(const sk_sp<h7::IColumn<T>> &a, const sk_sp<h7::IColumn<T>> &b) {
    if(a->size() >= b->size()){
        return false;
    }
    int size = a->size();
    for(int i = 0 ; i < size ; i ++){
        if(!b->contains(a->const_get(i))){
            return false;
        }
    }
    return true;
}

template<typename T>
inline bool operator==(const h7::IColumn<T> &a, const h7::IColumn<T> &b) {
    if(a.size() != b.size()){
        return false;
    }
    int size = a.size();
    for(int i = 0 ; i < size ; i ++){
        if(a.const_get(i) != b.const_get(i)){
            return false;
        }
    }
    return true;
}
template<typename T>
inline bool operator!=(const h7::IColumn<T> &a, const h7::IColumn<T> &b) {
    if(a.size() != b.size()){
        return true;
    }
    int size = a.size();
    for(int i = 0 ; i < size ; i ++){
        if(a.const_get(i) != b.const_get(i)){
            return true;
        }
    }
    return false;
}

template<typename T>
inline bool operator<(const h7::IColumn<T> &a, const h7::IColumn<T> &b) {
    if(a.size() >= b.size()){
        return false;
    }
    int size = a.size();
    for(int i = 0 ; i < size ; i ++){
        if(!b.contains(a.const_get(i))){
            return false;
        }
    }
    return true;
}


#endif
