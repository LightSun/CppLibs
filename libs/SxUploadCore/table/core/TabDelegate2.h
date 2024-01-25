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

struct TabBlock: public SkRefCnt{
    using CSortItem = const sort::SortItem&;

    sk_sp<GroupS> gs;
    String groupKey;   //group key
    int start_idx {0}; //-1 means idx invalid. like group

    void removeIf(std::function<bool(sk_sp<ListS>, int)> func);
    void removeByFlags(ListI* );
    void removeByFlags(ListB* );
    void sort(CSortItem si);

private:
    void sortDefault(CSortItem si);
};

class TabDelegate2: public SkRefCnt
{
public:
    using SPBlock = sk_sp<TabBlock>;
    using SPGroupS = sk_sp<GroupS>;
    using CSortItem = const sort::SortItem&;
    //auto group by max_count.
    TabDelegate2(GroupS*, int max_count);
    TabDelegate2(SPGroupS gs, sk_sp<ListS> head);
    TabDelegate2(GroupS* gs, sk_sp<ListS> head);
    //
    void removeIf(std::function<bool(sk_sp<ListS>, int)> func);
    void removeByFlags(ListI* );
    void removeByFlags(ListB* );
    void sort(CSortItem si);
    void group(int index);
    void group(CString name);
    void group(std::function<String(sk_sp<ListS>)> func);
    void group(CString name, std::function<String(String&)> func);
    //
    void copyTo(GroupS* gs);
    void copyToGroup();
    //update index of all blocks.
    void updateIdx(int startIdx = 0);
    //
    int getRowCount(){return m_gs->size();}
    bool hasContent(){return m_gs->size() > 0;}
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
    sk_sp<ListS> getColumn(CString name, bool copy);
    sk_sp<ListS> optColumn(CString name, bool copy);

private:
    TabDelegate2(){}

private:
    GroupS* m_gs;
    SPGroupS m_gs_local;
    sk_sp<ListS> m_headLine;
    IColumn<SPBlock> m_blocks;

    sk_sp<TabContentDelegate> m_columnDelegate;
};

}
