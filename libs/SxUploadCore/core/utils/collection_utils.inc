#include "collection_utils.h"
//#include "utils/IndirectSorter.hpp"
#include "binary_search.h"

namespace h7 {
namespace utils {

//foreach list1: find idx from list2
template <typename T>
sk_sp<ListI> rMatch(IColumn<T>* list1, IColumn<T>* list2){
    sk_sp<ListI> l1 = list1->toHash();
    sk_sp<ListI> l2 = list2->toHash();

    //IndexComparator<int> ic;
    //sk_sp<ListI> list2_indexes = IndirectSorter<int>::sort(l2.get(), &ic);
    ListI list2_indexes(sortReturnIndex(l2->list, 0, l2->size(), true));
    //
    const int size1 = list1->size();
    const int size2 = list2->size();
    sk_sp<ListI> ret = sk_make_sp<ListI>(size1, true);

    #pragma omp parallel for
    for(int i = 0 ; i < size1 ; i ++){
        if(l1->get(i) == INVALID_INT){
            ret->set0(i, INVALID_INT);
            continue;
        }
        int _idx = h7::binarySearch(l2->list.data(), 0, size2, l1->get(i));
        if(_idx >= 0){
            ret->set0(i, list2_indexes.get(_idx));
        }else{
            ret->set0(i, INVALID_INT);
        }
    }
    //#pragma omp barrier
    return ret;
}

template <typename T>
sk_sp<IColumn<Pair<T,int>>> rTable(IColumn<T>* main, std::unordered_set<T>* excludes,
                               IColumn<sk_sp<ListI>>* outIdxes){
    sk_sp<IColumn<Pair<T,int>>> list = sk_make_sp<IColumn<Pair<T,int>>>();
    std::unordered_map<T, int> indexMap;
    int idx;
    for(int i = 0; i < main->size(); i ++){
       T& t = main->get(i);
       //not in exclude
       if(excludes == nullptr || excludes->find(t) == excludes->end()){
           if(indexMap.find(t) != indexMap.end()){
               idx = indexMap[t];
               list->get(idx).value ++;
               if(outIdxes){
                   outIdxes->get(idx)->add(i);
               }
           }else{
               indexMap.emplace(t, list->size());
               list->add(h7::make_pair(t, 1));
               if(outIdxes){
                   outIdxes->add(ofList<int>(i));
               }
           }
       }
    }
    return list;
}

template <typename T>
sk_sp<IColumn<T>> produceList(int count, const T& val){
    sk_sp<IColumn<T>> sp = sk_make_sp<IColumn<T>>();
    sp->resize(count);
    for(int i = 0 ; i < count ; i ++){
        sp->set0(i, val);
    }
    return sp;
}

//--------------------------------------
template <typename K, typename V>
sk_sp<IColumn<K>> keyList(std::map<K, V>& test)
{
    sk_sp<IColumn<K>> keys = sk_make_sp<IColumn<K>>();
    for(auto it = test.begin(); it != test.end(); ++it){
        keys->add(it->first);
    }
    return keys;
}

template <typename K, typename V>
sk_sp<IColumn<V>> valueList(std::map<K, V>& test)
{
    sk_sp<IColumn<V>> keys = sk_make_sp<IColumn<V>>();
    for(auto it = test.begin(); it != test.end(); ++it){
        keys->add(it->second);
    }
    return keys;
}

template <typename K, typename V>
sk_sp<IColumn<V>> valueList(std::unordered_map<K, V>& test){
    sk_sp<IColumn<V>> keys = sk_make_sp<IColumn<V>>();
    for(auto it = test.begin(); it != test.end(); ++it){
        keys->add(it->second);
    }
    return keys;
}

template <typename T>
std::vector<int> sortReturnIndex(std::vector<T> &array,int start, int end,
                                 bool sortArray) {

    MED_ASSERT(start >=0 && start < end);
    MED_ASSERT(end > 0 && end <= (int)array.size());
    //array not changed
    const int array_len = end - start;
    std::vector<int> indexes(array_len, 0);
    for (int i = 0; i < array_len; ++i){
        indexes[i] = i + start;
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
            array[i + start] = copyArr[indexes[i]];
        }
    }
    return indexes;
}

template <typename T>
std::vector<int> sortIndexWithAssign(std::vector<T> &array,int start, int end,
                                 std::function<void(int dst, int src)> func_assign) {

    MED_ASSERT(start >=0 && start < end);
    MED_ASSERT(end > 0 && end <= (int)array.size());
    //array not changed
    const int array_len = end - start;
    std::vector<int> indexes(array_len, 0);
    for (int i = 0; i < array_len; ++i){
        indexes[i] = i + start;
    }
    //aesc. if the element val is the same we should check index is 'AESC'
    std::sort(
      indexes.begin(), indexes.end(),
      [&array](int pos1, int pos2) {
        return array[pos1] != array[pos2] ? (array[pos1] < array[pos2]) : (pos1 < pos2);
    });
    if(func_assign){
        //std::vector<T> copyArr(array);
        for(int i = 0 ; i < array_len ; i ++){
            //array[i + start] = copyArr[indexes[i]];
            func_assign(i+ start, indexes[i]);
        }
    }
    return indexes;
}

template <typename T>
void sortWithAssign(std::vector<T> &array,int start, int end,
                                 std::function<bool(int pos1, int pos2)> func_sort,
                                 std::function<void(int dst, int src)> func_assign) {

    MED_ASSERT(start >=0 && start < end);
    MED_ASSERT(end > 0 && end <= (int)array.size());
    //array not changed
    const int array_len = end - start;
    std::vector<int> indexes(array_len, 0);
    for (int i = 0; i < array_len; ++i){
        indexes[i] = i + start;
    }
    //aesc. if the element val is the same we should check index is 'AESC'
    std::sort(
      indexes.begin(), indexes.end(),
      func_sort);
    if(func_assign){
        //std::vector<T> copyArr(array);
        for(int i = 0 ; i < array_len ; i ++){
            //array[i + start] = copyArr[indexes[i]];
            func_assign(i+ start, indexes[i]);
        }
    }
}

}}
