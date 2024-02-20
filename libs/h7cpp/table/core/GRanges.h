#pragma once

#include <unordered_map>
#include "table/Column.h"
#include "table/HashMap.h"

namespace h7 {

class GRanges: public SkRefCnt
{
public:
    GRanges(){
        this->m_keys = sk_make_sp<ListS>();
        this->m_starts = sk_make_sp<ListI>();
        this->m_ends = sk_make_sp<ListI>();
    }
    GRanges(sk_sp<ListS> m_keys, sk_sp<ListI> m_starts, sk_sp<ListI> m_ends):
        m_keys(m_keys), m_starts(m_starts), m_ends(m_ends){
        MED_ASSERT(m_starts->size() == m_ends->size());
    }
    GRanges(sk_sp<ListI> m_starts, sk_sp<ListI> m_ends):
        m_starts(m_starts), m_ends(m_ends){
        MED_ASSERT(m_starts->size() == m_ends->size());
        this->m_keys = sk_make_sp<ListS>(m_starts->size(), true);
    }
    GRanges(Cons_Ref(ListS) m_keys, Cons_Ref(ListI) m_starts, Cons_Ref(ListI) m_ends){
        MED_ASSERT(m_starts.size() == m_ends.size());
        MED_ASSERT_IF(!m_keys.isEmpty(), m_keys.size() == m_starts.size());
        if(m_keys.isEmpty()){
            this->m_keys = sk_make_sp<ListS>(m_starts.size(), true);
        }else{
            this->m_keys = sk_make_sp<ListS>(m_keys);
        }
        this->m_starts = sk_make_sp<ListI>(m_starts);
        this->m_ends = sk_make_sp<ListI>(m_ends);
    }
    GRanges(const GRanges& oth): m_keys(oth.m_keys),
        m_starts(oth.m_starts), m_ends(oth.m_ends){
    }
    inline void addRow(CString key, int start, int end){
        m_keys->add(key);
        m_starts->add(start);
        m_ends->add(end);
    }
    inline void addRowAsFull(sk_sp<h7::ListS> sp){
        if(sp->size() < 3){
            PRINTERR("GRanges >> called addRowAsFull(): data error.\n");
        }else{
            int start = (sp->get(1).empty() || sp->get(1) == "-") ?
                        INT_MIN : std::stoi(sp->get(1));
            int end = (sp->get(2).empty() || sp->get(2) == "-") ?
                        INT_MIN : std::stoi(sp->get(2));
            m_keys->add(sp->get(0));
            m_starts->add(start);
            m_ends->add(end);
        }
    }

    inline void setName(CString name){
        this->m_name = name;
    }
    inline String getName()const{
        return this->m_name;
    }
    inline sk_sp<h7::ListS> keys(){
        return m_keys;
    }
    inline sk_sp<h7::ListI> starts(){
        return m_starts;
    }
    inline sk_sp<h7::ListI> ends(){
        return m_ends;
    }
    inline int getRowCount(){
        return m_keys->size();
    }
    inline sk_sp<h7::GRanges> copy(){
        auto sp = sk_make_sp<GRanges>(m_keys->copy(), m_starts->copy(), m_ends->copy());
        sp->m_name = this->m_name;
        return sp;
    }
    void offset(int start_offset, int end_offset);
    inline void offset(int _offset){
        offset(_offset, _offset);
    }

    bool isInAnyRange(CString key, long long val);

    sk_sp<h7::ListS> getFullRowData(int idx);

    sk_sp<h7::GRanges> sub(h7::ListI* rowIndexes);
    inline sk_sp<h7::GRanges> sub(sk_sp<ListI> rowIndexes){
        return sub(rowIndexes.get());
    }

    sk_sp<ListS> concat(CString sep);

    //as two column data
    sk_sp<MatrixI> rFindOverlaps(h7::GRanges* other, bool requireOrder = false);

    inline sk_sp<MatrixI> rFindOverlaps(sk_sp<h7::GRanges> other, bool requireOrder = false){
        return rFindOverlaps(other.get(), requireOrder);
    }

    sk_sp<h7::MatrixI> rFindOverlaps2(h7::GRanges* other, bool requireOrder = false);

    inline sk_sp<h7::MatrixI> rFindOverlaps2(sk_sp<h7::GRanges> other, bool requireOrder = false){
        return rFindOverlaps2(other.get(), requireOrder);
    }

    //--------------------------------------------
    void buildMainCache();
    void buildCache();
    void destroyCache();

private:
    struct RPair{
        int i;
        int j;
        RPair(int i,int j):i(i), j(j){}
    };

    String m_name;
    sk_sp<h7::ListS> m_keys;
    sk_sp<h7::ListI> m_starts;
    sk_sp<h7::ListI> m_ends;

    sk_sp<h7::ListI> m_mainHashess;
    sk_sp<h7::ListI> m_keyHashes;
    h7::HashMap<int,sk_sp<h7::ListI>> m_indexMap;

    void _processOverlap(std::vector<std::vector<RPair>>& out, int i, int _idx,
                         int s1_start, int s1_end, GRanges* oth);
};

}
