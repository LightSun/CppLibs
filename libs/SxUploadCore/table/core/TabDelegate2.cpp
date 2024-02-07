#include "TabDelegate2.h"
#include "utils/string_utils.hpp"
#include "table/core/TabContentDelegate.h"
#include "table/core/Table.h"
#include "table/SetColumn.h"

using namespace h7;
using namespace sort;

sk_sp<TabBlock> TabBlock::copy(){
    auto spb = sk_make_sp<TabBlock>();
    spb->gs = GroupS_copy(*this->gs);
    spb->groupKey = this->groupKey;
    spb->start_idx = this->start_idx;
    return spb;
}
void TabBlock::removeIf(std::function<bool(sk_sp<ListS>, int)> func){
    int s = gs->size();
    for(int i = s - 1 ; i >= 0 ; --i){
        if(func(gs->get(i), start_idx + i)){
            gs->removeAt0(i);
        }
    }
}
void TabBlock::removeByFlags(ListI* l){
    int s = gs->size();
    for(int i = s - 1 ; i >= 0 ; --i){
        if(l->const_get(start_idx + i)){
            gs->removeAt0(i);
        }
    }
}
void TabBlock::removeByFlags(ListB* l){
    int s = gs->size();
    for(int i = s - 1 ; i >= 0 ; --i){
        if(l->const_get(start_idx + i)){
            gs->removeAt0(i);
        }
    }
}
void TabBlock::sortDefault(CSortItem item2){
    using SPLS = sk_sp<ListS>;
    std::sort(gs->list.begin(), gs->list.end(),
           [&item2](SPLS& l1, SPLS& l2){
        auto& s1 = l1->get(item2.colIndex);
        auto& s2 = l2->get(item2.colIndex);
        return item2.aesc ? s1 < s2 : s1 > s2;
    });
}
void TabBlock::sort(CSortItem si){

    SortItem& item2 = (SortItem&)si;
    //
    using SPLS = sk_sp<ListS>;
    switch(item2.type){
    case kSortType_STRING:{
        if(item2.cvtStr){
            std::sort(gs->list.begin(), gs->list.end(),
                   [&item2](SPLS& l1, SPLS& l2){
                auto& s1 = l1->get(item2.colIndex);
                auto& s2 = l2->get(item2.colIndex);
                //std sort can't use ">= or <="
                auto s11 = item2.cvtStr(s1);
                auto s22 = item2.cvtStr(s2);
                return item2.aesc ? s11 < s22 : s11 > s22;
            });
        }else{
            sortDefault(item2);
        }
    }break;
    case kSortType_FLOAT:{
        if(!item2.cvtFloat){
            item2.cvtFloat = [](String& s){
                return utils::getFloat(s);
            };
        }
        std::sort(gs->list.begin(), gs->list.end(),
               [&item2](SPLS& l1, SPLS& l2){
            auto& s1 = l1->get(item2.colIndex);
            auto& s2 = l2->get(item2.colIndex);
            //std sort can't use ">= or <="
            auto s11 = item2.cvtFloat(s1);
            auto s22 = item2.cvtFloat(s2);
            return item2.aesc ? s11 < s22 : s11 > s22;
        });
    }break;

    case kSortType_I32:{
        if(!item2.cvtI32){
            item2.cvtI32 = [](String& s){
                return utils::getInt(s);
            };
        }
        std::sort(gs->list.begin(), gs->list.end(),
               [&item2](SPLS& l1, SPLS& l2){
            auto& s1 = l1->get(item2.colIndex);
            auto& s2 = l2->get(item2.colIndex);
            //std sort can't use ">= or <="
            auto s11 = item2.cvtI32(s1);
            auto s22 = item2.cvtI32(s2);
            return item2.aesc ? s11 < s22 : s11 > s22;
        });
    }break;

    case kSortType_UI32:{
        if(!item2.cvtUI32){
            item2.cvtUI32 = [](String& s){
                return utils::getUInt(s);
            };
        }
        std::sort(gs->list.begin(), gs->list.end(),
               [&item2](SPLS& l1, SPLS& l2){
            auto& s1 = l1->get(item2.colIndex);
            auto& s2 = l2->get(item2.colIndex);
            //std sort can't use ">= or <="
            auto s11 = item2.cvtUI32(s1);
            auto s22 = item2.cvtUI32(s2);
            return item2.aesc ? s11 < s22 : s11 > s22;
        });
    }break;
    case kSortType_I64:{
        if(!item2.cvtI64){
            item2.cvtI64 = [](String& s){
                return utils::getLong(s);
            };
        }
        std::sort(gs->list.begin(), gs->list.end(),
               [&item2](SPLS& l1, SPLS& l2){
            auto& s1 = l1->get(item2.colIndex);
            auto& s2 = l2->get(item2.colIndex);
            //std sort can't use ">= or <="
            auto s11 = item2.cvtI64(s1);
            auto s22 = item2.cvtI64(s2);
            return item2.aesc ? s11 < s22 : s11 > s22;
        });
    }break;
    case kSortType_UI64:{
        if(!item2.cvtUI64){
            item2.cvtUI64 = [](String& s){
                return utils::getULong(s);
            };
        }
        std::sort(gs->list.begin(), gs->list.end(),
               [&item2](SPLS& l1, SPLS& l2){
            auto& s1 = l1->get(item2.colIndex);
            auto& s2 = l2->get(item2.colIndex);
            //std sort can't use ">= or <="
            auto s11 = item2.cvtUI64(s1);
            auto s22 = item2.cvtUI64(s2);
            return item2.aesc ? s11 < s22 : s11 > s22;
        });
    }break;

    default:
        String msg = "TabBlock::sort >> wrong type = " + std::to_string(item2.type);
        MED_ASSERT_X(false, msg);
    }
}


//----------------
TabDelegate2::TabDelegate2(GroupS* gs, int max_count):m_gs(gs)
{
    groupByEveryCount(max_count);
}
TabDelegate2::TabDelegate2(SPGroupS gs, sk_sp<ListS> head){
    m_gs_local = gs;
    m_gs = gs.get();
    m_headLine = head;
    m_rowCount = gs->size();
}
TabDelegate2::TabDelegate2(GroupS* gs, sk_sp<ListS> head){
    m_gs = gs;
    m_headLine = head;
    m_rowCount = gs->size();
}

sk_sp<TabDelegate2> TabDelegate2::copy(){
    auto ins = sk_make_sp<TabDelegate2>(GroupS_copy(*m_gs_local), m_headLine);
    auto bs = this->m_blocks.copy();
    ins->m_blocks = std::move(bs->list);
    return ins;
}
void TabDelegate2::removeIf(std::function<bool(sk_sp<ListS>, int)> func,bool _updateIdx){
    int s = m_blocks.size();
#pragma omp parallel for
    for(int i = 0 ; i < s; ++i){
        m_blocks[i]->removeIf(func);
    }
    if(_updateIdx){
        updateIdx();
    }
}
void TabDelegate2::removeByFlags(ListI* fs, bool _updateIdx){
    int s = m_blocks.size();
#pragma omp parallel for
    for(int i = 0 ; i < s; ++i){
        m_blocks[i]->removeByFlags(fs);
    }
    if(_updateIdx){
        updateIdx();
    }
}
void TabDelegate2::removeByFlags(ListB* fs, bool _updateIdx){
    auto states = fs->asInt();
    removeByFlags(states.get(), _updateIdx);
}
void TabDelegate2::copyTo(GroupS* gs){
    int s = m_blocks.size();
    int total = 0;
    for(int i = 0 ; i < s; ++i){
        total += m_blocks[i]->gs->size();
    }
    gs->prepareSize(total + gs->size());
    for(int i = 0 ; i < s; ++i){
        gs->addAll(m_blocks[i]->gs);
    }
}
void TabDelegate2::sort(CSortItem si){
    if(!si.colName.empty()){
        MED_ASSERT_X(m_headLine, "must have head_line for sort-by-name.");
        ((SortItem&)si).colIndex = m_headLine->indexOf(si.colName);
    }
    int s = m_blocks.size();
#pragma omp parallel for
    for(int i = 0 ; i < s; ++i){
        m_blocks[i]->sort(si);
    }
}
void TabDelegate2::groupByEveryCount(int every){
    m_blocks.clear();
    auto& gs = m_gs;
    int rc = gs->size();
    for(int i = 0 ; i < rc ; i += every){
        auto b = sk_make_sp<TabBlock>();
        b->gs = gs->sub(i, HMIN(rc, i + every));
        b->start_idx = i;
        m_blocks.add(b);
    }
    m_rowCount = rc;
}
void TabDelegate2::groupByThreadCount(int tc){
    int count = getRowCount() / tc;
    groupByEveryCount(count + 1);
}
void TabDelegate2::group(int group_index){
    MED_ASSERT(group_index >= 0);
    m_blocks.clear();
    std::unordered_map<String, SPBlock> _map;
    int rc = m_gs->size();
    for(int i = 0 ; i < rc ;  ++i){
        auto row = m_gs->get(i);
        auto& key = row->get(group_index);
        auto it = _map.find(key);
        if(it != _map.end()){
            it->second->gs->add(row);
        }else{
            SPBlock b = sk_make_sp<TabBlock>();
            b->start_idx = -1;
            b->groupKey = key;
            b->gs = sk_make_sp<GroupS>();
            b->gs->add(row);
            _map[key] = b;
        }
    }
    //
    auto it = _map.begin();
    while (it != _map.end()) {
        m_blocks.add(it->second);
        ++it;
    }
    updateIdx();
}
void TabDelegate2::group(CString name){
    group(m_headLine->indexOf(name));
}
void TabDelegate2::group(std::function<String(sk_sp<ListS>)> func){
    m_blocks.clear();
    std::unordered_map<String, SPBlock> _map;
    int rc = m_gs->size();
    for(int i = 0 ; i < rc ;  ++i){
        auto row = m_gs->get(i);
        auto key = func(row);
        auto it = _map.find(key);
        if(it != _map.end()){
            it->second->gs->add(row);
        }else{
            SPBlock b = sk_make_sp<TabBlock>();
            b->start_idx = -1;
            b->groupKey = key;
            b->gs = sk_make_sp<GroupS>();
            b->gs->add(row);
            _map[key] = b;
        }
    }
    //
    auto it = _map.begin();
    while (it != _map.end()) {
        m_blocks.add(it->second);
        ++it;
    }
    updateIdx();
}
void TabDelegate2::group(CString name, std::function<String(String&)> func){
    MED_ASSERT(m_headLine);
    int idx = requireColumn(name);
    group([idx, func](sk_sp<ListS> l){
        return func(l->get(idx));
    });
}
void TabDelegate2::removeGroups(std::function<bool(String&)> func, bool _updateIdx){
    int s = m_blocks.size();
    for(int i = s - 1 ; i >= 0; --i){
        if(func(m_blocks[i]->groupKey)){
            m_blocks.removeAt0(i);
        }
    }
    if(_updateIdx){
        updateIdx();
    }
}
void TabDelegate2::copyToGroup(){
    m_gs->clear();
    copyTo(m_gs);
}
sk_sp<h7::Table> TabDelegate2::toTable(){
    copyToGroup();
    auto tcd = sk_make_sp<TabContentDelegate>();
    tcd->setRows(m_gs);
    return sk_make_sp<Table>(m_headLine->copy(), tcd);
}
sk_sp<h7::Table> TabDelegate2::asTable(){
    makeColumnCache();
    return sk_make_sp<Table>(m_headLine, getColumnDelegate());
}

void TabDelegate2::updateIdx(int startIdx){
    const int sid = startIdx;
    int s = m_blocks.size();
    for(int i = 0 ; i < s; ++i){
        m_blocks[i]->start_idx = startIdx;
        startIdx += m_blocks[i]->gs->size();
    }
    m_rowCount = startIdx - sid;
}
void TabDelegate2::updateAll(int startIdx){
    updateIdx(startIdx);
    copyToGroup();
}
int TabDelegate2::getColumnCount(){
    if(m_headLine) return m_headLine->size();
    if(m_blocks.size() > 0){
        int s = m_blocks.size();
        for(int i = 0 ; i < s; ++i){
            if(m_blocks[i]->gs->size() > 0){
                return m_blocks[i]->gs->get(0)->size();
            }
        }
    }
    return 0;
}
sk_sp<TabDelegate2> TabDelegate2::getBlockAsTab(int index, bool copy){
    if(copy){
        return sk_make_sp<TabDelegate2>(m_blocks.get(index)->gs->copy(),
                                        m_headLine->copy());
    }else{
        return sk_make_sp<TabDelegate2>(m_blocks.get(index)->gs,
                                        m_headLine);
    }
}
TabDelegate2::SPBlock TabDelegate2::findBlock(CString groupName){
    int s = m_blocks.size();
    for(int i = 0 ; i < s; ++i){
        auto b = m_blocks.get(i);
        if(b->groupKey == groupName && b->gs->size() > 0){
            return b;
        }
    }
    return nullptr;
}
sk_sp<TabDelegate2> TabDelegate2::findBlockAsTab(CString groupName, bool copy){
    auto ret = findBlock(groupName);
    if(!ret || ret->gs->size() == 0) return nullptr;
    if(copy){
        return sk_make_sp<TabDelegate2>(ret->gs->copy(), m_headLine->copy());
    }else{
        return sk_make_sp<TabDelegate2>(ret->gs, m_headLine);
    }
}
void TabDelegate2::makeColumnCache(bool force){
    if(!force && m_columnDelegate){
        return;
    }
    m_columnDelegate = sk_make_sp<TabContentDelegate>();
    m_columnDelegate->setRows(m_gs);
}
int TabDelegate2::requireColumn(CString name){
    MED_ASSERT(m_headLine);
    int idx = m_headLine->indexOf(name);
    if(idx < 0){
        String msg = "can't find column(" + name + ")";
        MED_ASSERT_X(false, msg);
    }
    return idx;
}
sk_sp<ListI> TabDelegate2::requireColumns(sk_sp<ListS> colNames){
    auto ret = sk_make_sp<ListI>(colNames->size());
    for(int i = 0 ; i < colNames->size() ; ++i){
        ret->add(requireColumn(colNames->get(i)));
    }
    return ret;
}
sk_sp<ListS> TabDelegate2::getColumn(CString name, bool copy){
    MED_ASSERT(m_columnDelegate);
    return m_columnDelegate->getColumn(requireColumn(name), copy);
}
sk_sp<ListS> TabDelegate2::optColumn(CString name, bool copy){
    MED_ASSERT(m_columnDelegate);
    MED_ASSERT(m_headLine);
    int idx = m_headLine->indexOf(name);
    if(idx < 0){
        return nullptr;
    }
    return m_columnDelegate->optColumn(idx, copy);
}
void TabDelegate2::copyToGroupByColumnDelegate(){
    MED_ASSERT(m_columnDelegate);
    m_columnDelegate->getRows(*m_gs);
}
//----------------
//
sk_sp<ListI> TabDelegate2::visitRowsToInt(sk_sp<ListI> idxes,
                                          std::function<int(ListS*)> func){
    //RowVisitor
    int s = m_blocks.size();
    std::vector<std::vector<int>> groups;
    groups.resize(s);
#pragma omp parallel for
    for(int i = 0 ; i < s; ++i){
        RowVisitor<int> rv;
        rv.func = func;
        m_blocks.get(i)->visitRowsByColumnIdxes(idxes, &rv);
        groups[i] = std::move(rv.results);
    }
    int total = 0;
    for(size_t i = 0 ; i < groups.size() ; ++i){
        total += groups[i].size();
    }
    auto ret = sk_make_sp<ListI>(total);
    for(size_t i = 0 ; i < groups.size() ; ++i){
        ret->addAll(groups[i]);
    }
    return ret;
}
void TabDelegate2::filterRowSelf(sk_sp<ListS> colNames,
                                 std::function<bool(h7::ListS* l)> func){
    filterRowSelfByColumnIndexes(requireColumns(colNames), func);
}
void TabDelegate2::filterRowSelfByColumnIndexes(sk_sp<ListI> idxes,
                                  std::function<bool(h7::ListS* l)> func){
    auto states = visitRowsToInt(idxes, [func](h7::ListS* l){
        return func(l) ? 1 : 0;
    });
    removeByFlags(states.get());
}
//check groups
void TabDelegate2::filterRowSelf(CString colName,
                                 std::function<bool(String& s)> func){
    int index = requireColumn(colName);
    auto ifunc = [index, func](h7::ListS* l){
        return func(l->get(index)) ? 1 : 0;
    };
    auto states = visitRowsToInt(nullptr, ifunc);
    removeByFlags(states.get());
}
void TabDelegate2::removeNotExist(CString colName, SetColumn<String>* valueList){
    int index = requireColumn(colName);
    auto ifunc = [index, valueList](h7::ListS* l){
        return valueList->contains(l->get(index)) ? 0 : 1;
    };
    auto states = visitRowsToInt(nullptr, ifunc);
    removeByFlags(states.get());
}
void TabDelegate2::removeNotExist(CString colName, ListS* valueList){
    int index = requireColumn(colName);
    auto ifunc = [index, valueList](h7::ListS* l){
        return valueList->contains(l->get(index)) ? 0 : 1;
    };
    auto states = visitRowsToInt(nullptr, ifunc);
    removeByFlags(states.get());
}
