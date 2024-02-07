#pragma once

#include "table/Column.h"

#define __DEF_SortItem_MAKE(Type, t, sort_t, cvtN)\
    static SortItem make##Type(CString colName, std::function<t(String&)> cvtI32){\
        SortItem si;\
        si.colName = colName;\
        si.cvtN = cvtI32;\
        si.type = sort_t;\
        return si;\
    }\
    static SortItem make##Type(CString colName){\
        SortItem si;\
        si.colName = colName;\
        si.type = sort_t;\
        return si;\
    }\
    static SortItem make##Type(int colIndex, std::function<t(String&)> cvtI32){\
        SortItem si;\
        si.colIndex = colIndex;\
        si.cvtN = cvtI32;\
        si.type = sort_t;\
        return si;\
    }\
    static SortItem make##Type(int colIndex){\
        SortItem si;\
        si.colIndex = colIndex;\
        si.type = sort_t;\
        return si;\
    }

namespace h7 {

class TabContentDelegate;
class Table;
template<typename T>
class SetColumn;

namespace sort {
enum{
    kSortType_STRING,
    kSortType_FLOAT,
    kSortType_UI32,
    kSortType_I32,
    kSortType_UI64,
    kSortType_I64,
};
struct SortItem{
    std::function<String(String&)> cvtStr;
    std::function<float(String&)> cvtFloat;
    std::function<uint32(String&)> cvtUI32;
    std::function<int(String&)> cvtI32;
    std::function<long long(String&)> cvtI64;
    std::function<unsigned long long(String&)> cvtUI64;
    //must assign 'colName' or 'colIndex'
    String colName;
    int colIndex {-1}; //means use colName
    int type {kSortType_STRING};
    bool aesc {true};

    __DEF_SortItem_MAKE(Int, int, kSortType_I32, cvtI32);
    __DEF_SortItem_MAKE(UInt, uint32, kSortType_UI32, cvtUI32);
    __DEF_SortItem_MAKE(Long, sint64, kSortType_I64, cvtI64);
    __DEF_SortItem_MAKE(ULong, uint64, kSortType_UI64, cvtUI64);
    __DEF_SortItem_MAKE(Float, float, kSortType_FLOAT, cvtFloat);
    __DEF_SortItem_MAKE(String, String, kSortType_STRING, cvtStr);
};
}

template<typename T>
class RowVisitor{
public:
    virtual ~RowVisitor() = default;
    void onStart(int rc){
        results.reserve(rc);
    }
    virtual void onVisit(ListS* l){
        results.push_back(func(l));
    }
    virtual void onEnd(){}
public:
    std::function<T(ListS* l)> func;
    std::vector<T> results;
};

template<typename T>
class IndexedRowVisitor{
public:
    virtual ~IndexedRowVisitor() = default;
    void onStart(int rc){
        results.reserve(rc);
    }
    virtual void onVisit(ListS* l, int rIdx){
        results.push_back(func(l, rIdx));
    }
    virtual void onEnd(){}
public:
    std::function<T(ListS* l,int)> func;
    std::vector<T> results;
};

struct TabBlock: public SkRefCnt{
    using CSortItem = const sort::SortItem&;

    sk_sp<GroupS> gs;
    String groupKey;   //group key
    int start_idx {0}; //-1 means idx invalid. like group

    sk_sp<TabBlock> copy();

    void removeIf(std::function<bool(sk_sp<ListS>, int)> func);
    void removeByFlags(ListI* );
    void removeByFlags(ListB* );
    void sort(CSortItem si);
    //
    int getRowCount(){return gs ? gs->size() : 0;}
    template<typename T>
    void visitRowsByColumnIdxes(sk_sp<ListI> idxes, T* visitor);
    template<typename T>
    void visitRows(T* visitor);

    template<typename T>
    void visitRowsByColumnIdxesWithRowIdx(sk_sp<ListI> idxes, T* visitor);
    template<typename T>
    void visitRowsWithRowIdx(T* visitor);

private:
    void sortDefault(CSortItem si);
    static inline void getColumnValues(sk_sp<ListS> row, ListI* idxes, ListS* out);
};

class TabDelegate2: public SkRefCnt
{
public:
    using SPBlock = sk_sp<TabBlock>;
    using SPGroupS = sk_sp<GroupS>;
    using CSortItem = const sort::SortItem&;
    //auto group by max_count.
    TabDelegate2(GroupS*, int max_count);
    //without group
    TabDelegate2(SPGroupS gs, sk_sp<ListS> head);
    //without group
    TabDelegate2(GroupS* gs, sk_sp<ListS> head);
    //
    sk_sp<TabDelegate2> copy();
    //
    void removeIf(std::function<bool(sk_sp<ListS>, int)> func, bool updateIdx = true);
    void removeByFlags(ListI* ,bool updateIdx = true);
    void removeByFlags(ListB* ,bool updateIdx = true);

    void sort(CSortItem si);
    void groupByEveryCount(int every);
    void groupByThreadCount(int count);
    //group by column value which is indicated by index
    void group(int index);
    //group by column value which is indicated by name
    void group(CString colName);
    //group by target callback
    void group(std::function<String(sk_sp<ListS>)> func);
    void group(CString colName, std::function<String(String&)> func);
    void removeGroups(std::function<bool(String&)> func, bool updateIdx = true);
    void sortGroup(std::function<bool(SPBlock&, SPBlock&)> func);
    //
    void copyTo(GroupS* gs);
    void copyToGroup();
    /// update all to the group, and copy data to table.
    sk_sp<h7::Table> toTable();
    /// like 'toTable()', but not update the group, and share the head-line and content.
    sk_sp<h7::Table> asTable();
    //update index of all blocks.
    void updateIdx(int startIdx = 0);
    //update idx and compose all group data to origin.
    void updateAll(int startIdx = 0);
    //
    int getRowCount(){return m_rowCount;}
    int getColumnCount();
    bool hasContent(){return getRowCount() > 0;}
    void setHeadLine(sk_sp<ListS> head){m_headLine = head;}

    int getBlockSize(){return m_blocks.size();}
    SPBlock getBlock(int index){return m_blocks.get(index);}
    sk_sp<TabDelegate2> getBlockAsTab(int index, bool copy = false);
    SPBlock findBlock(CString groupName);
    sk_sp<TabDelegate2> findBlockAsTab(CString groupName, bool copy = false);
    //
    void makeColumnCache(bool force = false);
    void clearColumnCache(){m_columnDelegate = nullptr;}
    int requireColumn(CString name);
    int getColumnIndex(CString name){return m_headLine ? m_headLine->indexOf(name) : -1;}
    sk_sp<ListI> requireColumns(sk_sp<ListS> colNames);
    sk_sp<ListS> getColumn(CString name, bool copy);
    sk_sp<ListS> optColumn(CString name, bool copy);
    //after call this. you may need call 'group(...)'
    void copyToGroupByColumnDelegate();
    sk_sp<TabContentDelegate> getColumnDelegate(){return m_columnDelegate;}
    //
    sk_sp<ListI> visitRowsToInt(sk_sp<ListI> idxes, std::function<int(ListS*)> func);
    template<typename T>
    sk_sp<h7::IColumn<T>> visitRows(sk_sp<ListI> idxes, std::function<T(ListS*)> func);
    template<typename T>
    sk_sp<h7::IColumn<T>> visitRows(sk_sp<ListS> colNames, std::function<T(ListS*)> func);

    template<typename T>
    sk_sp<h7::IColumn<T>> visitRows(sk_sp<ListI> idxes, std::function<T(ListS*,int)> func);
    template<typename T>
    sk_sp<h7::IColumn<T>> visitRows(sk_sp<ListS> colNames, std::function<T(ListS*,int)> func);

    //
    void filterRowSelf(sk_sp<ListS> colNames, std::function<bool(h7::ListS* l)> func);
    void filterRowSelfByColumnIndexes(sk_sp<ListI> idxes,
                                      std::function<bool(h7::ListS* l)> func);
    void filterRowSelf(CString colName, std::function<bool(String& s)> func);
    void removeNotExist(CString col_name, SetColumn<String>* valueList);
    void removeNotExist(CString col_name, ListS* valueList);

private:
    TabDelegate2(){}

private:
    int m_rowCount {-1};
    GroupS* m_gs;
    SPGroupS m_gs_local;
    sk_sp<ListS> m_headLine;
    IColumn<SPBlock> m_blocks;

    sk_sp<TabContentDelegate> m_columnDelegate;
};
//---------------------------------

void TabBlock::getColumnValues(sk_sp<ListS> row, ListI* idxes, ListS* out){
    for(int i = 0 ; i < idxes->size() ; ++i){
        out->add(row->get(idxes->get(i)));
    }
}
template<typename T>
void TabBlock::visitRowsByColumnIdxes(sk_sp<ListI> idxes, T* visitor){
    if(idxes){
        int rc = getRowCount();
        visitor->onStart(rc);
        for(int i = 0 ; i < rc ; ++i){
            ListS temp;
            getColumnValues(gs->get(i), idxes.get(), &temp);
            visitor->onVisit(&temp);
        }
        visitor->onEnd();
    }else{
        int rc = getRowCount();
        visitor->onStart(rc);
        for(int i = 0 ; i < rc ; ++i){
            visitor->onVisit(gs->get(i).get());
        }
        visitor->onEnd();
    }
}
template<typename T>
void TabBlock::visitRows(T* visitor){
    visitRowsByColumnIdxes(nullptr, visitor);
}

template<typename T>
void TabBlock::visitRowsByColumnIdxesWithRowIdx(sk_sp<ListI> idxes, T* visitor){
    if(idxes){
        int rc = getRowCount();
        visitor->onStart(rc);
        for(int i = 0 ; i < rc ; ++i){
            ListS temp;
            getColumnValues(gs->get(i), idxes.get(), &temp);
            visitor->onVisit(&temp, start_idx + i);
        }
        visitor->onEnd();
    }else{
        int rc = getRowCount();
        visitor->onStart(rc);
        for(int i = 0 ; i < rc ; ++i){
            visitor->onVisit(gs->get(i).get(), start_idx + i);
        }
        visitor->onEnd();
    }
}
template<typename T>
void TabBlock::visitRowsWithRowIdx(T* visitor){
    visitRowsByColumnIdxesWithRowIdx(nullptr, visitor);
}

template<typename T>
sk_sp<h7::IColumn<T>> TabDelegate2::visitRows(sk_sp<ListI> idxes,
                                          std::function<T(ListS*)> func){
    int s = m_blocks.size();
    std::vector<std::vector<T>> groups;
    groups.resize(s);
#pragma omp parallel for
    for(int i = 0 ; i < s; ++i){
        RowVisitor<T> rv;
        rv.func = func;
        m_blocks.get(i)->visitRowsByColumnIdxes(idxes, &rv);
        groups[i] = std::move(rv.results);
    }
    int total = 0;
    for(size_t i = 0 ; i < groups.size() ; ++i){
        total += groups[i].size();
    }
    auto ret = sk_make_sp<h7::IColumn<T>>(total);
    for(size_t i = 0 ; i < groups.size() ; ++i){
        ret->addAll(groups[i]);
    }
    return ret;
}
template<typename T>
sk_sp<h7::IColumn<T>> TabDelegate2::visitRows(sk_sp<ListS> colNames,
                                              std::function<T(ListS*)> func){
    return visitRows(requireColumns(colNames), func);
}
template<typename T>
sk_sp<h7::IColumn<T>> TabDelegate2::visitRows(sk_sp<ListI> idxes,
                                              std::function<T(ListS*,int)> func){
    int s = m_blocks.size();
    std::vector<std::vector<T>> groups;
    groups.resize(s);
    //
#pragma omp parallel for
    for(int i = 0 ; i < s; ++i){
        IndexedRowVisitor<T> rv;
        rv.func = func;
        m_blocks.get(i)->visitRowsByColumnIdxesWithRowIdx(idxes, &rv);
        groups[i] = std::move(rv.results);
    }
    int total = 0;
    for(size_t i = 0 ; i < groups.size() ; ++i){
        total += groups[i].size();
    }
    auto ret = sk_make_sp<h7::IColumn<T>>(total);
    for(size_t i = 0 ; i < groups.size() ; ++i){
        ret->addAll(groups[i]);
    }
    return ret;
}
template<typename T>
sk_sp<h7::IColumn<T>> TabDelegate2::visitRows(sk_sp<ListS> colNames,
                                              std::function<T(ListS*,int)> func){
     return visitRows(requireColumns(colNames), func);
}

inline void TabDelegate2::sortGroup(std::function<bool(SPBlock& b1, SPBlock& b2)> func_smaller){
    std::sort(m_blocks.begin(), m_blocks.end(), func_smaller);
    updateIdx();
}

}
