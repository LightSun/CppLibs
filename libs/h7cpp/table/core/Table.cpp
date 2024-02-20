#include <set>
#include <thread>
//#include <omp.h>
#include "Table.h"
#include "table/SetColumn.h"
#include "utils/collection_utils.h"
#include "utils/string_utils.hpp"
#include "utils/Executor.h"
//#include "utils/BubbleSort.hpp"
#include "TabDelegate2.h"

#define __BLOCK_MAX_COUNT 100000
#define INVALID_INT INT_MIN

namespace h7 {
sk_sp<Table> Table::copy(){
    return sk_make_sp<Table>(this->m_name, m_headLine->copy(), m_delegate->copy());
}

bool Table::updateColumnNameStartWith(CString prefix, CString newName){
    int idx = -1;
    for(int i = 0 ; i < m_headLine->size() ; ++i){
        if(h7::utils::startsWith(m_headLine->get(i), prefix)){
            idx = i;
            break;
        }
    }
    if(idx >= 0){
        m_headLine->set0(idx, newName);
        return true;
    }
    return false;
}
sk_sp<IColumn<sk_sp<ListS>>> Table::getColumns(ListS* col_names, bool copy){
    sk_sp<IColumn<sk_sp<ListS>>> ret = sk_make_sp<IColumn<sk_sp<ListS>>>();
    for(int i = 0; i < col_names->size() ; i++){
        ret->add(m_delegate->getColumn(requireColumn(col_names->get(i)), copy));
    }
    return ret;
}

int Table::getRowIdx(CString columnName, CString val){
    if(getRowCount() == 0){
        return -1;
    }
    auto cols = getColumn(columnName, false);
    return cols->indexOf(val);
}

bool Table::readWriteColumn(CString columnName, std::function<void(String&,int)> visitor){
    const int col_idx = getColumnIndex(columnName);
    if(col_idx < 0){
        PRINTLN("readWriteColumn >> column not exists. name = %s\n", columnName.c_str());
        return false;
    }
    int rc = getRowCount();
    for(int i = 0 ; i < rc ; i ++){
        visitor(m_delegate->get(i, col_idx), i);
    }
    return true;
}

void Table::removeNotExist(CString col_name, h7::SetColumn<String>* valueList){
    int col_idx = getColumnIndex(col_name);
    if(col_idx < 0){
        PRINTLN("removeNotExist >> column not exists. name = %s\n", col_name.c_str());
        return;
    }
    removeIf([col_idx, valueList](sk_sp<ListS> ls, int){
           if(!valueList->contains(ls->get(col_idx))){
               return true;
           }
           return false;
    });
}

void Table::removeNotExist(CString col_name, ListS* valueList){
    //h7::SetColumn<String> set;

    int col_idx = getColumnIndex(col_name);
    if(col_idx < 0){
        PRINTLN("removeNotExist >> column not exists. name = %s\n", col_name.c_str());
        return;
    }
    removeIf([col_idx, valueList](sk_sp<ListS> ls, int){
           if(!valueList->contains(ls->get(col_idx))){
               return true;
           }
           return false;
    });
}

void Table::removeByFlags(ListB* flags){
    GroupS gs;
    getDelegate()->getRows(gs);
    TabDelegate2 td(&gs, __BLOCK_MAX_COUNT);
    td.removeByFlags(flags);
    td.copyToGroup();
    getDelegate()->setRows(gs);
}
void Table::removeByFlags(ListI* flags){
    GroupS gs;
    getDelegate()->getRows(gs);
    TabDelegate2 td(&gs, __BLOCK_MAX_COUNT);
    td.removeByFlags(flags);
    td.copyToGroup();
    getDelegate()->setRows(gs);
}
void Table::removeIf(std::function<bool(sk_sp<ListS>, int)> func){
    GroupS gs;
    getDelegate()->getRows(gs);
    TabDelegate2 td(&gs, __BLOCK_MAX_COUNT);
    td.removeIf(func);
    td.copyToGroup();
    getDelegate()->setRows(gs);
}

sk_sp<TabDelegate2> Table::toTabDelegate2(){
    sk_sp<GroupS> gs = sk_make_sp<GroupS>();
    getDelegate()->getRows(*gs);
    return sk_make_sp<TabDelegate2>(gs, getHeadLine()->copy());
}

//visitor: row, index, col_vals
void Table::addColumnsByOPColumns(ListS* newTags, ListS* existTags,
                           std::function<void(ListS&,int,sk_sp<ListS>&)> visitor){
    MED_ASSERT_X(newTags != nullptr, "newTags can't be null");
    const int cc = getColumnCount();
    const int newSize = newTags->size();
    ListI idxes;
    requireIndexes(existTags, idxes);
    //new columns
    IColumn<sk_sp<ListS>> newColumns;
    for(int i = 0 ; i < newSize ; i ++){
        newColumns.add(sk_make_sp<ListS>(cc));
    }
    //
    const int rc = getRowCount();
    //std::vector<sk_sp<ListS>> sp_temps;
   // sp_temps.resize(rc);
//#pragma omp parallel for
    for(int i = 0 ; i < rc ; ++i ){
        h7::IColumn<String> list_tmp;
        getByColumIndexes(i, &idxes, list_tmp);
        //
        sk_sp<ListS> sp_temp = sk_make_sp<ListS>();
        visitor(list_tmp, i, sp_temp);
        MED_ASSERT(sp_temp->size() == newSize);
        for(int j = 0 ; j < newSize ; ++j ){
            newColumns.get(j)->add(sp_temp->get(j));
        }
    }
    m_headLine->addAll(newTags);
    m_delegate->addColumnsAsFull(newColumns);
}

void Table::addColumnsByOPColumns_OMP(ListS* newTags, ListS* existTags,
                          std::function<void(ListS&,int,sk_sp<ListS>&)> visitor){
    MED_ASSERT_X(newTags != nullptr, "newTags can't be null");
    const int cc = getColumnCount();
    const int newSize = newTags->size();
    ListI idxes;
    requireIndexes(existTags, idxes);
    //new columns
    IColumn<sk_sp<ListS>> newColumns;
    for(int i = 0 ; i < newSize ; i ++){
        newColumns.add(sk_make_sp<ListS>(cc));
    }
    const int rc = getRowCount();
    //
    std::vector<sk_sp<ListS>> sp_temps;
    sp_temps.resize(rc);
#pragma omp parallel for
    for(int i = 0 ; i < rc ; ++i ){
        h7::IColumn<String> list_tmp;
        getByColumIndexes(i, &idxes, list_tmp);
        //
        sk_sp<ListS> sp_temp = sk_make_sp<ListS>();
        visitor(list_tmp, i, sp_temp);
        MED_ASSERT(sp_temp->size() == newSize);
        sp_temps[i] = sp_temp;
    }
    for(int i = 0 ; i < rc ; ++i ){
        auto& sp_temp = sp_temps[i];
        for(int j = 0 ; j < newSize ; ++j ){
            newColumns.get(j)->add(sp_temp->get(j));
        }
    }
    m_headLine->addAll(newTags);
    m_delegate->addColumnsAsFull(newColumns);
}

void Table::addColumnByOPColumns(CString newTag, h7::IColumn<String>* existTags,
                          std::function<String(h7::IColumn<String>&,int)> visitor){
    ListI idxes;
    requireIndexes(existTags, idxes);
    MED_ASSERT(idxes.size() == existTags->size());

    sk_sp<ListS> list = sk_make_sp<ListS>();
    int rc = getRowCount();
    for(int i = 0 ; i < rc ; ++i ){
        h7::IColumn<String> list_tmp;
        getByColumIndexes(i, &idxes, list_tmp);
        list->add(visitor(list_tmp, i));
    }
    m_headLine->add(newTag);
    m_delegate->addColumnAsFull(list);
}

void Table::addColumnByOPColumn(CString newTag, CString existTag,
                          std::function<String(String&,int)> visitor){
    int col_idx = getColumnIndex(existTag);
    if(col_idx < 0){
        PRINTLN("addColumnByOPColumns >> column not exists. name = %s\n", existTag.c_str());
        return;
    }
    int rc = getRowCount();
    sk_sp<ListS> list = sk_make_sp<ListS>(rc, true);
    for(int i = 0 ; i < rc ; ++i){
        list->set0(i, visitor(get(i, col_idx), i));
    }
    m_headLine->add(newTag);
    m_delegate->addColumnAsFull(list);
}

bool Table::filterNonContainsSelf(CString columnName, SetColumn<String>* valueList){
    const int col_idx = getColumnIndex(columnName);
    if(col_idx < 0){
        PRINTLN("filterNonContainsSelf >> column not exists. name = %s\n", columnName.c_str());
        return false;
    }
    removeIf([col_idx, valueList](sk_sp<ListS> l, int){
        return !valueList->contains(l->get(col_idx));
    });
    return true;
}
void Table::filterRowByColumnIndex(int index, std::function<bool(String&,int)> func){
    int rc = getRowCount();
       //
    ListI rm_idxes(rc, true);
    {
       auto col = m_delegate->getColumn(index, false);
       for(int i = 0 ; i < rc ; ++i){
            rm_idxes.set0(i, func(col->get(i), i) ? 1 : 0);
       }
    }
    removeByFlags(&rm_idxes);
}
void Table::filterRowSelf(CString colName, std::function<bool(String&,int)> func){

    int index = requireColumn(colName);
    if(index >= m_delegate->getColumnCount()){
        PRINTLN("WARNING: no data to support column = %s\n", colName.data());
        return;
    }
    filterRowByColumnIndex(index, func);
}
void Table::filterRowSelfByColumnIndexes_OMP(sk_sp<ListI> columns,
                                             std::function<bool(ListS*)> func){
    ListI& indexes= *columns;
    int rc = getRowCount();
    //
    ListI rm_idxes(rc, true);
#pragma omp parallel for
    for(int i = 0 ; i < rc ; ++i){
        h7::IColumn<String> list_tmp;
        getByColumIndexes(i, &indexes, list_tmp);
        if(func(&list_tmp)){
            //rm_idxes.add(i);
            rm_idxes.set0(i, 1);
        }else{
            rm_idxes.set0(i, 0);
        }
    }
    removeByFlags(&rm_idxes);
}
void Table::filterRowSelfByColumnIndexes(sk_sp<ListI> columns,
                                         std::function<bool(ListS*)> func){
    ListI& indexes= *columns;
    int rc = getRowCount();
    //
    ListI rm_idxes(rc, true);
    //#pragma omp parallel for
    for(int i = 0 ; i < rc ; ++i){
     h7::IColumn<String> list_tmp;
     getByColumIndexes(i, &indexes, list_tmp);
     if(func(&list_tmp)){
         //rm_idxes.add(i);
         rm_idxes.set0(i, 1);
     }else{
         rm_idxes.set0(i, 0);
     }
    }
    removeByFlags(&rm_idxes);
}
void Table::filterRowSelf(ListS* columns, std::function<bool(ListS*)> func){
    sk_sp<ListI> cols = sk_make_sp<ListI>();
    requireIndexes(columns, *cols);
    filterRowSelfByColumnIndexes(cols, func);
}

void Table::filterRowSelf_OMP(ListS* columns, std::function<bool(ListS*)> func){
    sk_sp<ListI> cols = sk_make_sp<ListI>();
    requireIndexes(columns, *cols);
    filterRowSelfByColumnIndexes_OMP(cols, func);
}
void Table::filterRowSelf(sk_sp<ListS> columns,
                   std::function<bool(ListS*)> func){
    return filterRowSelf(columns.get(), func);
}
void Table::filterRowSelf_OMP(sk_sp<ListS> columns,
                   std::function<bool(ListS*)> func){
    return filterRowSelf_OMP(columns.get(), func);
}

bool Table::sort(CString name, bool desc){
    int col_idx = getColumnIndex(name);
    if(col_idx < 0){
        PRINTLN("sort >> column not exists. name = %s\n", name.c_str());
        return false;
    }
    if(desc){
        m_delegate->sortRow([col_idx](IColumn<String>* l1, IColumn<String>* l2){
            auto& s1 = l1->get(col_idx);
            auto& s2 = l2->get(col_idx);
            return CMP_DESC(s1, s2);
        });
    }else{
        m_delegate->sortRow([col_idx](IColumn<String>* l1, IColumn<String>* l2){
            auto& s1 = l1->get(col_idx);
            auto& s2 = l2->get(col_idx);
            return CMP_AESC(s1, s2);
        });
    }
    return true;
}

//slow
bool Table::sortFunc(CString name, std::function<int(String&,String&)> func){
    int col_idx = getColumnIndex(name);
    if(col_idx < 0){
        PRINTLN("sort >> column not exists. name = %s\n", name.c_str());
        return false;
    }
    m_delegate->sortRow([col_idx, func](IColumn<String>* l1, IColumn<String>* l2){
        return func(l1->get(col_idx), l2->get(col_idx));
    });
    return true;
}

void Table::swapColumn(CString name1, CString name2){
    int id1 = requireColumn(name1);
    int id2 = requireColumn(name2);
    if(m_headLine){
        m_headLine->swap(id1, id2);
    }
    m_delegate->swapColumn(id1, id2);
}

//-------------------------------------------------
void Table::removeDuplicate(CString columnName){
    int col_idx = getColumnIndex(columnName);
    if(col_idx < 0){
        PRINTLN("removeDuplicate >> column not exists. name = %s\n", columnName.c_str());
        return;
    }
    //
    SetColumn<String> set;
    const int rc = getRowCount();
    ListI list_r;
    list_r.resize(rc);
    //
    GroupS rows;
    m_delegate->getRows(rows);
    for(int i = 0 ; i < rc ; ++i ){
        String& str = rows[i]->get(col_idx);
        if(set.contains(str)){
           list_r.set0(i, 1);
        }else{
           set.add(str);
           list_r.set0(i, 0);
        }
    }
    removeByFlags(&list_r);
}

void Table::removeDuplicate_memPerf(ListS* columnNames){
    MED_ASSERT(columnNames != nullptr);
    ListI indexes;
    requireIndexes(columnNames, indexes);
    //
    const int rc = getRowCount();
    SetColumn<int> set(rc);
    ListI list_r;
    list_r.resize(rc);
    //
    ListI lhash;
    lhash.resize(rc);
#pragma omp parallel for
    for(int i = 0 ; i < rc ; ++i ){
        lhash.set0(i, m_delegate->computeHash(i, &indexes));
    }
    for(int i = 0 ; i < rc ; ++i ){
       // hash = rows[i]->hashCode(&indexes);
        if(set.contains(lhash[i])){
           list_r.set0(i, 1);
        }else{
           set.add(lhash[i]);
           list_r.set0(i, 0);
        }
    }
    removeByFlags(&list_r);
}
void Table::removeDuplicate(ListS* columnNames){
    MED_ASSERT(columnNames != nullptr);
    ListI indexes;
    requireIndexes(columnNames, indexes);
    //
    const int rc = getRowCount();
    SetColumn<int> set(rc);
    ListI list_r;
    list_r.resize(rc);
    //
    ListI lhash;
    lhash.resize(rc);
#pragma omp parallel for
    for(int i = 0 ; i < rc ; ++i ){
        lhash.set0(i, m_delegate->computeHash(i, &indexes));
    }
    for(int i = 0 ; i < rc ; ++i ){
       // hash = rows[i]->hashCode(&indexes);
        if(set.contains(lhash[i])){
           list_r.set0(i, 1);
        }else{
           set.add(lhash[i]);
           list_r.set0(i, 0);
        }
    }
    removeByFlags(&list_r);
}

sk_sp<ListS> Table::getColumnByFilter(CString columnName, ListS* param,
                                      std::function<bool(ListS&, int)> visitor){
    int col_idx = getColumnIndex(columnName);
    if(col_idx < 0){
        PRINTLN("getColumnByFilter >> column not exists. name = %s\n", columnName.c_str());
        return nullptr;
    }
    ListI indexes;
    requireIndexes(param, indexes);
    sk_sp<ListS> result = sk_make_sp<ListS>();
    ListS list(indexes.isEmpty()? m_headLine->size() : indexes.size());
    //
    int rc = getRowCount();
    for(int i = 0 ; i < rc ; ++i ){
         m_delegate->get(i, &indexes, list);
         if(visitor(list, i)){
            result->add(m_delegate->get(i, col_idx));
         }
         list.clear();
    }
    return result;
}

sk_sp<IColumn<sk_sp<Table>>> Table::splitByRowCount(int rowCount, bool onlyRead){
    sk_sp<IColumn<sk_sp<Table>>> ret = sk_make_sp<IColumn<sk_sp<Table>>>();
    int c = getRowCount() / rowCount + (getRowCount() % rowCount != 0 ? 1 : 0);
    if(c == 1){
        if(onlyRead){
            ret->add(sk_ref_sp(this));
        }else{
            ret->add(this->copy());
        }
        return ret;
    }
    //
    int rc = getRowCount();
    if(onlyRead){
        sk_sp<ListS> headLine = m_headLine->copy();
        for(int i = 0 ; i < c ; ++i){
            ret->add(sk_make_sp<Table>(headLine));
        }
    }else{
        for(int i = 0 ; i < c ; ++i){
            ret->add(sk_make_sp<Table>(m_headLine->copy()));
        }
    }
    for(int i = 0 ; i < rc ; ++i){
       // int id = i / rowCount;
       // ret->get(id)->addRowAsFull(m_delegate->getRow(i));
        ret->get(i / rowCount)->addRowAsFull(m_delegate->getRow(i));
    }
    return ret;
}

void Table::addColumn(CString name, sk_sp<ListS> values, bool check){
    if(check){
        int rc = getRowCount();
        if(rc > 0){
            char buf[256];
            snprintf(buf, 256,
        "addColumn(%s) >>rc must == values.size(), rc = %d, c_size = %d",
                     name.data(), getRowCount(), (int)values->size());
            MED_ASSERT_X(rc == values->size(), String(buf));
        }
        //MED_ASSERT_X(getRowCount() > 0, "contents is empty");
    }
    m_headLine->add(name);
    m_delegate->addColumnAsFull(values);
}

sk_sp<IColumn<bool>> Table::visitRowStates(ListS* columnNames, std::function<bool(ListS&, int)> visitor){
    ListI indexes;
    requireIndexes(columnNames, indexes);
    ListS list_temp;
    sk_sp<IColumn<bool>> ret = sk_make_sp<IColumn<bool>>();
    int rc = getRowCount();
    ret->prepareSize(rc);
    for(int i = 0 ; i < rc ; ++i ){
        m_delegate->get(i, &indexes, list_temp);
        ret->add(visitor(list_temp, i));
        list_temp.clear();
    }
    return ret;
}

sk_sp<IColumn<bool>> Table::visitRowStates(CString colName,
                                    std::function<bool(String&, int)> visitor){
    int col_idx = getColumnIndex(colName);
    if(col_idx < 0){
        PRINTLN("visitRowStates >> column not exists. name = %s\n", colName.c_str());
        return nullptr;
    }
    sk_sp<IColumn<bool>> ret = sk_make_sp<IColumn<bool>>();
    int rc = getRowCount();
    ret->prepareSize(rc);
    for(int i = 0 ; i < rc ; ++i){
        ret->add(visitor(m_delegate->get(i, col_idx), i));
    }
    return ret;
}

sk_sp<ListCh> Table::visitRowStates2(ListS* names,
                                  std::function<char(ListS&, int)> visitor){
    ListI indexes;
    requireIndexes(names, indexes);
    //
    sk_sp<IColumn<char>> ret = sk_make_sp<IColumn<char>>();
    int rc = getRowCount();
    ret->prepareSize(rc);
    for(int i = 0 ; i < rc ; ++i ){
        ListS list_temp;
        m_delegate->get(i, &indexes, list_temp);
        ret->add(visitor(list_temp, i));
    }
    return ret;
}

void Table::visitRows2(ListS* names, std::function<void(ListS&, int)> visitor){
    ListI indexes;
    requireIndexes(names, indexes);
    //
    int rc = getRowCount();
    for(int i = 0 ; i < rc ; ++i ){
        ListS list_temp;
        m_delegate->get(i, &indexes, list_temp);
        visitor(list_temp, i);
    }
}

void Table::visitRows2OMP(ListI* indexes, std::function<void(ListS&, int)> visitor,
                          ListI* rowIdxes){
    if(rowIdxes == nullptr || rowIdxes->size() == 0){
        const int rc = getRowCount();
        #pragma omp parallel for
        for(int i = 0 ; i < rc ; ++i ){
            ListS list_temp;
            m_delegate->get(i, indexes, list_temp);
            visitor(list_temp, i);
        }
    }else{
        int rc = rowIdxes->size();
        #pragma omp parallel for
        for(int i = 0 ; i < rc ; ++i ){
            ListS list_temp;
            m_delegate->get(rowIdxes->get(i), indexes, list_temp);
            visitor(list_temp, rowIdxes->get(i));
        }
    }
    return;
}
void Table::visitRows2OMP(ListS* columnNames, std::function<void(ListS&, int)> visitor,
                          ListI* rowIdxes){
    ListI idxes;
    requireIndexes(columnNames, idxes);
    visitRows2OMP(&idxes, visitor, rowIdxes);
}
void Table::visitRows2(sk_sp<ListS> columnNames,
                       std::function<void(ListS&, int)> visitor){
    return visitRows2(columnNames.get(), visitor);
}
void Table::visitRows2(sk_sp<ListI> colIdxes, std::function<void(ListS&, int)> visitor){
    return visitRows2(colIdxes.get(), visitor);
}
void Table::visitRows2OMP(sk_sp<ListI> colIdxes, std::function<void(ListS&, int)> visitor,
                          ListI* rowIdxes){
    return visitRows2OMP(colIdxes.get(), visitor, rowIdxes);
}
void Table::visitRows2OMP(sk_sp<ListS> columnNames,
                       std::function<void(ListS&, int)> visitor,
                          ListI* rowIdxes){
    return visitRows2OMP(columnNames.get(), visitor, rowIdxes);
}

void Table::visitRows2(ListI* indexes, std::function<void(ListS&, int)> visitor){
    ListS list_temp;
    //
    int rc = getRowCount();
    for(int i = 0 ; i < rc ; ++i ){
        m_delegate->get(i, indexes, list_temp);
        visitor(list_temp, i);
        list_temp.clear();
    }
}

sk_sp<ListCh> Table::visitRowStates2(CString colName,
                                     std::function<char(String&, int)> visitor){
    int col_idx = getColumnIndex(colName);
    if(col_idx < 0){
        PRINTLN("visitRowStates2 >> column not exists. name = %s\n", colName.c_str());
        return nullptr;
    }
    sk_sp<IColumn<char>> ret = sk_make_sp<IColumn<char>>();
    int rc = getRowCount();
    ret->prepareSize(rc);
    for(int i = 0 ; i < rc ; ++i ){
        ret->add(visitor(m_delegate->get(i, col_idx), i));
    }
    return ret;
}

sk_sp<Table> Table::remapTable(ListS* pNames, ListS* outNames,
                               std::function<bool(ListS&, int)> visitor){
    ListI in_idxes;
    requireIndexes(pNames, in_idxes);
    ListI out_idxes;
    requireIndexes(outNames, out_idxes);
    //
    sk_sp<Table> sp = sk_make_sp<Table>();
    sp->setHeadLine(outNames->copy());
    ListS list_temp;
    int rc = getRowCount();
    for(int i = 0 ; i < rc ; i ++){
        m_delegate->get(i, &in_idxes, list_temp);
        if(visitor(list_temp, i)){
            list_temp.clear();
            m_delegate->get(i, &out_idxes, list_temp);
            sp->addRowAsFull(list_temp);
        }
        list_temp.clear();
    }
    return sp;
}

sk_sp<Table> Table::remapTable(ListS* outNames, bool copy){
    MED_ASSERT_X(outNames != nullptr, "remapTable: outNames can't be null.");
    ListI in_idxes;
    requireIndexes(outNames, in_idxes);
    //
    sk_sp<Table> sp = sk_make_sp<Table>();
    for(int i = 0 ; i < outNames->size() ; i ++){
        sp->addColumn(outNames->get(i),
                      m_delegate->getColumn(in_idxes.get(i), copy), false);
    }
    return sp;
}

sk_sp<Table> Table::remapTable(sk_sp<ListS> pNames, sk_sp<ListS> outNames,
                        std::function<bool(ListS&, int)> visitor){
    return remapTable(pNames.get(), outNames.get(), visitor);
}
sk_sp<Table> Table::remapTable(sk_sp<ListS> pNames, bool copy){
    return remapTable(pNames.get(), copy);
}


sk_sp<Table> Table::getTableByNonColumnName(SetColumn<String>* unNames, bool copy){
    sk_sp<ListS> sp_head = m_headLine->copy();
    int size = sp_head->size();
    for(int i = size-1 ; i >= 0 ; --i ){
        if(unNames->contains(sp_head->get(i))){
            sp_head->removeAt0(i);
        }
    }
    return remapTable(sp_head.get(), copy);
}
sk_sp<Table> Table::getTableByNonColumnName(ListS* unNames,bool copy){
    sk_sp<ListS> sp_head = m_headLine->copy();
    int size = sp_head->size();
    for(int i = size-1 ; i >= 0 ; --i ){
        if(unNames->contains(sp_head->get(i))){
            sp_head->removeAt0(i);
        }
    }
    return remapTable(sp_head.get(), copy);
}

sk_sp<Table> Table::subTable(ListI* row_indexes, ListI* col_indexes){
    if(row_indexes == nullptr && col_indexes == nullptr){
        return nullptr;
    }
    sk_sp<Table> sp = sk_make_sp<Table>();
    if(row_indexes != nullptr){
        for(int i = 0, size = row_indexes->size() ; i < size ; ++i ){
            sp->addRowAsFull(m_delegate->get(row_indexes->get(i), col_indexes));
        }
    }else{
        int size = col_indexes->size();
        for (int i = 0; i < size; ++i) {
            sp->m_delegate->addColumnAsFull(getColumn(col_indexes->get(i), true));
        }
    }
    return sp;
}

sk_sp<Table> Table::subTable(ListI* row_indexes, bool non_row_id){
    if(row_indexes == nullptr){
        return nullptr;
    }
    sk_sp<Table> sp = sk_make_sp<Table>();
    if(non_row_id){
        int rc = getRowCount();
        for (int i = 0; i < rc; ++i ) {
            if(!row_indexes->contains(i)){
                sp->addRowAsFull(m_delegate->getRow(i));
            }
        }
    }else{
        int size = row_indexes->size();
        for(int i = 0; i < size ; ++i ){
            sp->addRowAsFull(m_delegate->getRow(row_indexes->get(i)));
        }
    }
    sp->setHeadLine(m_headLine->copy());
    return sp;
}
sk_sp<Table> Table::subTable(sk_sp<ListI> row_indexes, bool non_row_id){
    return subTable(row_indexes.get(), non_row_id);
}

sk_sp<Table> Table::filter(ListS* colNames, std::function<bool(ListS*)> visitor){
    ListI in_idxes;
    requireIndexes(colNames, in_idxes);
    int rc = getRowCount();
    sk_sp<Table> sp = sk_make_sp<Table>();
    //
    ListS list_tmp;
    for (int i = 0; i < rc; ++i) {
        m_delegate->get(i, &in_idxes, list_tmp);
        if(visitor(&list_tmp)){
            sp->addRowAsFullDirect(list_tmp.list);
        }
        list_tmp.clear();
    }
    sp->setHeadLine(colNames !=nullptr && colNames->size() > 0 ?
                colNames->copy() : m_headLine->copy());
    return sp;
}
sk_sp<Table> Table::filter(sk_sp<ListS> colNames, std::function<bool(ListS*)> visitor){
    return filter(colNames.get(), visitor);
}

void Table::unique(){
    SetColumn<int> set;
    const int rc = getRowCount();
    ListI list_r(rc);
    //
    GroupS rows;
    m_delegate->getRows(rows);
    for(int i = 0 ; i < rc ; ++i ){
        int hash = rows[i]->hashCode();
        if(set.contains(hash)){
           list_r.add(i);
        }else{
           set.add(hash);
        }
    }
    if(list_r.size() == 0){
        return;
    }
    removeByFlags(&list_r);
}

sk_sp<ListS> Table::concatColumns(CString seq, ListS* colNames){
    ListI in_idxes;
    requireIndexes(colNames, in_idxes);
    //
    sk_sp<ListS> sp = sk_make_sp<ListS>();
    int rc = getRowCount();
    for (int i = 0; i < rc; i++) {
        sp->add(m_delegate->get(i, &in_idxes)->toString(seq));
    }
    return sp;
}
sk_sp<ListS> Table::concatColumns(CString seq, sk_sp<ListS> colNames){
    return concatColumns(seq, colNames.get());
}

int Table::findFirstIndex(ListS* colNames, std::function<bool(int, ListS*)> func){
    ListI in_idxes;
    requireIndexes(colNames, in_idxes);
    //
    int rc = getRowCount();
    for (int i = 0; i < rc; i++) {
        auto ref = m_delegate->get(i, &in_idxes);
        if(func(i, ref.get())){
            return i;
        }
    }
    return -1;
}
int Table::findFirstIndex(CString colName, std::function<bool(int, ListS*)> func){
    ListS list;
    list.add(colName);
    return findFirstIndex(&list, func);
}

sk_sp<ListS> Table::findFirst(ListS* in_names, ListS* out_names,
                              std::function<bool(int, ListS*)> func){
    ListI in_idxes;
    requireIndexes(in_names, in_idxes);
    ListI out_idxes;
    requireIndexes(out_names, out_idxes);
    //
    int rc = getRowCount();
    for (int i = 0; i < rc; i++) {
        auto ref = m_delegate->get(i, &in_idxes);
        if(func(i, ref.get())){
            return m_delegate->get(i, &out_idxes);
        }
    }
    return nullptr;
}
sk_sp<ListS> Table::findFirst(sk_sp<ListS> in_names, sk_sp<ListS> out_names,
                       std::function<bool(int, ListS*)> func){
    return findFirst(in_names.get(), out_names.get(), func);
}

sk_sp<h7::GRanges> Table::newGranges(CString colName, std::function<void(int, String&,
                                                              GRanges*)> func){
    int col_idx = getColumnIndex(colName);
    if(col_idx < 0){
        PRINTLN("newGranges >> column not exists. name = %s\n", colName.c_str());
        return nullptr;
    }
    //
    sk_sp<h7::GRanges> ret = sk_make_sp<GRanges>();
    int size = getRowCount();
    for(int i = 0 ; i < size ; i ++){
        func(i, m_delegate->get(i, col_idx), ret.get());
    }
    return ret;
}

sk_sp<GRanges> Table::newGranges(ListS* colNames, std::function<void(int, ListS*, GRanges*)> func){
    ListI in_idxes;
    requireIndexes(colNames, in_idxes);
    //
    sk_sp<GRanges> ret = sk_make_sp<GRanges>();
    int size = getRowCount();
    for(int i = 0 ; i < size ; i ++){
        auto row = m_delegate->get(i, &in_idxes);
        func(i, row.get(), ret.get());
    }
    return ret;
}

void Table::splitByColumn(CString colName, std::function<sk_sp<ListS>(String&, int)> func){
    int idx = requireColumn(colName);
    auto alts = getColumn(idx, false);
    auto altss = alts->map<sk_sp<ListS>>(func);
    int rc = alts->size();
    GroupS gs;
    getDelegate()->getRows(gs);

    GroupS newGs(gs.size());
    for(int i = 0 ; i < rc ; ++i){
        auto row = gs.get(i);
        auto& nss = altss->get(i);
        if(nss->size() == 1){
            newGs.add(row);//no chaned
        }else{
            //1 row -> N row
            for(int j = 0 ; j < nss->size() ; ++j){
                auto row1 = row->copy();
                row1->set0(idx, nss->get(j));
                newGs.add(row1);
            }
        }
    }
    getDelegate()->setRows(newGs);
}
void Table::group(CString colName, h7::HashMap<String, GroupS>& groups){
    int idx = requireColumn(colName);
    int rc = getRowCount();
    GroupS gs(rc);
    getDelegate()->getRows(gs);
    //
    for(int i = 0 ; i < rc ; ++i){
        auto row = gs.get(i);
        auto& name = row->get(idx);
        auto it = groups.find(name);
        if(it != groups.end()){
            it->second.add(row);
        }else{
            GroupS cs(32);
            cs.add(row);
            groups.put0(name, cs);
        }
    }
    //
}
void Table::group(CString colName, ListS* o_keys, h7::IColumn<GroupS>* o_vals){
     h7::HashMap<String, GroupS> groups;
     group(colName, groups);
     o_keys->prepareSize(groups.size());
     o_vals->prepareSize(groups.size());
     //
     auto it = groups.begin();
     for(; it != groups.end(); ++it){
        o_keys->add(it->first);
        o_vals->add(it->second);
     }
}
sk_sp<h7::IColumn<sk_sp<Table>>> Table::group(CString colName){
    auto o_keys = sk_make_sp<ListS>();
    auto o_vals = sk_make_sp<h7::IColumn<GroupS>>();
    group(colName, o_keys.get(), o_vals.get());
    auto head = getHeadLine();
    //
    auto ret = sk_make_sp<h7::IColumn<sk_sp<Table>>>();
    for(int i = 0 ; i < o_keys->size() ; ++i){
        auto tab = sk_make_sp<Table>(head->copy());
        tab->setName(o_keys->get(i));
        tab->getDelegate()->setRows(o_vals->get(i));
        ret->add(tab);
    }
    return ret;
}

sk_sp<ListS> Table::rMatchResult(CString outName, ListI* indexes, CString defVal){
    if(indexes == nullptr || indexes->size() == 0){
        return nullptr;
    }
    int col_idx = getColumnIndex(outName);
    if(col_idx < 0){
        PRINTLN("rMatchResult >> column not exists. name = %s\n", outName.c_str());
        return nullptr;
    }
    const int rc = getRowCount();
    const int size = indexes->size();
    sk_sp<ListS> ret = sk_make_sp<ListS>(size);
    int _val;
    for(int i = 0 ; i < size ; i++){
        _val = indexes->get(i);
        if(_val != INVALID_INT){
            if(_val >= rc){
                ret->add(defVal);
            }else{
                ret->add(m_delegate->get(_val, col_idx));
            }
        }else{
            ret->add(defVal);
        }
    }
    return ret;
}
sk_sp<GroupS> Table::rMatchResult(ListS* outNames, ListI* indexes, CString defVal){
    if(indexes == nullptr || indexes->size() == 0){
        return nullptr;
    }
    ListI in_idxes;
    requireIndexes(outNames, in_idxes);
    int colCount = in_idxes.isEmpty() ? getColumnCount():in_idxes.size();
    sk_sp<ListS> list_null = h7::utils::produceList<String>(colCount, defVal);
    //
    const int rc = getRowCount();
    const int size = indexes->size();
    sk_sp<GroupS> ret = sk_make_sp<GroupS>(size);
    int _val;
    for(int i = 0 ; i < size ; ++i ){
        _val = indexes->get(i);
        if(_val != INVALID_INT){
            if(_val >= rc){
                ret->add(list_null->copy());
            }else{
                ret->add(m_delegate->get(_val, &in_idxes));
            }
        }else{
            ret->add(list_null->copy());
        }
    }
    return ret;
}

sk_sp<Table> Table::rMatchResultTable(ListS* outNames, ListI* indexes, CString defVal){
    if(indexes == nullptr || indexes->size() == 0){
        return nullptr;
    }
    ListI in_idxes;
    requireIndexes(outNames, in_idxes);
    int colCount = in_idxes.isEmpty() ? getColumnCount():in_idxes.size();
    sk_sp<ListS> list_null = h7::utils::produceList<String>(colCount, defVal);
    //
    sk_sp<Table> ret = sk_make_sp<Table>(outNames != nullptr ?
                outNames->copy() : getHeadLine()->copy());
    const int rc = getRowCount();
    const int size = indexes->size();
    ret->prepareRowCount(size);
    int _val;
    for(int i = 0 ; i < size ; ++i ){
        _val = indexes->get(i);
        if(_val != INVALID_INT){
            if(_val >= rc){
                ret->addRowAsFull(list_null->copy());
            }else{
                ret->addRowAsFull(m_delegate->get(_val, &in_idxes));
            }
        }else{
            ret->addRowAsFull(list_null->copy());
        }
    }
    return ret;
}

sk_sp<ListS> Table::rMatchResult(CString outName, sk_sp<ListI> indexes,
                                 CString defVal){
    return rMatchResult(outName, indexes.get(), defVal);
}
sk_sp<GroupS> Table::rMatchResult(ListS* outNames,sk_sp<ListI> indexes,
                                 CString defVal){
    return rMatchResult(outNames, indexes.get(), defVal);
}
sk_sp<Table> Table::rMatchResultTable(ListS* outNames, sk_sp<ListI> indexes,
                               CString defVal){
    return rMatchResultTable(outNames, indexes.get(), defVal);
}

sk_sp<ListS> Table::rMatch(CString colName, ListS* valueList, CString outColName){
    int col_idx = getColumnIndex(colName);
    if(col_idx < 0){
        PRINTLN("rMatch >> column not exists. name = %s\n", colName.c_str());
        return nullptr;
    }
    sk_sp<ListS> vals = getColumn(col_idx, false);
    if(!vals || vals->size() == 0){
        return nullptr;
    }
    sk_sp<ListI> indexes = h7::utils::rMatch(vals.get(), valueList);
    return rMatchResult(outColName, indexes.get(), "");
}
sk_sp<GroupS> Table::rMatch(CString colName, ListS* valueList, ListS* outColNames){
    int col_idx = getColumnIndex(colName);
    if(col_idx < 0){
        PRINTLN("rMatch >> column not exists. name = %s\n", colName.c_str());
        return nullptr;
    }
    sk_sp<ListS> vals = getColumn(col_idx,false);
    if(!vals || vals->size() == 0){
        return nullptr;
    }
    sk_sp<ListI> indexes = h7::utils::rMatch(vals.get(), valueList);
    return rMatchResult(outColNames, indexes.get(), "");
}

sk_sp<ListS> Table::rMatchReversed(CString colName, ListS* valueList, CString outColName){
    int col_idx = getColumnIndex(colName);
    if(col_idx < 0){
        PRINTLN("rMatchReversed >> column not exists. name = %s\n", colName.c_str());
        return nullptr;
    }
    sk_sp<ListS> vals = getColumn(col_idx, false);
    if(!vals || vals->size() == 0){
        return nullptr;
    }
    sk_sp<ListI> indexes = h7::utils::rMatch(valueList, vals.get());
    return rMatchResult(outColName, indexes.get(), "");
}

sk_sp<ListS> Table::rMatchReversed(CString colName, sk_sp<ListS> valueList,
                                  CString outColName){
    return rMatchReversed(colName, valueList.get(), outColName);
}

sk_sp<ListS> Table::rMatchReversed0(ListS* valueList){
    return rMatchReversed(m_headLine->get(0), valueList, m_headLine->get(0));
}

sk_sp<Table> Table::rStrsplit_rbind(CString colName, CString sep){
    int col_idx = getColumnIndex(colName);
    if(col_idx < 0){
        PRINTLN("rStrsplit_rbind >> column not exists. name = %s\n", colName.c_str());
        return nullptr;
    }
    int maxLen = 0;
    const int rc = getRowCount();
    IColumn<sk_sp<ListS>> rows(rc, true);
    for(int i = 0 ; i < rc ; ++i ){
        String s = m_delegate->get(i, col_idx);
        auto sp = sk_make_sp<ListS>(h7::utils::split(sep, s));
        maxLen = HMAX(maxLen, sp->size());
        rows.set0(i, sp);
    }
    //tile and add to table
    sk_sp<Table> ret = sk_make_sp<Table>();
    for (int i = 0; i < rc; ++i ) {
        auto sp = rows.get(i);
        sp->tileToLength(maxLen);
        ret->addRowAsFull(sp);
    }
    return ret;
}

void Table::rCbindRow(ListS* colNames, TabContentDelegate* rowData){
    MED_ASSERT_X(colNames->size() == rowData->getColumnCount(),
                 "column count must be the same");
    m_delegate->addContentByColumn(rowData);
    m_headLine->addAll(colNames);
}
void Table::rCbindRow(ListS* colNames, sk_sp<TabContentDelegate> rowData){
    rCbindRow(colNames, rowData.get());
}
void Table::rCbindRow(ListS* colNames, Table* rowData){
    rCbindRow(colNames, rowData->m_delegate.get());
}
void Table::rCbindRow(ListS* colNames, sk_sp<Table> rowData){
    rCbindRow(colNames, rowData.get());
}
void Table::rRbindRow(TabContentDelegate* rowData){
    MED_ASSERT(getColumnCount() == rowData->getColumnCount());
    m_delegate->addContentByRow(rowData);
}
void Table::rRbindRow(sk_sp<TabContentDelegate> rowData){
    MED_ASSERT(getColumnCount() == rowData->getColumnCount());
    m_delegate->addContentByRow(rowData);
}
void Table::rRbindRow(Table* rowData){
    MED_ASSERT(getColumnCount() == rowData->getColumnCount());
    m_delegate->addContentByRow(rowData->m_delegate);
}
void Table::rRbindRow(sk_sp<Table> rowData){
    MED_ASSERT(getColumnCount() == rowData->getColumnCount());
    m_delegate->addContentByRow(rowData->m_delegate);
}


bool Table::rGreplAny(CString colName, std::regex& reg){
    int col_idx = getColumnIndex(colName);
    if(col_idx < 0){
        PRINTLN("rGreplAny >> column not exists. name = %s\n", colName.c_str());
        return false;
    }
    const int rc = getRowCount();
    for(int i = 0 ; i < rc ; ++i ){
        if(h7::utils::rGrepl(reg, m_delegate->get(i, col_idx))){
            return true;
        }
    }
    return false;
}
bool Table::rGreplAny(CString colName, CString pat){
    std::regex reg(pat.c_str());
    return rGreplAny(colName, reg);
}

bool Table::rWhichAny(ListS* colNames, std::function<bool(ListS&, int)> which){
    ListI indexes;
    requireIndexes(colNames, indexes);
    int rc = getRowCount();
    ListS list_temp(indexes.size() != 0 ? indexes.size()
                                : m_delegate->getColumnCount());
    for(int i = 0 ; i < rc ; ++i){
        m_delegate->get(i, &indexes, list_temp);
        if(which(list_temp, i)){
            return true;
        }
        list_temp.clear();
    }
    return false;
}
bool Table::rWhichAny(sk_sp<ListS> colNames, std::function<bool(ListS&, int)> which){
    return rWhichAny(colNames.get(), which);
}
sk_sp<ListI> Table::rWhich(ListS* colNames, std::function<bool(ListS&, int)> which){
    return visitRows<int>(colNames, [which](ListS& strs, int _index){
        return which(strs, _index) ? _index : -1;
    }, [](int& val){
        return val >= 0;
    });
}
sk_sp<ListI> Table::rWhich(CString colName, std::function<bool(ListS&, int)> which){
    ListS list;
    list.add(colName);
    return rWhich(&list, which);
}
sk_sp<ListI> Table::rWhich(sk_sp<ListS> colNames, std::function<bool(ListS&, int)> which){
    return rWhich(colNames.get(), which);
}

void Table::rSplit(CString outColName, ListS* combinedColumns,
                   std::map<String, sk_sp<ListS>>& outMap,
            CString sep){
    sk_sp<ListS> cols = combinedColumns->copy();
    cols->add(outColName);
    std::map<String, sk_sp<ListS>> *out = &outMap;
    visitRows2(cols.get(), [out, sep](ListS& ls, int){
        String key = ls.sub(0, ls.size() - 1)->toString(sep);
        if(out->find(key) == out->end()){
            (*out)[key] = sk_make_sp<ListS>();
        }
        out->at(key)->add(ls.last());
    });
}
void Table::rSplit(CString outColName, sk_sp<ListS> combinedColumns,
            std::map<String,sk_sp<ListS>>& outMap, CString sep){
    return rSplit(outColName, combinedColumns.get(), outMap, sep);
}

sk_sp<IColumn<Table::PairSI>> Table::rTable(CString colName,
                                            std::function<bool(PairSI&, int)> pre,
                              IColumn<sk_sp<ListI>>* outIdxes){
    auto strs = optColumn(colName, false);
    if(!strs){
        return nullptr;
    }
    auto pairs = h7::utils::rTable<String>(strs.get(), nullptr, outIdxes);
    if(pre){
        for(int i = 0 ; i < pairs->size(); ++i ){
            auto& p = pairs->get(i);
            if(!pre(p, i)){
                pairs->removeAt0(i);
                if(outIdxes){
                    outIdxes->removeAt0(i);
                }
                i--;
            }
        }
    }
    return pairs;
}
sk_sp<ListS> Table::rTableKeys(CString colName, std::function<bool(PairSI&,int)> pre,
                                 IColumn<sk_sp<ListI>>* outIdxes){
    if(!hasColumn(colName)){
        return nullptr;
    }
    auto strs = optColumn(colName, false);
    if(!strs){
        return nullptr;
    }
    auto pairs = h7::utils::rTable<String>(strs.get(), nullptr, outIdxes);
    sk_sp<ListS> ret = sk_make_sp<ListS>();
    if(pre){
        ret->prepareSize(pairs->size() / 2);
        for(int i = 0 ; i < pairs->size(); ++i){
            auto& p = pairs->get(i);
            if(!pre(p, i)){
                pairs->removeAt0(i);
                if(outIdxes){
                    outIdxes->removeAt0(i);
                }
                i--;
            }else{
                ret->add(p.key);
            }
        }
    }else{
        ret->prepareSize(pairs->size());
        for(int i = 0 ; i < pairs->size(); ++i ){
            ret->add(pairs->get(i).key);
        }
    }
    return ret;
}
void Table::alignByNames(ListS* names){
    auto heads = getHeadLine();
    ListS newColNames;
    for(String& n: names->list){
        if(heads->indexOf(n) < 0){
            newColNames.add(n);
        }
    }
    //no new names
    if(newColNames.size() == 0){
        return;
    }
    int rc = getRowCount();
    if(rc == 0){
        heads->addAll(&newColNames);
        return;
    }
    for(int i = 0 ; i < newColNames.size() ; ++i){
        auto list = h7::utils::produceList<String>(rc, "");
        addColumn(newColNames.get(i), list);
    }
}
void Table::prepareColumns(sk_sp<ListS> names, CString defVal){
    ListS newNames;
    int rc = getRowCount();
    for(int i = 0 ; i < names->size(); ++i){
        if(!hasColumn(names->get(i))){
            addColumn(names->get(i), utils::produceList<String>(rc, defVal));
            newNames.add(names->get(i));
        }
    }
    auto str = newNames.toStringFullly();
    printf("prepareColumns >> %s\n", str.data());
}

}
