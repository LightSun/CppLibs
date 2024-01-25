#include "table/core/GRanges.h"
#include "table/SetColumn.h"
#include "utils/collection_utils.h"
#include "utils/convert.hpp"
#include "utils/PerformanceHelper.h"
//#include "utils/BubbleSort.hpp"
#include "utils/binary_search.h"

namespace h7 {

void GRanges::offset(int startOffset, int endOffset){
    int rc = getRowCount();
    for(int i = 0 ; i < rc ; i ++){
        m_starts->get(i) += startOffset;
        m_ends->get(i) += endOffset;
    }
}

sk_sp<ListS> GRanges::getFullRowData(int idx){
    return ListS::NewFromVec<String>(m_keys->getString(idx, ""),
                                     m_starts->getString(idx, ""),
                                     m_ends->getString(idx, "")
                                     );
}

bool GRanges::isInAnyRange(CString key, long long val){
    int _val = val;
    int size = getRowCount();
    for(int i = 0 ; i < size ; i ++){
        if(m_keys->get(i) == key){
            if(_val >= m_starts->get(i) && _val <= m_ends->get(i)){
                return true;
            }
        }
    }
    return false;
}

sk_sp<GRanges> GRanges::sub(ListI* rowIndexes){
    MED_ASSERT(rowIndexes != nullptr);
    int size = rowIndexes->size();
    sk_sp<ListS> keys = sk_make_sp<ListS>();
    sk_sp<ListI> starts = sk_make_sp<ListI>();
    sk_sp<ListI> ends = sk_make_sp<ListI>();
    keys->resize(size);
    starts->resize(size);
    ends->resize(size);
    int id;
    for(int i = 0 ; i < size ; i ++){
        id = rowIndexes->get(i);
        keys->set0(i, m_keys->get(id));
        starts->set0(i, m_starts->get(id));
        ends->set0(i, m_ends->get(id));
    }
    return sk_make_sp<GRanges>(keys, starts, ends);
}

sk_sp<ListS> GRanges::concat(CString sep){
    sk_sp<ListS> ret = sk_make_sp<ListS>();
    int size = getRowCount();
    for(int i = 0 ; i < size ; i ++){
        ret->add(getFullRowData(i)->toString(sep));
    }
    return ret;
}

#define MACRO_processOverlap(l1, l2, i, _idx, s1_start, s1_end, other)\
{\
    int hash = other->m_keyHashes->get(_idx);\
    sk_sp<ListI> column = other->m_indexMap[hash];\
    const int size = column->size();\
    auto st1 = s1_start;\
    auto se1 = s1_end;\
    auto& sts2 = other->m_starts;\
    auto& ses2 = other->m_ends;\
    int j;\
    for(int k = 0 ; k < size ; ++k){\
        j = column->get(k);\
        if(HMIN(se1, ses2->get(j)) >= \
                HMAX(st1, sts2->get(j))){\
            l1->add(i);\
            l2->add(j);\
        }\
    }\
}

sk_sp<MatrixI> GRanges::rFindOverlaps2(GRanges* other, bool){
    if(getRowCount() == 0 || other->getRowCount() == 0){
        return nullptr;
    }
    this->buildMainCache();
    other->buildCache();
    //
    const int ic = getRowCount();
    const int ic2 = other->getRowCount();
    char buf[128];
    snprintf(buf, 128 , "rFindOverlaps(%d, %d)", ic, ic2);
    String str(buf);
    //
    sk_sp<MatrixI> result = sk_make_sp<MatrixI>();
    sk_sp<ListI> l1 = sk_make_sp<ListI>(ic);
    sk_sp<ListI> l2 = sk_make_sp<ListI>(ic);
    result->add(l1);
    result->add(l2);
    //
    PerfHelper helper;
    helper.begin();
    //
    int _idx;
    for(int i = 0 ; i < ic ; i ++){
        _idx = binarySearch(other->m_keyHashes->list.data(), 0,
                            other->m_keyHashes->size(),
                            m_mainHashess->get(i));
        if(_idx >= 0){
            //_processOverlap, MACRO_processOverlap
            MACRO_processOverlap(l1.get(), l2.get(), i, _idx,
                            m_starts->get(i), m_ends->get(i), other);
        }
    }
    helper.print(str);
    return result;
}

void GRanges::_processOverlap(std::vector<std::vector<RPair>>& vec, int i, int _idx,
                              int s1_start, int s1_end,
                              GRanges* other){
    int hash = other->m_keyHashes->get(_idx);
    sk_sp<ListI> column = other->m_indexMap[hash];
    const int size = column->size();
    std::vector<RPair>& list = vec[i];
    list.reserve(size / 2);

    for(int k = 0 ; k < size ; ++k){
        int j = column->get(k);
        //overlap
        if(HMIN(s1_end, other->m_ends->get(j)) >=
                HMAX(s1_start, other->m_starts->get(j))){
            list.emplace_back(i, j);
        }
    }
}

sk_sp<MatrixI> GRanges::rFindOverlaps(GRanges* other, bool){

    if(getRowCount() == 0 || other->getRowCount() == 0){
        return nullptr;
    }
    this->buildMainCache();
    other->buildCache();
    //
    const int ic = getRowCount();
    const int ic2 = other->getRowCount();
    char buf[128];
    snprintf(buf, 128 , "rFindOverlaps2(%d, %d)", ic, ic2);
    String str(buf);
    //
    sk_sp<MatrixI> result = sk_make_sp<MatrixI>();
    sk_sp<ListI> l1 = sk_make_sp<ListI>(ic);
    sk_sp<ListI> l2 = sk_make_sp<ListI>(ic);
    result->add(l1);
    result->add(l2);
    std::vector<std::vector<RPair>> pairs;
    pairs.resize(ic);
    //
    PerfHelper helper;
    helper.begin();
    //
    #pragma omp parallel for
    for(int i = 0 ; i < ic ; i ++){
        int _idx = binarySearch(other->m_keyHashes->list.data(), 0,
                        other->m_keyHashes->size(),
                        m_mainHashess->get(i));
        if(_idx >= 0){
            _processOverlap(pairs, i, _idx,
                            m_starts->get(i), m_ends->get(i), other);
        }
    }
    helper.print(str);
    for(int i = 0, ssize = pairs.size() ; i < ssize ; i ++){
        for(auto& p: pairs[i]){
            l1->add(p.i);
            l2->add(p.j);
        }
    }
    helper.print("rFindOverlaps2_merge");
    return result;
}

//----------------------------
void GRanges::buildMainCache(){
    if(m_mainHashess){
        return;
    }
    int rc = getRowCount();
    m_mainHashess = sk_make_sp<ListI>();
    m_mainHashess->prepareSize(rc);
    for(int i = 0 ; i < rc ; i ++){
        m_mainHashess->add(m_keys->hashCode(i));
    }
}

void GRanges::buildCache(){
    if(m_keyHashes){
        return;
    }
    PerfHelper helper;
    helper.begin();
    const int rc = getRowCount();
    //cache some
    m_keyHashes = sk_make_sp<ListI>(128);
    SetColumn<int> uniqueHashes;
    //
    int hash;
    for (int i = 0; i < rc; i++) {
        hash = m_keys->hashCode(i);
        if(!uniqueHashes.contains(hash)){
            uniqueHashes.add(hash);
            m_keyHashes->add(hash);
        }
        //index
        sk_sp<ListI> indexes = m_indexMap.get(hash);
        if(!indexes){
            indexes = sk_make_sp<ListI>();
            m_indexMap.put0(hash, indexes);
        }
        indexes->add(i);
    }
    //find child tree.
    std::sort(m_keyHashes->list.begin(), m_keyHashes->list.end());
    //BubbleSort<int>::sort(m_keyHashes->list);
    helper.print("buildCache");
}

void GRanges::destroyCache(){
    if(m_mainHashess){
        m_mainHashess.release()->unref();
    }
    if(m_keyHashes){
        m_keyHashes.release()->unref();
    }
    m_indexMap.clear();
}

}

