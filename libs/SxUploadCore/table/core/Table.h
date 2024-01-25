#ifndef TABLE_H
#define TABLE_H

#include "common/common.h"
#include <regex>
#include <unordered_map>
#include "common/SkRefCnt.h"
#include "table/core/TabContentDelegate.h"
#include "table/core/GRanges.h"
#include "table/SetColumn.h"
#include "utils/Pair.h"

#define HTABLE_CALL_PERF(t, func, ...)\
{\
ph.begin();\
(t)->func(##__VA_ARGS__);\
ph.print(#func + "__rc=" + std::to_string(t->getRowCount()));\
}

#define HTABLE_CALL_GENERIC(t, func, T,...)\
{\
ph.begin();\
(t)->func##T(##__VA_ARGS__);\
ph.print(#func + "__rc=" + std::to_string(t->getRowCount()));\
}

namespace h7 {

template <typename T>
class SetColumn;

class TabDelegate2;

class Table : public SkRefCnt{
public:
    enum{
        kSortType_STRING,
        kSortType_UINT,
        kSortType_FLOAT,
    };
    struct SortItem{
        std::function<String(String&)> group;//

        std::function<String(String&)> cvtStr;
        std::function<float(String&)> cvtF;
        std::function<uint32(String&)> cvtUint;
        String name;
        bool aesc {true};
        int type {kSortType_STRING};
        int _index {-1}; //internal
    };
    using LSortItems = std::vector<SortItem>&;
    typedef Pair<String, int> PairSI;
    //----------
    Table(){
        this->m_headLine = sk_make_sp<ListS>();
        this->m_delegate = sk_make_sp<TabContentDelegate>();
    }
    Table(sk_sp<GroupS> data){
        this->m_headLine = sk_make_sp<ListS>();
        this->m_delegate = sk_make_sp<TabContentDelegate>(data);
    }
    Table(sk_sp<ListS> headLine, sk_sp<GroupS> data):m_headLine(headLine){
        this->m_delegate = sk_make_sp<TabContentDelegate>(data);
    }
    Table(const GroupS& data){
        this->m_headLine = sk_make_sp<ListS>();
        this->m_delegate = sk_make_sp<TabContentDelegate>(data);
    }
    Table(const Table& tab){
        this->m_name = tab.m_name;
        this->m_headLine = tab.m_headLine;
        this->m_delegate = tab.m_delegate;
    }
    Table(CString m_name, sk_sp<ListS> m_headLine, sk_sp<TabContentDelegate> m_delegate):
        m_name(m_name), m_headLine(m_headLine), m_delegate(m_delegate){
    }
    Table(sk_sp<ListS> m_headLine, sk_sp<TabContentDelegate> m_delegate):
        m_headLine(m_headLine), m_delegate(m_delegate){
    }
    Table(sk_sp<ListS> m_headLine):
        m_headLine(m_headLine){
        this->m_delegate = sk_make_sp<TabContentDelegate>();
    }
    Table(sk_sp<TabContentDelegate> m_delegate):
        m_delegate(m_delegate){
        this->m_headLine = sk_make_sp<ListS>();
    }
    inline int getRowCount() {
        return m_delegate->getRowCount();
    }
    inline int getColumnCount() {
        return m_delegate->getColumnCount();
    }
    inline sk_sp<TabContentDelegate> getDelegate()const{
        return m_delegate;
    }
    inline void setName(CString name){
        this->m_name = name;
    }
    inline String& getName(){
        return m_name;
    }
    inline sk_sp<IColumn<String>> getHeadLine(){
        return m_headLine;
    }
    inline bool hasHeadLine(){
        return m_headLine && m_headLine->size() > 0;
    }
    inline int requireColumn(CString name){
        int idx = m_headLine->indexOf(name);
        MED_ASSERT_X(idx >= 0, "can't find column name = " + name);
        return idx;
    }
    inline bool hasColumn(CString name){
        return m_headLine && m_headLine->indexOf(name) >=0 ;
    }
    inline int getColumnIndex(CString name)const{
        return m_headLine->indexOf(name);
    }
    inline void setHeadLine(sk_sp<IColumn<String>> col){
        m_headLine = col;
    }
    inline void getContents(GroupS& out){
        return m_delegate->getRows(out);
    }
    inline void setContents(sk_sp<GroupS> out){
        m_delegate->setRows(out);
    }
    inline void setContents(const GroupS& out){
        m_delegate->setRows(out);
    }
    inline bool hasContent(){
        return m_delegate->getRowCount() > 0;
    }
    inline void setColumns(const std::vector<sk_sp<ListS>>& vecs){
        m_delegate->setColumns(vecs);
    }
    inline void mergeTableByRow(Table* table){
        if(table->getRowCount() == 0 || table->getColumnCount() == 0){
            return;
        }
        m_delegate->addContentByRow(table->m_delegate);
    }
    inline void mergeTableByColumn(Table* table){
        if(table->getRowCount() == 0 || table->getColumnCount() == 0){
            return;
        }
        m_delegate->addContentByColumn(table->m_delegate);
    }
    inline void mergeTableByRow(sk_sp<Table> table){
        mergeTableByRow(table.get());
    }
    inline void mergeTableByColumn(sk_sp<Table> table){
        mergeTableByColumn(table.get());
    }

    void prepareColumns(sk_sp<ListS> names, CString defVal);
    void alignByNames(ListS* names);
//------------------------------------------------
    inline void align(int expectColumnCount = -1){
        m_delegate->align(expectColumnCount);
    }
    sk_sp<Table> copy();

    inline void prepareRowCount(int c){
        m_delegate->prepareRowCount(c);
    }
    inline void prepareColumnCount(int c){
        m_delegate->prepareColumnCount(c);
    }
    inline sk_sp<ListS> getRow(int index){
        return m_delegate->getRow(index);
    }
    inline String getRowToStr(int index, CString seq){
        return m_delegate->getRowToStr(index, seq);
    }
    inline void addRowAsFull(sk_sp<ListS> sp){
         m_delegate->addRowAsFull(sp.get());
    }
    inline void addRowAsFull(ListS& list){
         m_delegate->addRowAsFull(&list);
    }
    inline void addRowAsFull(const ListS& list){
        ListS* ls = (ListS*)&list;
        m_delegate->addRowAsFull(ls);
    }
    inline void addRowAsFull(std::vector<String>& list){
         m_delegate->addRowAsFull(list);
    }
    inline void addRowAsFullDirect(std::vector<String>& list){
         m_delegate->addRowAsFullDirect(list);
    }
    inline sk_sp<ListS> get(int index, IColumn<int>* col_idxes){
        return m_delegate->get(index, col_idxes);
    }
    inline String& get(int index, int col_idx)const{
        return m_delegate->get(index, col_idx);
    }
    //col_names: null means empty out.with all columns
    inline void requireIndexes(ListS* col_names, ListI& out){
        if(col_names != nullptr){
            int idx;
            for(int i = 0; i < col_names->size() ; i++){
                idx = requireColumn(col_names->get(i));
                if(idx >= 0){
                    out.add(idx);
                }
            }
        }else{
//            int size = m_headLine->size();
//            for(int i = 0 ; i < size ;i ++){
//                out.add(i);
//            }
              //empty means all
        }
    }
    sk_sp<ListI> getColumnIndexes(sk_sp<ListS> names){
        sk_sp<ListI> li = sk_make_sp<ListI>(names->size());
        requireIndexes(names.get(), *li);
        return li;
    }
    inline void getByColumIndexes(int row_index, ListI* col_idxes, ListS& out){
        m_delegate->get(row_index, col_idxes, out);
    }
    inline sk_sp<ListS> getByColumnNames(int row_index, ListS* outColumnNames){
        ListI col_idxes;
        requireIndexes(outColumnNames, col_idxes);
        sk_sp<ListS> out = sk_make_sp<ListS>();
        m_delegate->get(row_index, &col_idxes, *out);
        return out;
    }
    inline sk_sp<ListS> getColumnData(ListI* row_indexes, int column_index,
                                      bool copy = false){
        return m_delegate->get(row_indexes, column_index, copy);
    }

    inline sk_sp<ListS> getColumn(CString columnName, bool copy){
        int idx = requireColumn(columnName);
        return idx >=0 ? getColumn(idx, copy) : nullptr;
    }
    inline sk_sp<ListS> getColumn(int index, bool copy){
        return m_delegate->getColumn(index, copy);
    }
    bool updateColumnName(CString oldName, CString newName){
        int idx = m_headLine->indexOf(oldName);
        if(idx >= 0){
            m_headLine->set0(idx, newName);
            return true;
        }
        return false;
    }
    bool updateColumnNameStartWith(CString prefix, CString newName);

    inline sk_sp<ListS> optColumn(CString columnName, bool copy){
        int idx = m_headLine->indexOf(columnName);
        return idx >=0 ? m_delegate->optColumn(idx, copy) : nullptr;
    }
    inline void removeColumn(CString columnName){
        int idx = requireColumn(columnName);
        if(idx >= 0){
            removeColumnAt(idx);
        }
    }
    void removeColumnAt(int idx){
       if(idx >= 0){
           if(m_headLine){
                m_headLine->removeAt0(idx);
           }
           m_delegate->removeColumn(idx);
       }
    }
    sk_sp<IColumn<sk_sp<ListS>>> getColumns(ListS* col_names, bool copy);

    int getRowIdx(CString columnName, CString val);

    inline bool get(CString columnName, int rowIndex, String& out)const{
        int col_idx = getColumnIndex(columnName);
        if(col_idx < 0){
            PRINTLN("column not exists. name = %s\n", columnName.c_str());
            return false;
        }
        out = get(rowIndex, col_idx);
        return true;
    }
    inline sk_sp<IColumn<int>> getIntColumn(CString columnName){
        int col_idx = getColumnIndex(columnName);
        if(col_idx < 0){
            PRINTLN("column not exists. name = %s\n", columnName.c_str());
            return nullptr;
        }
        return m_delegate->getIntColumn(col_idx);
    }
    inline sk_sp<IColumn<int>> getIntColumn(int col_idx){
        return m_delegate->getIntColumn(col_idx);
    }
    inline sk_sp<SetColumn<String>> getColumnAsSet(int index){
        return m_delegate->getColumnAsSet(index);
    }
    inline sk_sp<SetColumn<String>> getColumnAsSet(CString columnName){
        int col_idx = getColumnIndex(columnName);
        if(col_idx < 0){
            PRINTLN("column not exists. name = %s\n", columnName.c_str());
            return nullptr;
        }
        return m_delegate->getColumnAsSet(col_idx);
    }
    inline sk_sp<ListS> getColumnAsUnique(int col_idx){
        return m_delegate->getColumnAsUnique(col_idx);
    }
    inline sk_sp<ListS> getColumnAsUnique(CString columnName){
        int col_idx = getColumnIndex(columnName);
        if(col_idx < 0){
            PRINTLN("getColumnAsUnique >> column not exists. name = %s\n", columnName.c_str());
            return nullptr;
        }
        return m_delegate->getColumnAsUnique(col_idx);
    }
    bool readWriteColumn(CString columnName, std::function<void(String&,int)> visitor);

    void removeNotExist(CString col_name, SetColumn<String>* valueList);
    void removeNotExist(CString col_name, ListS* valueList);

    void removeIf(std::function<bool(sk_sp<ListS>, int)> func);
    void removeByFlags(ListI* );
    void removeByFlags(ListB* );

    sk_sp<TabDelegate2> toTabDelegate2();

    void addColumnsByOPColumns(ListS* newTags, ListS* existTags,
                              std::function<void(ListS&,int,sk_sp<ListS>&)> visitor);

    void addColumnsByOPColumns_OMP(ListS* newTags, ListS* existTags,
                              std::function<void(ListS&,int,sk_sp<ListS>&)> visitor);

    inline void addColumnsByOPColumns(sk_sp<ListS> newTags, sk_sp<ListS> existTags,
                               std::function<void(ListS&,int,sk_sp<ListS>&)> visitor){
        addColumnsByOPColumns(newTags.get(), existTags.get(), visitor);
    }

    void addColumnByOPColumns(CString newTag, h7::IColumn<String>* existTags,
                              std::function<String(h7::IColumn<String>&,int)> visitor);

    void addColumnByOPColumn(CString newTag, CString existTag,
                              std::function<String(String&,int)> visitor);

    bool filterNonContainsSelf(CString refColumnName, SetColumn<String>* valueList);

    //omp default.
    void filterRowSelfByColumnIndexes(sk_sp<ListI> columns, std::function<bool(ListS*)> func);

    void filterRowSelfByColumnIndexes_OMP(sk_sp<ListI> columns, std::function<bool(ListS*)> func);
    //omp default.
    void filterRowSelfByColumnIndexes_memPerf(sk_sp<ListI> columns,
                                             std::function<bool(ListS*)> func);

    void filterRowSelf(ListS* columns, std::function<bool(ListS*)> func);
    void filterRowSelf_OMP(ListS* columns, std::function<bool(ListS*)> func);
    void filterRowSelf_memPerf(ListS* columns, std::function<bool(ListS*)> func);

    void filterRowSelf(CString colName, std::function<bool(String&,int)> func);
    void filterRowByColumnIndex(int index, std::function<bool(String&,int)> func);

    void filterRowSelf(sk_sp<ListS> columns,
                       std::function<bool(ListS*)> func){
        return filterRowSelf(columns.get(), func);
    }
    void filterRowSelf_OMP(sk_sp<ListS> columns,
                       std::function<bool(ListS*)> func){
        return filterRowSelf_OMP(columns.get(), func);
    }
    void filterRowSelf_memPerf(sk_sp<ListS> columns,
                       std::function<bool(ListS*)> func){
        return filterRowSelf_memPerf(columns.get(), func);
    }

    bool sort(CString name, bool desc);
    bool sortFunc(CString name, std::function<int(String&,String&)> func);

    void sortRows(LSortItems items);
    //---------------
    void swapColumn(CString name1, CString name2);
    //----------------------

    void removeDuplicate(CString columnName);
    void removeDuplicate(ListS* columnNames);
    void removeDuplicate_memPerf(ListS* columnNames);
    sk_sp<ListS> getColumnByFilter(CString columnName, ListS* param,
                                   std::function<bool(ListS&,int)> visitor);
    sk_sp<IColumn<sk_sp<Table>>> splitByRowCount(int rowCount, bool onlyRead);

    void addColumn(CString name, sk_sp<ListS> values, bool check = true);

    template<typename E>
    void addColumns(ListS* names, std::function<sk_sp<IColumn<E>>(String&)> visitor,
                            std::function<String(E&)> func_toString){
        int rc = getRowCount();
        int size = names->size();
        for(int j = 0 ; j < size ; j ++){
           sk_sp<IColumn<E>> tList = visitor(names->get(j));
           MED_ASSERT(tList->size() == rc);
           if(func_toString){
               sk_sp<ListS> ret = sk_make_sp<ListS>();
               for(int i = 0 ; i < rc ; i ++){
                   ret->add(func_toString(tList->get(i)));
               }
               m_delegate->addColumnAsFull(ret);
           }else{
               m_delegate->addColumnAsFull(tList->toStrs());
           }
        }
        getHeadLine()->addAll(names);
    }
    //---------------------------------------
    //pre and collector can be empty
    template<typename E>
    void visitRowsWithCollect0(ListI* column_idxes,
                               std::function<E(ListS&, int)> visitor,
                               std::function<bool(E&)> pre,
                               std::function<void(E&)> collector){
         //
         int rc = getRowCount();
         for(int i = 0 ; i < rc ; i ++){
             h7::IColumn<String> list_temp;
             m_delegate->get(i, column_idxes, list_temp);
             E e = visitor(list_temp, i);
             if(pre && !pre(e)){
                // continue;
             }else if(collector){
                 collector(e);
             }
         }
    }
    template<typename E>
    sk_sp<IColumn<E>> visitRows(ListS* names, std::function<E(ListS&, int)> visitor,
                                std::function<bool(E&)> pre){
        ListI indexes;
        requireIndexes(names, indexes);
        sk_sp<IColumn<E>> sp = sk_make_sp<IColumn<E>>(getRowCount());
        visitRowsWithCollect0<E>(&indexes, visitor, pre, [sp](E& e){
            sp->add(e);
        });
        return sp;
    }
    template<typename E>
    sk_sp<IColumn<E>> visitRows(sk_sp<ListS> names,
                                       std::function<E(ListS&, int)> visitor){
        std::function<bool(E&)> pre;
        return visitRows<E>(names.get(), visitor, pre);
    }
    template<typename E>
    sk_sp<IColumn<E>> visitRows_OMP(sk_sp<ListS> names,
                                       std::function<E(ListS&, int)> visitor){
        std::function<bool(E&)> pre;
        return visitRowsSP_OMP<E>(names, visitor, pre);
    }

    template<typename E>
    sk_sp<IColumn<E>> visitRowsSP(sk_sp<ListS> names,
                                       std::function<E(ListS&, int)> visitor,
                                       std::function<bool(E&)> pre){
        return visitRows(names.get(), visitor, pre);
    }

    template<typename E>
    sk_sp<IColumn<E>> visitRowsSP_OMP(sk_sp<ListS> names,
                                       std::function<E(ListS&, int)> visitor,
                                       std::function<bool(E&)> pre);

    //------------------------- char as 3 state: true/false/NA ----------------------------
    sk_sp<ListCh> visitRowStates2(ListS* names,
                                       std::function<char(ListS&, int)> visitor);
    sk_sp<ListCh> visitRowStates2(CString name,
                                       std::function<char(String&, int)> visitor);
    inline sk_sp<ListCh> visitRowStates2(sk_sp<ListS> names,
                                         std::function<char(ListS&, int)> visitor){
        return visitRowStates2(names.get(), visitor);
    }

    //-----------------------------------------------

    //columnNames: nullable
    sk_sp<IColumn<bool>> visitRowStates(ListS* columnNames,
                                        std::function<bool(ListS&, int)> visitor);
    sk_sp<IColumn<bool>> visitRowStates(CString columnName,
                                        std::function<bool(String&, int)> visitor);

    void visitRows2(ListI* colIdxes, std::function<void(ListS&, int)> visitor);
    void visitRows2(ListS* columnNames, std::function<void(ListS&, int)> visitor);

    void visitRows2OMP(ListI* colIdxes, std::function<void(ListS&, int)> visitor,
                       ListI* rowIdxes = nullptr);

    inline void visitRows2OMP(ListS* columnNames, std::function<void(ListS&, int)> visitor,
                              ListI* rowIdxes = nullptr){
        ListI idxes;
        requireIndexes(columnNames, idxes);
        visitRows2OMP(&idxes, visitor, rowIdxes);
    }
    inline void visitRows2(sk_sp<ListS> columnNames,
                           std::function<void(ListS&, int)> visitor){
        return visitRows2(columnNames.get(), visitor);
    }
    inline void visitRows2(sk_sp<ListI> colIdxes, std::function<void(ListS&, int)> visitor){
        return visitRows2(colIdxes.get(), visitor);
    }
    inline void visitRows2OMP(sk_sp<ListI> colIdxes, std::function<void(ListS&, int)> visitor,
                              ListI* rowIdxes = nullptr){
        return visitRows2OMP(colIdxes.get(), visitor, rowIdxes);
    }
    inline void visitRows2OMP(sk_sp<ListS> columnNames,
                           std::function<void(ListS&, int)> visitor,
                              ListI* rowIdxes = nullptr){
        return visitRows2OMP(columnNames.get(), visitor, rowIdxes);
    }

    sk_sp<Table> remapTable(ListS* pNames, ListS* outNames,
                            std::function<bool(ListS&, int)> visitor);
    sk_sp<Table> remapTable(ListS* outNames, bool copy);
    //
    inline sk_sp<Table> remapTable(sk_sp<ListS> pNames, sk_sp<ListS> outNames,
                            std::function<bool(ListS&, int)> visitor){
        return remapTable(pNames.get(), outNames.get(), visitor);
    }
    inline sk_sp<Table> remapTable(sk_sp<ListS> pNames, bool copy){
        return remapTable(pNames.get(), copy);
    }

    sk_sp<Table> getTableByNonColumnName(SetColumn<String>* unNames, bool copy);
    sk_sp<Table> getTableByNonColumnName(ListS* unNames, bool copy);

    template<typename E>
    sk_sp<IColumn<E>> mapColumn(CString colName, std::function<E(String&)> func){
        int col_idx = getColumnIndex(colName);
        if(col_idx < 0){
            PRINTLN("mapColumn >> column not exists. name = %s\n", colName.c_str());
            return nullptr;
        }
        int rc = getRowCount();
        //
        sk_sp<IColumn<E>> sp = sk_make_sp<IColumn<E>>();
        //String str;
        for(int i = 0 ; i < rc ; i ++){
           // str = m_delegate->get(i, col_idx);
            sp->add(func(m_delegate->get(i, col_idx)));
        }
        return sp;
    }
    template<typename E>
    inline void visitGroup(std::function<void(ListS*, String&)> group,
                    std::function<E(const String&, sk_sp<GroupS>&)> visitor,
                    std::map<String, E>& outMap){
        m_delegate->visitGroup<E>(group, visitor, outMap);
    }
    template<typename E>
    inline void visitGroup_big(std::function<void(ListS*, String&)> group,
                    std::function<E(const String&, sk_sp<GroupS>&)> visitor,
                    std::map<String, E>& outMap, int size){
        m_delegate->visitGroup2<E>(size, group, visitor, outMap);
    }

    template<typename E>
    void visitGroupByColumn(CString colName, std::function<E(const String&, sk_sp<GroupS>&)> visitor,
                    std::unordered_map<String, E>& outMap){
        int col_idx = getColumnIndex(colName);
        if(col_idx < 0){
            PRINTLN("visitGroupByColumn >> column not exists. name = %s\n", colName.c_str());
            return;
        }
        m_delegate->visitGroup<E>([col_idx](ListS* l, String& key){
            key = l->get(col_idx);
        }, visitor, outMap);
    }
    //row_indexes can be null.
    sk_sp<Table> subTable(ListI* row_indexes, ListI* col_indexes);
    sk_sp<Table> subTable(ListI* row_indexes, bool non_row_id= false);
    inline sk_sp<Table> subTable(sk_sp<ListI> row_indexes, bool non_row_id= false){
        return subTable(row_indexes.get(), non_row_id);
    }
    //filter and return raw cols.
    sk_sp<Table> filterRaw(ListS* colNames, std::function<bool(ListS*)> visitor);
    //if assign colNames, return the assigned cols.
    sk_sp<Table> filter(ListS* colNames, std::function<bool(ListS*)> visitor);
    inline sk_sp<Table> filter(sk_sp<ListS> colNames, std::function<bool(ListS*)> visitor){
        return filter(colNames.get(), visitor);
    }

    void unique();

    sk_sp<ListS> concatColumns(CString seq, ListS* colNames);
    inline sk_sp<ListS> concatColumns(CString seq, sk_sp<ListS> colNames){
        return concatColumns(seq, colNames.get());
    }
    int findFirstIndex(ListS* colNames, std::function<bool(int, ListS*)> func);

    inline int findFirstIndex(CString colName, std::function<bool(int, ListS*)> func){
        ListS list;
        list.add(colName);
        return findFirstIndex(&list, func);
    }
    sk_sp<ListS> findFirst(ListS* in_names, ListS* out_names,
                           std::function<bool(int, ListS*)> func);
    inline sk_sp<ListS> findFirst(sk_sp<ListS> in_names, sk_sp<ListS> out_names,
                           std::function<bool(int, ListS*)> func){
        return findFirst(in_names.get(), out_names.get(), func);
    }

    sk_sp<GRanges> newGranges(ListS* colNames, std::function<void(int, ListS*,
                                                     GRanges*)> func);
    sk_sp<GRanges> newGranges(CString colName, std::function<void(int, String&,
                                                     GRanges*)> func);
    //one row to more by a col
    void splitByColumn(CString colName, std::function<sk_sp<ListS>(String&, int)> func);

    //row count not change.
    void group(CString colName, h7::HashMap<String, GroupS>& out);
    void group(CString colName, ListS* o_keys, h7::IColumn<GroupS>* o_vals);
    sk_sp<h7::IColumn<sk_sp<Table>>> group(CString colName);

    //-------------------- r methods ----------------------
    sk_sp<ListS> rMatchResult(CString outName, ListI* indexes, CString defVal = "");
    sk_sp<GroupS> rMatchResult(ListS* outNames, ListI* indexes, CString defVal = "");
    sk_sp<Table> rMatchResultTable(ListS* outNames, ListI* indexes, CString defVal = "");
    //
    inline sk_sp<ListS> rMatchResult(CString outName, sk_sp<ListI> indexes, CString defVal = ""){
        return rMatchResult(outName, indexes.get(), defVal);
    }
    inline sk_sp<GroupS> rMatchResult(ListS* outNames,sk_sp<ListI> indexes,
                                     CString defVal = ""){
        return rMatchResult(outNames, indexes.get(), defVal);
    }
    inline sk_sp<Table> rMatchResultTable(ListS* outNames, sk_sp<ListI> indexes,
                                   CString defVal = ""){
        return rMatchResultTable(outNames, indexes.get(), defVal);
    }

    sk_sp<ListS> rMatch(CString colName, ListS* valueList, CString outColName);
    sk_sp<GroupS> rMatch(CString colName, ListS* valueList, ListS* outColNames);
    sk_sp<ListS> rMatchReversed(CString colName, ListS* valueList, CString outColName);
    inline sk_sp<ListS> rMatchReversed(CString colName, sk_sp<ListS> valueList,
                                      CString outColName){
        return rMatchReversed(colName, valueList.get(), outColName);
    }

    inline sk_sp<ListS> rMatchReversed0(ListS* valueList){
        return rMatchReversed(m_headLine->get(0), valueList, m_headLine->get(0));
    }

    sk_sp<Table> rStrsplit_rbind(CString colName, CString seq);

    void rCbindRow(ListS* colNames, TabContentDelegate* rowData);
    inline void rCbindRow(ListS* colNames, sk_sp<TabContentDelegate> rowData){
        rCbindRow(colNames, rowData.get());
    }
    inline void rCbindRow(ListS* colNames, Table* rowData){
        rCbindRow(colNames, rowData->m_delegate.get());
    }
    inline void rCbindRow(ListS* colNames, sk_sp<Table> rowData){
        rCbindRow(colNames, rowData.get());
    }
    inline void rRbindRow(TabContentDelegate* rowData){
        MED_ASSERT(getColumnCount() == rowData->getColumnCount());
        m_delegate->addContentByRow(rowData);
    }
    inline void rRbindRow(sk_sp<TabContentDelegate> rowData){
        MED_ASSERT(getColumnCount() == rowData->getColumnCount());
        m_delegate->addContentByRow(rowData);
    }
    inline void rRbindRow(Table* rowData){
        MED_ASSERT(getColumnCount() == rowData->getColumnCount());
        m_delegate->addContentByRow(rowData->m_delegate);
    }
    inline void rRbindRow(sk_sp<Table> rowData){
        MED_ASSERT(getColumnCount() == rowData->getColumnCount());
        m_delegate->addContentByRow(rowData->m_delegate);
    }

    sk_sp<ListI> rWhich(ListS* colNames, std::function<bool(ListS&, int)> which){
        return visitRows<int>(colNames, [which](ListS& strs, int _index){
            return which(strs, _index) ? _index : -1;
        }, [](int& val){
            return val >= 0;
        });
    }
    inline sk_sp<ListI> rWhich(CString colName, std::function<bool(ListS&, int)> which){
        ListS list;
        list.add(colName);
        return rWhich(&list, which);
    }
    inline sk_sp<ListI> rWhich(sk_sp<ListS> colNames, std::function<bool(ListS&, int)> which){
        return rWhich(colNames.get(), which);
    }
    //like rWhich. but if any matches.
    bool rWhichAny(ListS* colNames, std::function<bool(ListS&, int)> which);

    inline bool rWhichAny(sk_sp<ListS> colNames, std::function<bool(ListS&, int)> which){
        return rWhichAny(colNames.get(), which);
    }

    bool rGreplAny(CString colName, std::regex& pat);
    inline bool rGreplAny(CString colName, CString pat){
        std::regex reg(pat.c_str());
        return rGreplAny(colName, reg);
    }
    template<typename E>
    inline sk_sp<IColumn<E>> rApplyRow(ListS* names, std::function<E(ListS&, int)> visitor,
                                std::function<bool(E&)> pre){
        return visitRows(names, visitor, pre);
    }
    template<typename E>
    inline sk_sp<IColumn<E>> rApplyRow(CString name, std::function<E(ListS&, int)> visitor,
                                std::function<bool(E&)> pre){
        ListS ls{name};
        return visitRows(&ls, visitor, pre);
    }
    template<typename E>
    inline sk_sp<IColumn<E>> rApplyRow(CString name, std::function<E(ListS&, int)> visitor){
        ListS ls{name};
        std::function<bool(E&)> pre;
        return visitRows(&ls, visitor, pre);
    }

    void rSplit(CString outColName, ListS* combinedColumns,
                std::map<String,sk_sp<ListS>>& outMap, CString sep = ".");

    void rSplitOMP(CString outColName, ListS* combinedColumns,
                std::unordered_map<String,sk_sp<ListS>>& outMap, CString sep = ".");

    void rSplitOMP(CString outColName, sk_sp<ListS> combinedColumns,
                   std::unordered_map<String,sk_sp<ListS>>& outMap, CString sep = "."){
        rSplitOMP(outColName, combinedColumns.get(), outMap, sep);
    }

    inline void rSplit(CString outColName, sk_sp<ListS> combinedColumns,
                std::map<String,sk_sp<ListS>>& outMap, CString sep = "."){
        return rSplit(outColName, combinedColumns.get(), outMap, sep);
    }

    IColumn<sk_sp<Table>> rSplit(std::function<void(ListS*, String&)> group,
                                 ListS* outNames, int init_size = 32);

    IColumn<sk_sp<Table>> rSplit2(CString groupColumnName, ListS* outNames);
    inline IColumn<sk_sp<Table>> rSplit2(CString groupColumnName, sk_sp<ListS> outNames){
        return rSplit2(groupColumnName, outNames.get());
    }

    //outIdxes: can be null
    sk_sp<IColumn<PairSI>> rTable(CString colName, std::function<bool(PairSI&,int)> pre,
                                  IColumn<sk_sp<ListI>>* outIdxes);

    sk_sp<ListS> rTableKeys(CString colName, std::function<bool(PairSI&,int)> pre,
                                  IColumn<sk_sp<ListI>>* outIdxes);

    //----------------------------------------
    Table& operator=(Table&& oth){
        this->m_name = oth.m_name;
        this->m_headLine = oth.m_headLine;
        this->m_delegate = oth.m_delegate;
        return *this;
    }
    Table& operator=(const Table& oth){
        this->m_name = oth.m_name;
        this->m_headLine = oth.m_headLine;
        this->m_delegate = oth.m_delegate;
        return *this;
    }
    sk_sp<ListS> operator[](int rowId){
        return m_delegate->getRow(rowId);
    }
private:
    String m_name;
    sk_sp<IColumn<String>> m_headLine;
    sk_sp<TabContentDelegate> m_delegate;
};

//-------------------------------
template<typename E>
sk_sp<IColumn<E>> Table::visitRowsSP_OMP(sk_sp<ListS> names,
                                   std::function<E(ListS&, int)> visitor,
                                   std::function<bool(E&)> pre){
    ListI indexes;
    requireIndexes(names.get(), indexes);
    const int rc = getRowCount();
    std::vector<std::vector<E>> vec;
    vec.resize(rc);
#pragma omp parallel for
    for(int i = 0 ; i < rc ; i ++){
        h7::IColumn<String> list_temp;
        m_delegate->get(i, &indexes, list_temp);
        E e = visitor(list_temp, i);
        if(pre && !pre(e)){
           // continue;
        }else {
            //collector(e);
            vec[i].push_back(std::move(e));
        }
    }
    sk_sp<IColumn<E>> sp = sk_make_sp<IColumn<E>>(rc);
    for(int i = 0 ; i < rc ; ++i){
        if(!vec[i].empty()){
            sp->add(vec[i][0]);
        }
    }
    return sp;
}
}

#endif // TABLE_H
