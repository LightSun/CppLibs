#pragma once

#include <vector>
#include <unordered_map>
#include <map>
#include <initializer_list>
#include "table/Column.h"
#include "common/common.h"
//#include "utils/IndexComparator.hpp"

namespace h7 {

template <typename T>
class SetColumn;

class TabContentDelegate: public SkRefCnt{
public:

    TabContentDelegate(){}
    TabContentDelegate(sk_sp<GroupS> rowData){
        setRows(rowData);
    }
    TabContentDelegate(const GroupS& rowData){
        GroupS* gs = (GroupS*)&rowData;
        setRows(gs);
    }

    static sk_sp<TabContentDelegate> ofColumns(GroupS* colDatas){
        sk_sp<TabContentDelegate> ret = sk_make_sp<TabContentDelegate>();
        ret->setColumns(colDatas);
        return ret;
    }
    static sk_sp<TabContentDelegate> ofColumns(sk_sp<GroupS> colDatas){
        sk_sp<TabContentDelegate> ret = sk_make_sp<TabContentDelegate>();
        ret->setColumns(colDatas);
        return ret;
    }

    int computeHash(int row_idx, ListI* col_idxes);
    int computeHash(int row_idx, int col_idx);

    int getRowCount();
    int getRowCount()const;
    int getColumnCount()const;

    //col_idxes: nullable
    sk_sp<ListS> get(int row_idx, ListI* col_idxes);
    void get(int row_idx, ListI* col_idxes, ListS& out);
    //row_idxes: nullable
    sk_sp<ListS> get(ListI* row_idxes, int col_idx, bool copy = false);
    String& get(int row_idx, int col_idx);

    void set(int row_idx, int column_idx, const String& val);

    sk_sp<ListS> getRow(int row_idx);
    void getRow(int row_idx, ListS* out);
    String getRowToStr(int row_idx, CString seq);
    //return a copy column
    sk_sp<ListS> getColumn(int col_idx, bool copy);
    sk_sp<ListS> optColumn(int col_idx, bool copy);

    sk_sp<ListS> getColumnAsUnique(int col_idx);
    sk_sp<ListI> getIntColumn(int col_idx);
    sk_sp<h7::SetColumn<String>> getColumnAsSet(int col_idx);
    void getColumn(int col_idx, ListS& out);

    void prepareColumnCount(int size);
    void prepareRowCount(int size);

    void getRows(GroupS& list, ListI* colIdxes);
    void getRows(GroupS& list);

    inline void setRows(sk_sp<GroupS> list){
        setRows(list.get());
    }
    inline void setRows(const GroupS& list){
         GroupS* gs = (GroupS*)&list;
         setRows(gs);
    }
    void setRows(GroupS& list);
    void setRows(GroupS* list);

    void removeRow(int row_idx);
    void removeColumn(int col_idx);

    inline void setColumns(const std::vector<sk_sp<ListS>>& vecs){
        m_columns.setAll(vecs);
    }
    inline void setColumns(sk_sp<IColumn<sk_sp<ListS>>> vecs){
        m_columns.setAll(vecs);
    }
    inline void setColumns(IColumn<sk_sp<ListS>>* vecs){
        m_columns.setAll(vecs);
    }

    void addContentByRow(TabContentDelegate* sp);
    inline void addContentByRow(sk_sp<TabContentDelegate> sp){
        addContentByRow(sp.get());
    }
    inline void addContentByColumn(sk_sp<TabContentDelegate> sp){
        addContentByColumn(sp.get());
    }
    void addContentByColumn(TabContentDelegate* sp);

    inline void addColumnAsFull(std::initializer_list<String> list){
        sk_sp<ListS> sp = sk_make_sp<ListS>();
        sp->list.assign(list);
        addColumnAsFull(sp);
    }
    inline void addColumnAsFull(sk_sp<ListS> column){
        m_columns.add(column);
    }
    inline void addColumnsAsFull(const IColumn<sk_sp<ListS>>& cols){
        m_columns.addAll(cols);
    }
    void addRowAsFull(std::vector<String>& row);
    void addRowAsFullDirect(std::vector<String>& row);
    void addRowAsFull(ListS* column);
    inline void addRowAsFull(std::initializer_list<String> list){
        sk_sp<ListS> sp = sk_make_sp<ListS>();
        sp->list.assign(list);
        addRowAsFull(sp.get());
    }
    inline void addRowAsFull(sk_sp<ListS> column){
        addRowAsFull(column.get());
    }

    void swapColumn(int idx1, int idx2);

    void swapRow(int idx1, int idx2);
    void sortRow(int index, std::function<int(String&, String&)> func, bool desc);
    void sortRow(int index, bool desc){
        auto func = [](String& s1, String& s2){
            return s1 < s2;
        };
        sortRow(index, func, desc);
    }
    void sortRow(std::function<int(ListS*, ListS*)> func);
    //void sortRow(IndexComparator<sk_sp<ListS>>* cmp, std::function<void(int,int)> cb);

    void sortRowByFloatColumn(int col_index, std::function<int(float, float)> func);
    void sortRowByFloatColumn(int col_index, bool aesc);

    sk_sp<TabContentDelegate> copy();
    void align(int expectColumnCount);

    const String toString(bool oneLine = true);

    IColumn<sk_sp<ListS>>& getColumns(){
        return m_columns;
    }
    //-------------------------------------
    String& operator()(int x, int y){
        return m_columns.get(y)->get(x);
    }
    sk_sp<ListS> operator()(int y){
        return m_columns.get(y);
    }

private:
    IColumn<sk_sp<ListS>> m_columns;
};

}
