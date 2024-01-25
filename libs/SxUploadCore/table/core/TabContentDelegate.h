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

struct _GItem: public SkRefCnt{
    String key;
    sk_sp<GroupS> gs;

    _GItem(CString _key, int prepareSize): key(_key){
        gs = sk_make_sp<GroupS>(prepareSize);
    }
    void add(sk_sp<ListS> l){
        gs->add(l);
    }
};
template<typename E>
struct GRetItem: public SkRefCnt{
    String key;
    E ret;
    GRetItem(CString _key): key(_key){}
};

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

    template<typename E>
    void visitGroup(std::function<void(ListS*, String&)> group,
                    std::function<E(const String&, sk_sp<GroupS>&)> visitor,
                    std::map<String, E>& outMap){
         int rc = getRowCount();
         std::map<String, sk_sp<GroupS>> _map;
         String key;
         sk_sp<ListS> sp;
         for(int i = 0 ; i < rc ; ++i ){
            sp = getRow(i);
            group(sp.get(), key);
            auto it = _map.find(key);
            if(it != _map.end()){
                it->second->add(sp);
            }else{
                auto val = sk_make_sp<GroupS>();
                val->prepareSize(8);
                val->add(sp);
                _map[key] = val;
            }
         }
         auto it_end = _map.end();
         auto it = _map.begin();
         while(it != it_end){
             auto& key = it->first;
             outMap[key] = visitor(key, it->second);
             it ++;
         }
    }

    template<typename E>
    void visitGroup(std::function<void(ListS*, String&)> group,
                    std::function<E(const String&, sk_sp<GroupS>&)> visitor,
                    std::unordered_map<String, E>& outMap, int p_size = 32){
         int rc = getRowCount();
         String key;
         sk_sp<ListS> sp;
         std::map<String, sk_sp<GroupS>> _map;
         for(int i = 0 ; i < rc ; ++i ){
            sp = getRow(i);
            group(sp.get(), key);
            auto it = _map.find(key);
            if(it != _map.end()){
                it->second->add(sp);
            }else{
                auto val = sk_make_sp<GroupS>();
                val->prepareSize(p_size);
                val->add(sp);
                _map[key] = val;
            }
         }
         auto it_end = _map.end();
         auto it = _map.begin();
         while(it != it_end){
              auto& key = it->first;
              outMap[key] = visitor(key, it->second);
              it ++;
         }
    }

    template<typename E>
    void visitGroup2(int PSize, std::function<void(ListS*, String&)> group,
                    std::function<E(const String&, sk_sp<GroupS>&)> visitor,
                    std::map<String, E>& outMap){
         int rc = getRowCount();
         String key;
         sk_sp<ListS> sp;
         std::map<String, sk_sp<GroupS>> _map;
         for(int i = 0 ; i < rc ; ++i ){
            sp = getRow(i);
            group(sp.get(), key);
            auto it = _map.find(key);
            if(it != _map.end()){
                it->second->add(sp);
            }else{
                auto val = sk_make_sp<GroupS>();
                val->prepareSize(PSize);
                val->add(sp);
                _map[key] = val;
            }
         }
         auto it_end = _map.end();
         auto it = _map.begin();
         while(it != it_end){
              auto& key = it->first;
              outMap[key] = visitor(key, it->second);
              it ++;
         }
    }

    template<typename E>
    void visitGroupOMP(ListI* idxes, int psize,
                       std::function<void(ListS*, String&)> group,
                       std::function<E(const String&, sk_sp<GroupS>&)> visitor,
                       IColumn<sk_sp<GRetItem<E>>>& out);

    void visitGroupU32(std::function<void(ListS*, uint32&)> group,
                    std::map<uint32, sk_sp<GroupS>>& _map){
         int rc = getRowCount();
         uint32 key;
         sk_sp<ListS> sp;
         //std::map<uint32, sk_sp<GroupS>> _map;
         for(int i = 0 ; i < rc ; ++i ){
            sp = getRow(i);
            group(sp.get(), key);
            auto it = _map.find(key);
            if(it != _map.end()){
                it->second->add(sp);
            }else{
                auto val = sk_make_sp<GroupS>();
                val->prepareSize(8);
                val->add(sp);
                _map[key] = val;
            }
         }
    }

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

//----------------------------------------------------------------------
template<typename E>
void TabContentDelegate::visitGroupOMP(ListI* idxes, int psize,
                   std::function<void(ListS*, String&)> group,
                   std::function<E(const String&, sk_sp<GroupS>&)> visitor,
                   IColumn<sk_sp<GRetItem<E>>>& out){
    std::vector<sk_sp<_GItem>> items;
    {
    GroupS gs(getRowCount());
    if(idxes == nullptr || idxes->size() == 0){
        getRows(gs);
    }else{
        int rc = getRowCount();
        int cc = getColumnCount();
        gs.resize(rc);
#pragma omp parallel for
        for(int i = 0 ; i < rc ; ++i){
            sk_sp<ListS> ls = sk_make_sp<ListS>(cc);
            get(i, idxes, *ls);
            gs.set0(i, ls);
        }
    }
    //
    int rc = gs.size();
    ListS ls;
    ls.resize(rc);
#pragma omp parallel for
    for(int i = 0 ; i < rc ; ++i){
        String key;
        group(gs.get(i).get(), key);
        ls.set0(i, std::move(key));
    }
    std::unordered_map<String, sk_sp<_GItem>> _map;
    _map.reserve(rc / 3);
    items.reserve(rc / 3);
    for(int i = 0 ; i < rc ; ++i){
        auto& key = ls.get(i);
        auto it = _map.find(key);
        if(it != _map.end()){
            it->second->add(gs.get(i));
        }else{
            auto val = sk_make_sp<_GItem>(key, psize);
            val->add(gs.get(i));
            items.push_back(val);
            _map[key] = val;
        }
    }
    }
    int _size = items.size();
    out.resize(_size);
#pragma omp parallel for
    for(int i = 0 ; i < _size ; ++i){
        auto& key = items[i]->key;
        sk_sp<GRetItem<E>> item = sk_make_sp<GRetItem<E>>(key);
        item->ret = visitor(key, items[i]->gs);
        out.set0(i, item);
    }
}

}
