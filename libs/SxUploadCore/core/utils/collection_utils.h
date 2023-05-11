#ifndef COLLECTION_UTILS_H
#define COLLECTION_UTILS_H

#include "common/common.h"

namespace h7 {
namespace utils {

//sort array with AESC
//start: include
//end: exclude
template <typename T>
inline std::vector<int> sortReturnIndex(std::vector<T> &array,int start, int end,
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

//#include "utils/collection_utils.inc"

}}
#endif // COLLECTION_UTILS_H
