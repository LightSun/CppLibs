#ifndef COLLECTION_UTILS_H
#define COLLECTION_UTILS_H

#include <map>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include "table/Column.h"
#include "table/SetColumn.h"
#include "utils/Pair.h"

#define INVALID_INT INT_MIN
#define ARRAYS_ASLIST_STR(...) \
    h7::utils::ofListN<String>(__VA_ARGS__)
#define ARRAYS_ASLIST_INT(...) \
    h7::utils::ofListN<int>(__VA_ARGS__)

namespace h7 {
namespace utils {


template <typename T>
inline bool isEmpty(sk_sp<IColumn<T>> sp){
    return !sp || sp->size() == 0;
}

template <typename T>
inline bool isEmpty(IColumn<T>* sp){
    return sp == nullptr || sp->size() == 0;
}

template <typename T>
inline sk_sp<IColumn<T>> ofList(const T& t1){
    sk_sp<IColumn<T>> sp = sk_make_sp<IColumn<T>>();
    sp->add(t1);
    return sp;
}

template <typename T>
inline sk_sp<IColumn<T>> ofList(const T& t1, const T& t2){
    sk_sp<IColumn<T>> sp = sk_make_sp<IColumn<T>>();
    sp->add(t1);
    sp->add(t2);
    return sp;
}

template <typename T>
inline sk_sp<IColumn<T>> ofList(const T& t1, const T& t2, const T& t3){
    T arr[3] = {t1, t2, t3};
    std::vector<T> list(arr, arr + 3);
    return sk_make_sp<IColumn<T>>(list);
}

template <typename T,class ...Args>
inline sk_sp<IColumn<T>> ofListN(Args&&... args){
    int size = sizeof...(args);
    T arr[size] = {args...};
    std::vector<T> list(arr, arr + size);
    return sk_make_sp<IColumn<T>>(list);
}
template <typename T,class ...Args>
inline sk_sp<SetColumn<T>> ofSetN(Args&&... args){
    int size = sizeof...(args);
    T arr[size] = {args...};
    std::unordered_set<T> list(arr, arr + size);
    return sk_make_sp<SetColumn<T>>(list);
}
//------------------------------------------

template <typename T>
sk_sp<ListI> rMatch(IColumn<T>* list1, IColumn<T>* list2);

template <typename T>
inline sk_sp<ListI> rMatch(sk_sp<IColumn<T>> list1, sk_sp<IColumn<T>> list2){
    return rMatch(list1.get(), list2.get());
}

template <typename T>
inline uint32 rMatchCount(IColumn<T>* list1, IColumn<T>* list2){
    auto ret = rMatch(list1, list2);
    uint32 overlap_c = 0;
    for(int i = 0 ; i < ret->size() ; i ++){
        if(ret->get(i) != INVALID_INT){
            overlap_c ++;
        }
    }
    return overlap_c;
}

//excludes can be null
template <typename T>
sk_sp<IColumn<Pair<T,int>>> rTable(IColumn<T>* main, std::unordered_set<T>* excludes,
                               IColumn<sk_sp<ListI>>* outIdxes);

//sort array with AESC
//start: include
//end: exclude
template <typename T>
std::vector<int> sortReturnIndex(std::vector<T> &array,int start, int end,
                                 bool sortArray = true);

template <typename T>
inline void sortReturnIndex2(std::vector<T> &array,std::vector<unsigned int>& indexes,
                                 bool sortArray = true){
    MED_ASSERT(array.size() == indexes.size());
    //array not changed
    const int array_len = array.size();
    for (unsigned int i = 0; i < array_len; ++i){
        indexes[i] = i;
    }
    //aesc. if the element val is the same we should check index is 'AESC'
    std::sort(
      indexes.begin(), indexes.end(),
      [&array](int pos1, int pos2) {
        return array[pos1] != array[pos2] ? (array[pos1] < array[pos2]) : (pos1 < pos2);
    });
    if(sortArray){
        std::vector<T> copyArr(array);
        for(int i = 0 ; i < array_len ; i ++){
            array[i] = copyArr[indexes[i]];
        }
    }
}

//start: include
//end: exclude
template <typename T>
std::vector<int> sortIndexWithAssign(std::vector<T> &array,int start, int end,
                                 std::function<void(int dst, int src)> func_assign);
//start: include
//end: exclude
template <typename T>
void sortWithAssign(std::vector<T> &array,int start, int end,
                                 std::function<bool(int pos1, int pos2)> func_sort,
                                 std::function<void(int dst, int src)> func_assign);
//--------------------------------------
template <typename K, typename V>
sk_sp<IColumn<K>> keyList(std::map<K, V>& test);

template <typename K, typename V>
sk_sp<IColumn<V>> valueList(std::map<K, V>& test);

template <typename K, typename V>
sk_sp<IColumn<V>> valueList(std::unordered_map<K, V>& test);

template <typename K, typename V>
inline void keyList2(std::map<K, V>& test, IColumn<K>* keys)
{
    for(auto it = test.begin(); it != test.end(); ++it){
        keys->add(it->first);
    }
}

template <typename K, typename V>
inline void valueList2(std::map<K, V>& test, IColumn<V>* keys)
{
    for(auto it = test.begin(); it != test.end(); ++it){
        keys->add(it->second);
    }
}
//--------------------------------------------------------
template <typename T>
inline sk_sp<IColumn<sk_sp<T>>> productSpList(int count){
    sk_sp<IColumn<sk_sp<T>>> sp = sk_make_sp<IColumn<T>>();
    sp->prepareSize(count);
    for(int i = 0 ; i < count ; ++i){
        sp->add(sk_make_sp<T>());
    }
    return sp;
}
template <typename T>
inline sk_sp<IColumn<sk_sp<IColumn<T>>>> productListList(int count){
    sk_sp<IColumn<sk_sp<IColumn<T>>>> sp = sk_make_sp<IColumn<sk_sp<IColumn<T>>>>();
    sp->prepareSize(count);
    for(int i = 0 ; i < count ; i ++){
        sp->add(sk_make_sp<IColumn<T>>());
    }
    return sp;
}

sk_sp<ListI> produceInt2(int start, int end);
sk_sp<ListI> produceInt(int count, int val);
sk_sp<ListI> orderInt(int start, int end);

template <typename T>
sk_sp<IColumn<T>> produceList(int count, const T& val);
}
}

#include "utils/collection_utils.inc"

#endif // COLLECTION_UTILS_H
