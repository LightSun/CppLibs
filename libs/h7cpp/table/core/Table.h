#pragma once

#include "common/common.h"
#include <regex>
#include <unordered_map>
#include "common/SkRefCnt.h"
#include "table/core/TabContentDelegate.h"
#include "table/core/GRanges.h"
#include "table/SetColumn.h"
#include "utils/Pair.h"

namespace h7 {

template <typename T>
class SetColumn;

class TabDelegate2;

class Table : public SkRefCnt{
public:
    typedef Pair<String, int> PairSI;
    //----------
    Table();
    Table(sk_sp<GroupS> data);
    Table(sk_sp<ListS> headLine, sk_sp<GroupS> data);
    Table(const GroupS& data);
    Table(const Table& tab);
    Table(CString m_name, sk_sp<ListS> m_headLine,
          sk_sp<TabContentDelegate> m_delegate);
    Table(sk_sp<ListS> m_headLine, sk_sp<TabContentDelegate> m_delegate);
    Table(sk_sp<ListS> m_headLine);
    Table(sk_sp<TabContentDelegate> m_delegate);
    //
    sk_sp<TabContentDelegate> getDelegate()const{return m_delegate;}
    sk_sp<Table> copy();
    sk_sp<TabDelegate2> toTabDelegate2();

    int getRowCount() {return m_delegate->getRowCount();}
    int getColumnCount() {return m_delegate->getColumnCount();}
    void prepareRowCount(int c){m_delegate->prepareRowCount(c);}
    void prepareColumnCount(int c){ m_delegate->prepareColumnCount(c);}

    void setName(CString name){this->m_name = name;}
    String& getName(){return m_name;}

    sk_sp<IColumn<String>> getHeadLine(){return m_headLine;}
    bool hasHeadLine(){return m_headLine && m_headLine->size() > 0;}
    void setHeadLine(sk_sp<IColumn<String>> col){m_headLine = col;}

    int requireColumn(CString name);
    bool hasColumn(CString name){return m_headLine && m_headLine->indexOf(name) >=0 ;}
    int getColumnIndex(CString name)const{return m_headLine->indexOf(name);}

    void getContents(GroupS& out){return m_delegate->getRows(out);}
    void setContents(sk_sp<GroupS> out){m_delegate->setRows(out);}
    void setContents(const GroupS& out){m_delegate->setRows(out);}
    bool hasContent(){return m_delegate->getRowCount() > 0;}

    void mergeTableByColumn(Table* table);
    void mergeTableByColumn(sk_sp<Table> table){mergeTableByColumn(table.get());}
    void mergeTableByRow(Table* table);
    void mergeTableByRow(sk_sp<Table> table){mergeTableByRow(table.get());}

    void setColumns(const std::vector<sk_sp<ListS>>& vecs);
    void prepareColumns(sk_sp<ListS> names, CString defVal);
//------------------------------------------------
    void alignByNames(ListS* names);
    void align(int expectColumnCount = -1){m_delegate->align(expectColumnCount);}

    int getRowIdx(CString columnName, CString val);
    sk_sp<ListS> getRow(int index){return m_delegate->getRow(index);}
    String getRowToStr(int index, CString seq){return m_delegate->getRowToStr(index, seq);}
    void addRowAsFull(sk_sp<ListS> sp){m_delegate->addRowAsFull(sp.get());}
    void addRowAsFull(ListS& list){m_delegate->addRowAsFull(&list);}
    void addRowAsFull(const ListS& list);
    void addRowAsFull(std::vector<String>& list){m_delegate->addRowAsFull(list);}
    void addRowAsFullDirect(std::vector<String>& list){m_delegate->addRowAsFullDirect(list);}
    sk_sp<ListS> get(int index, IColumn<int>* col_idxes){return m_delegate->get(index, col_idxes);}
    String& get(int row_idx, int col_idx)const{ return m_delegate->get(row_idx, col_idx);}
    bool get(CString columnName, int rowIndex, String& out)const;

    //col_names: null means empty out.with all columns
    void requireIndexes(ListS* col_names, ListI& out);
    sk_sp<ListI> getColumnIndexes(sk_sp<ListS> names);
    void getByColumIndexes(int row_index, ListI* col_idxes, ListS& out);
    sk_sp<ListS> getByColumnNames(int row_index, ListS* outColumnNames);
    sk_sp<ListS> getColumnData(ListI* row_indexes, int column_index,
                                      bool copy = false);
    sk_sp<ListS> getColumn(CString columnName, bool copy);
    sk_sp<ListS> getColumn(int index, bool copy){ return m_delegate->getColumn(index, copy);}
    sk_sp<IColumn<sk_sp<ListS>>> getColumns(ListS* col_names, bool copy);

    bool updateColumnName(CString oldName, CString newName);
    bool updateColumnNameStartWith(CString prefix, CString newName);

    sk_sp<ListS> optColumn(CString columnName, bool copy);
    void removeColumn(CString columnName);
    void removeColumnAt(int idx);

    sk_sp<IColumn<int>> getIntColumn(CString columnName);
    sk_sp<IColumn<int>> getIntColumn(int col_idx);
    sk_sp<SetColumn<String>> getColumnAsSet(int index);
    sk_sp<SetColumn<String>> getColumnAsSet(CString columnName);
    sk_sp<ListS> getColumnAsUnique(int col_idx);
    sk_sp<ListS> getColumnAsUnique(CString columnName);
    bool readWriteColumn(CString columnName, std::function<void(String&,int)> visitor);

    void removeIf(std::function<bool(sk_sp<ListS>, int)> func);
    void removeNotExist(CString col_name, SetColumn<String>* valueList);
    void removeNotExist(CString col_name, ListS* valueList);
    void removeByFlags(ListI* );
    void removeByFlags(ListB* );

    void addColumnsByOPColumns(ListS* newTags, ListS* existTags,
                              std::function<void(ListS&,int,sk_sp<ListS>&)> visitor);
    void addColumnsByOPColumns_OMP(ListS* newTags, ListS* existTags,
                         std::function<void(ListS&,int,sk_sp<ListS>&)> visitor);
    void addColumnsByOPColumns(sk_sp<ListS> newTags, sk_sp<ListS> existTags,
                         std::function<void(ListS&,int,sk_sp<ListS>&)> visitor);
    void addColumnByOPColumns(CString newTag, h7::IColumn<String>* existTags,
                              std::function<String(h7::IColumn<String>&,int)> visitor);
    void addColumnByOPColumn(CString newTag, CString existTag,
                              std::function<String(String&,int)> visitor);

    bool filterNonContainsSelf(CString refColumnName, SetColumn<String>* valueList);
    void filterRowSelfByColumnIndexes(sk_sp<ListI> columns, std::function<bool(ListS*)> func);
    void filterRowSelfByColumnIndexes_OMP(sk_sp<ListI> columns, std::function<bool(ListS*)> func);
    void filterRowSelf(ListS* columns, std::function<bool(ListS*)> func);
    void filterRowSelf_OMP(ListS* columns, std::function<bool(ListS*)> func);
    void filterRowSelf(CString colName, std::function<bool(String&,int)> func);
    void filterRowByColumnIndex(int index, std::function<bool(String&,int)> func);

    void filterRowSelf(sk_sp<ListS> columns,
                       std::function<bool(ListS*)> func);
    void filterRowSelf_OMP(sk_sp<ListS> columns,
                       std::function<bool(ListS*)> func);
    bool sort(CString name, bool desc);
    bool sortFunc(CString name, std::function<int(String&,String&)> func);

    //---------------
    void swapColumn(CString name1, CString name2);
    //----------------------
    void unique();
    void removeDuplicate(CString columnName);
    void removeDuplicate(ListS* columnNames);
    void removeDuplicate_memPerf(ListS* columnNames);
    sk_sp<ListS> getColumnByFilter(CString columnName, ListS* param,
                                   std::function<bool(ListS&,int)> visitor);
    sk_sp<IColumn<sk_sp<Table>>> splitByRowCount(int rowCount, bool onlyRead);

    void addColumn(CString name, sk_sp<ListS> values, bool check = true);

    template<typename E>
    void addColumns(ListS* names, std::function<sk_sp<IColumn<E>>(String&)> visitor,
                            std::function<String(E&)> func_toString);
    //---------------------------------------
    //pre and collector can be empty
    template<typename E>
    void visitRowsWithCollect0(ListI* column_idxes,
                               std::function<E(ListS&, int)> visitor,
                               std::function<bool(E&)> pre,
                               std::function<void(E&)> collector);
    template<typename E>
    sk_sp<IColumn<E>> visitRows(sk_sp<ListS> names,
                                  std::function<E(ListS&, int)> visitor,
                                       std::function<bool(E&)> pre);
    template<typename E>
    sk_sp<IColumn<E>> visitRows(ListS* names, std::function<E(ListS&, int)> visitor,
                                std::function<bool(E&)> pre);
    template<typename E>
    sk_sp<IColumn<E>> visitRows(sk_sp<ListS> names,
                                std::function<E(ListS&, int)> visitor);
    template<typename E>
    sk_sp<IColumn<E>> visitRows_OMP(sk_sp<ListS> names,
                                       std::function<E(ListS&, int)> visitor);
    template<typename E>
    sk_sp<IColumn<E>> visitRows_OMP(sk_sp<ListS> names,
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

    void visitRows2OMP(ListS* columnNames, std::function<void(ListS&, int)> visitor,
                              ListI* rowIdxes = nullptr);
    void visitRows2(sk_sp<ListS> columnNames, std::function<void(ListS&, int)> visitor);
    void visitRows2(sk_sp<ListI> colIdxes, std::function<void(ListS&, int)> visitor);
    void visitRows2OMP(sk_sp<ListI> colIdxes, std::function<void(ListS&, int)> visitor,
                              ListI* rowIdxes = nullptr);
    void visitRows2OMP(sk_sp<ListS> columnNames, std::function<void(ListS&, int)> visitor,
                              ListI* rowIdxes = nullptr);

    sk_sp<Table> remapTable(ListS* pNames, ListS* outNames,
                            std::function<bool(ListS&, int)> visitor);
    sk_sp<Table> remapTable(ListS* outNames, bool copy);
    //
    sk_sp<Table> remapTable(sk_sp<ListS> pNames, sk_sp<ListS> outNames,
                            std::function<bool(ListS&, int)> visitor);
    sk_sp<Table> remapTable(sk_sp<ListS> pNames, bool copy);

    sk_sp<Table> getTableByNonColumnName(SetColumn<String>* unNames, bool copy);
    sk_sp<Table> getTableByNonColumnName(ListS* unNames, bool copy);

    template<typename E>
    sk_sp<IColumn<E>> mapColumn(CString colName, std::function<E(String&)> func);

    //row_indexes can be null.
    sk_sp<Table> subTable(ListI* row_indexes, ListI* col_indexes);
    sk_sp<Table> subTable(ListI* row_indexes, bool non_row_id= false);
    sk_sp<Table> subTable(sk_sp<ListI> row_indexes, bool non_row_id= false);

    //if assign colNames, return the assigned cols.
    sk_sp<Table> filter(ListS* colNames, std::function<bool(ListS*)> visitor);
    sk_sp<Table> filter(sk_sp<ListS> colNames, std::function<bool(ListS*)> visitor);

    sk_sp<ListS> concatColumns(CString seq, ListS* colNames);
    sk_sp<ListS> concatColumns(CString seq, sk_sp<ListS> colNames);

    int findFirstIndex(ListS* colNames, std::function<bool(int, ListS*)> func);

    int findFirstIndex(CString colName, std::function<bool(int, ListS*)> func);
    sk_sp<ListS> findFirst(ListS* in_names, ListS* out_names,
                           std::function<bool(int, ListS*)> func);
    sk_sp<ListS> findFirst(sk_sp<ListS> in_names, sk_sp<ListS> out_names,
                           std::function<bool(int, ListS*)> func);

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
    sk_sp<ListS> rMatchResult(CString outName, sk_sp<ListI> indexes,
                                     CString defVal = "");
    sk_sp<GroupS> rMatchResult(ListS* outNames,sk_sp<ListI> indexes,
                                     CString defVal = "");
    sk_sp<Table> rMatchResultTable(ListS* outNames, sk_sp<ListI> indexes,
                                   CString defVal = "");

    sk_sp<ListS> rMatch(CString colName, ListS* valueList, CString outColName);
    sk_sp<GroupS> rMatch(CString colName, ListS* valueList, ListS* outColNames);
    sk_sp<ListS> rMatchReversed(CString colName, ListS* valueList, CString outColName);
    sk_sp<ListS> rMatchReversed(CString colName, sk_sp<ListS> valueList,
                                      CString outColName);
    sk_sp<ListS> rMatchReversed0(ListS* valueList);

    sk_sp<Table> rStrsplit_rbind(CString colName, CString seq);

    void rCbindRow(ListS* colNames, TabContentDelegate* rowData);
    void rCbindRow(ListS* colNames, sk_sp<TabContentDelegate> rowData);
    void rCbindRow(ListS* colNames, Table* rowData);
    void rCbindRow(ListS* colNames, sk_sp<Table> rowData);
    void rRbindRow(TabContentDelegate* rowData);
    void rRbindRow(sk_sp<TabContentDelegate> rowData);
    void rRbindRow(Table* rowData);
    void rRbindRow(sk_sp<Table> rowData);

    sk_sp<ListI> rWhich(ListS* colNames, std::function<bool(ListS&, int)> which);
    sk_sp<ListI> rWhich(CString colName,
                               std::function<bool(ListS&, int)> which);
    sk_sp<ListI> rWhich(sk_sp<ListS> colNames,
                        std::function<bool(ListS&, int)> which);
    //like rWhich. but if any matches.
    bool rWhichAny(ListS* colNames, std::function<bool(ListS&, int)> which);
    bool rWhichAny(sk_sp<ListS> colNames, std::function<bool(ListS&, int)> which);

    bool rGreplAny(CString colName, std::regex& pat);
    bool rGreplAny(CString colName, CString pat);

    template<typename E>
    sk_sp<IColumn<E>> rApplyRow(ListS* names, std::function<E(ListS&, int)> visitor,
                                std::function<bool(E&)> pre);
    template<typename E>
    sk_sp<IColumn<E>> rApplyRow(CString name, std::function<E(ListS&, int)> visitor,
                                std::function<bool(E&)> pre);
    template<typename E>
    sk_sp<IColumn<E>> rApplyRow(CString name, std::function<E(ListS&, int)> visitor);

    void rSplit(CString outColName, ListS* combinedColumns,
                std::map<String,sk_sp<ListS>>& outMap, CString sep = ".");

    void rSplit(CString outColName, sk_sp<ListS> combinedColumns,
                std::map<String,sk_sp<ListS>>& outMap, CString sep = ".");
    //outIdxes: can be null
    sk_sp<IColumn<PairSI>> rTable(CString colName,
                                  std::function<bool(PairSI&,int)> pre,
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

//#include "table/core/Table.hxx"

Table::Table(){
    this->m_headLine = sk_make_sp<ListS>();
    this->m_delegate = sk_make_sp<TabContentDelegate>();
}
Table::Table(sk_sp<GroupS> data){
    this->m_headLine = sk_make_sp<ListS>();
    this->m_delegate = sk_make_sp<TabContentDelegate>(data);
}
Table::Table(sk_sp<ListS> headLine, sk_sp<GroupS> data):m_headLine(headLine){
    this->m_delegate = sk_make_sp<TabContentDelegate>(data);
}
Table::Table(const GroupS& data){
    this->m_headLine = sk_make_sp<ListS>();
    this->m_delegate = sk_make_sp<TabContentDelegate>(data);
}
Table::Table(const Table& tab){
    this->m_name = tab.m_name;
    this->m_headLine = tab.m_headLine;
    this->m_delegate = tab.m_delegate;
}
Table::Table(CString m_name, sk_sp<ListS> m_headLine, sk_sp<TabContentDelegate> m_delegate):
    m_name(m_name), m_headLine(m_headLine), m_delegate(m_delegate){
}
Table::Table(sk_sp<ListS> m_headLine, sk_sp<TabContentDelegate> m_delegate):
    m_headLine(m_headLine), m_delegate(m_delegate){
}
Table::Table(sk_sp<ListS> m_headLine):
    m_headLine(m_headLine){
    this->m_delegate = sk_make_sp<TabContentDelegate>();
}
Table::Table(sk_sp<TabContentDelegate> m_delegate):
    m_delegate(m_delegate){
    this->m_headLine = sk_make_sp<ListS>();
}
//----------------------------
inline int Table::requireColumn(CString name){
    int idx = m_headLine->indexOf(name);
    MED_ASSERT_X(idx >= 0, "can't find column name = " + name);
    return idx;
}
inline void Table::setColumns(const std::vector<sk_sp<ListS>>& vecs){
    m_delegate->setColumns(vecs);
}
inline void Table::mergeTableByRow(Table* table){
    if(table->getRowCount() == 0 || table->getColumnCount() == 0){
        return;
    }
    m_delegate->addContentByRow(table->m_delegate);
}
inline void Table::mergeTableByColumn(Table* table){
    if(table->getRowCount() == 0 || table->getColumnCount() == 0){
        return;
    }
    m_delegate->addContentByColumn(table->m_delegate);
}
inline void Table::addRowAsFull(const ListS& list){
    ListS* ls = (ListS*)&list;
    m_delegate->addRowAsFull(ls);
}
//col_names: null means empty out.with all columns
inline void Table::requireIndexes(ListS* col_names, ListI& out){
    if(col_names && col_names->size() > 0){
        int idx;
        for(int i = 0; i < col_names->size() ; i++){
            idx = requireColumn(col_names->get(i));
            if(idx >= 0){
                out.add(idx);
            }
        }
    }else{
        int size = m_headLine->size();
        for(int i = 0 ; i < size ;i ++){
            out.add(i);
        }
    }
}
inline sk_sp<ListI> Table::getColumnIndexes(sk_sp<ListS> names){
    sk_sp<ListI> li = sk_make_sp<ListI>(names->size());
    requireIndexes(names.get(), *li);
    return li;
}
inline void Table::getByColumIndexes(int row_index, ListI* col_idxes, ListS& out){
    m_delegate->get(row_index, col_idxes, out);
}
inline sk_sp<ListS> Table::getByColumnNames(int row_index, ListS* outColumnNames){
    ListI col_idxes;
    requireIndexes(outColumnNames, col_idxes);
    sk_sp<ListS> out = sk_make_sp<ListS>();
    m_delegate->get(row_index, &col_idxes, *out);
    return out;
}
inline sk_sp<ListS> Table::getColumnData(ListI* row_indexes, int column_index,
                                  bool copy){
    return m_delegate->get(row_indexes, column_index, copy);
}

inline sk_sp<ListS> Table::getColumn(CString columnName, bool copy){
    int idx = requireColumn(columnName);
    return idx >=0 ? getColumn(idx, copy) : nullptr;
}
inline bool Table::updateColumnName(CString oldName, CString newName){
    int idx = m_headLine->indexOf(oldName);
    if(idx >= 0){
        m_headLine->set0(idx, newName);
        return true;
    }
    return false;
}
inline sk_sp<ListS> Table::optColumn(CString columnName, bool copy){
    int idx = m_headLine->indexOf(columnName);
    return idx >=0 ? m_delegate->optColumn(idx, copy) : nullptr;
}
inline void Table::removeColumn(CString columnName){
    int idx = requireColumn(columnName);
    if(idx >= 0){
        removeColumnAt(idx);
    }
}
inline void Table::removeColumnAt(int idx){
   if(idx >= 0){
       if(m_headLine){
            m_headLine->removeAt0(idx);
       }
       m_delegate->removeColumn(idx);
   }
}
inline bool Table::get(CString columnName, int rowIndex, String& out)const{
    int col_idx = getColumnIndex(columnName);
    if(col_idx < 0){
        PRINTLN("column not exists. name = %s\n", columnName.c_str());
        return false;
    }
    out = get(rowIndex, col_idx);
    return true;
}
inline sk_sp<IColumn<int>> Table::getIntColumn(CString columnName){
    int col_idx = getColumnIndex(columnName);
    if(col_idx < 0){
        PRINTLN("column not exists. name = %s\n", columnName.c_str());
        return nullptr;
    }
    return m_delegate->getIntColumn(col_idx);
}
inline sk_sp<IColumn<int>> Table::getIntColumn(int col_idx){
    return m_delegate->getIntColumn(col_idx);
}
inline sk_sp<SetColumn<String>> Table::getColumnAsSet(int index){
    return m_delegate->getColumnAsSet(index);
}
inline sk_sp<SetColumn<String>> Table::getColumnAsSet(CString columnName){
    int col_idx = getColumnIndex(columnName);
    if(col_idx < 0){
        PRINTLN("column not exists. name = %s\n", columnName.c_str());
        return nullptr;
    }
    return m_delegate->getColumnAsSet(col_idx);
}
inline sk_sp<ListS> Table::getColumnAsUnique(int col_idx){
    return m_delegate->getColumnAsUnique(col_idx);
}
inline sk_sp<ListS> Table::getColumnAsUnique(CString columnName){
    int col_idx = getColumnIndex(columnName);
    if(col_idx < 0){
        PRINTLN("getColumnAsUnique >> column not exists. name = %s\n", columnName.c_str());
        return nullptr;
    }
    return m_delegate->getColumnAsUnique(col_idx);
}
inline void Table::addColumnsByOPColumns(sk_sp<ListS> newTags, sk_sp<ListS> existTags,
                           std::function<void(ListS&,int,sk_sp<ListS>&)> visitor){
    addColumnsByOPColumns(newTags.get(), existTags.get(), visitor);
}
//-------------------------

template<typename E>
void Table::addColumns(ListS* names, std::function<sk_sp<IColumn<E>>(String&)> visitor,
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
template<typename E>
void Table::visitRowsWithCollect0(ListI* column_idxes,
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
sk_sp<IColumn<E>> Table::visitRows(ListS* names, std::function<E(ListS&, int)> visitor,
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
sk_sp<IColumn<E>> Table::visitRows(sk_sp<ListS> names,
                                   std::function<E(ListS&, int)> visitor){
    std::function<bool(E&)> pre;
    return visitRows<E>(names.get(), visitor, pre);
}
template<typename E>
sk_sp<IColumn<E>> Table::visitRows_OMP(sk_sp<ListS> names,
                                   std::function<E(ListS&, int)> visitor){
    std::function<bool(E&)> pre;
    return visitRows_OMP<E>(names, visitor, pre);
}
template<typename E>
sk_sp<IColumn<E>> Table::visitRows(sk_sp<ListS> names,
                                   std::function<E(ListS&, int)> visitor,
                                   std::function<bool(E&)> pre){
    return visitRows(names.get(), visitor, pre);
}

template<typename E>
sk_sp<IColumn<E>> Table::visitRows_OMP(sk_sp<ListS> names,
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

template<typename E>
sk_sp<IColumn<E>> Table::mapColumn(CString colName, std::function<E(String&)> func){
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
inline sk_sp<IColumn<E>> Table::rApplyRow(ListS* names, std::function<E(ListS&, int)> visitor,
                            std::function<bool(E&)> pre){
    return visitRows(names, visitor, pre);
}
template<typename E>
inline sk_sp<IColumn<E>> Table::rApplyRow(CString name, std::function<E(ListS&, int)> visitor,
                            std::function<bool(E&)> pre){
    ListS ls{name};
    return visitRows(&ls, visitor, pre);
}
template<typename E>
inline sk_sp<IColumn<E>> Table::rApplyRow(CString name, std::function<E(ListS&, int)> visitor){
    ListS ls{name};
    std::function<bool(E&)> pre;
    return visitRows(&ls, visitor, pre);
}

}
