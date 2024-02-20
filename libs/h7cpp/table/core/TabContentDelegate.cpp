
#include "table/core/TabContentDelegate.h"
#include "table/SetColumn.h"
#include "utils/convert.hpp"
//#include "utils/BubbleSort.hpp"
#include "utils/string_utils.hpp"

namespace h7 {

int TabContentDelegate::getRowCount(){
    return m_columns.size() > 0 ? m_columns.get(0)->size() : 0;
}
int TabContentDelegate::getRowCount()const{
    return m_columns.size() > 0 ? m_columns.const_get(0)->size() : 0;
}
int TabContentDelegate::getColumnCount()const{
    return m_columns.size();
}

int TabContentDelegate::computeHash(int row_idx, int col_idx){
    return hashImpl<String>(m_columns.get(col_idx)->get(row_idx));
}
int TabContentDelegate::computeHash(int row_idx, ListI* col_idxes){
    std::vector<String*> vec;
    if(col_idxes){
        for(int i = 0 ; i < col_idxes->size() ; ++i){
            vec.push_back(&m_columns.get(col_idxes->get(i))->get(row_idx));
        }
    }else{
        int size = m_columns.size();
        for(int i = 0 ; i < size ; ++i){
            vec.push_back(&m_columns.get(i)->get(row_idx));
        }
    }
    int result = 1;
    int size = vec.size();
    for (int i = 0; i < size; ++i ) {
         result = 31 * result + hashImpl<String>(*vec[i]);
    }
    return result;
}

sk_sp<ListS> TabContentDelegate::get(int row_idx, ListI* col_idxes){
    MED_ASSERT(row_idx < getRowCount());
    sk_sp<ListS> ret = sk_make_sp<ListS>();
    //empty means all
    if(col_idxes == nullptr || col_idxes->size() == 0){
        int s = m_columns.size();
        for(int i = 0 ; i < s ; ++i){
            ret->add(m_columns.get(i)->get(row_idx));
        }
    }else{
        int s = col_idxes->size();
        for(int i = 0 ; i < s ; ++i){
            ret->add(m_columns.get(col_idxes->get(i))->get(row_idx));
        }
    }
    return ret;
}

void TabContentDelegate::get(int row_idx, ListI* col_idxes, ListS& out){
    MED_ASSERT(row_idx < getRowCount());
    if(col_idxes == nullptr || col_idxes->size() == 0){
        int s = m_columns.size();
        out.prepareSize(s);
        for(int i = 0 ; i < s ; ++i){
            out.add(m_columns.get(i)->get(row_idx));
        }
    }else{
        int s = col_idxes->size();
        out.prepareSize(s);
        for(int i = 0 ; i < s ; ++i){
            out.add(m_columns.get(col_idxes->get(i))->get(row_idx));
        }
    }
}

sk_sp<ListS> TabContentDelegate::get(ListI* row_idxes, int col_idx, bool copy){
    MED_ASSERT(col_idx < getColumnCount());
    if(row_idxes == nullptr){
        if(copy){
            return m_columns.get(col_idx)->copy();
        }
        return m_columns.get(col_idx);
    }else{
        int s = row_idxes->size();
        sk_sp<ListS> ret = sk_make_sp<ListS>(s);
        for(int i = 0 ; i < s ; ++i){
            ret->add(m_columns.get(col_idx)->get(row_idxes->get(i)));
        }
        return ret;
    }
}

String& TabContentDelegate::get(int row_idx, int col_idx){
    MED_ASSERT(row_idx < getRowCount());
    MED_ASSERT(col_idx < getColumnCount());
    return m_columns.get(col_idx)->get(row_idx);
}

void TabContentDelegate::set(int row_idx, int col_idx, const String& val){
    MED_ASSERT(row_idx < getRowCount());
    MED_ASSERT(col_idx < getColumnCount());
    m_columns.get(col_idx)->set0(row_idx, val);
}

sk_sp<ListS> TabContentDelegate::getRow(int row_idx){
    MED_ASSERT(row_idx < getRowCount());
    int cc = getColumnCount();
    sk_sp<ListS> ret = sk_make_sp<ListS>(cc);
    for(int i = 0 ; i < cc ; ++i){
        ret->add(m_columns.get(i)->get(row_idx));
    }
    return ret;
}
String TabContentDelegate::getRowToStr(int row_idx, CString seq){
    String str;
    str.reserve(256);
    int cc = getColumnCount();
    for(int i = 0 ; i < cc ; ++i){
        str += m_columns.get(i)->get(row_idx);
        if(i != cc - 1){
            str += seq;
        }
    }
    return str;
}
void TabContentDelegate::getRow(int row_idx, ListS* out){
    MED_ASSERT(row_idx < getRowCount());
    int cc = getColumnCount();
    out->prepareSize(cc);
    for(int i = 0 ; i < cc ; ++i){
        out->add(m_columns.get(i)->get(row_idx));
    }
}

sk_sp<ListS> TabContentDelegate::getColumn(int col_idx, bool copy){
    MED_ASSERT(col_idx < getColumnCount());
    return copy ? m_columns.get(col_idx)->copy() : m_columns.get(col_idx);
}

sk_sp<ListS> TabContentDelegate::optColumn(int col_idx, bool copy){
    if(col_idx >= getColumnCount()){
        return nullptr;
    }
    return copy ? m_columns.get(col_idx)->copy() : m_columns.get(col_idx);
}

sk_sp<ListS> TabContentDelegate::getColumnAsUnique(int col_idx){
    MED_ASSERT(col_idx < getColumnCount());
    auto& col = m_columns.get(col_idx);
    sk_sp<ListS> ret = sk_make_sp<ListS>(col->size() / 2);
    SetColumn<String> set;
    for (int i = 0, size = col->size(); i < size; ++i) {
        String& str = col->get(i);
        if(!set.contains(str)){
            set.add(str);
            ret->add(str);
        }
    }
    return ret;
}

sk_sp<h7::SetColumn<String>> TabContentDelegate::getColumnAsSet(int col_idx){
    MED_ASSERT(col_idx < getColumnCount());
    auto& col = m_columns.get(col_idx);
    sk_sp<SetS> ret = sk_make_sp<SetS>(col->size() / 2);
    for (int i = 0, size = col->size(); i < size; ++i) {
        String& str = col->get(i);
        ret->add(str);
    }
    return ret;
}

sk_sp<ListI> TabContentDelegate::getIntColumn(int col_idx){
     MED_ASSERT(col_idx < getColumnCount());
    return m_columns.get(col_idx)->asInt();
}
void TabContentDelegate::getColumn(int col_idx, ListS& out){
     MED_ASSERT(col_idx < getColumnCount());
    m_columns.get(col_idx)->copyTo(&out);
}

void TabContentDelegate::prepareColumnCount(int size){
    if(m_columns.size() < size){
        m_columns.prepareSize(size);
        int rc = getRowCount();
        int diff = size - m_columns.size();
        for(int i = 0 ; i < diff ; ++i){
            m_columns.add(sk_make_sp<ListS>(rc, true));
        }
    }else if(m_columns.size() > size){
        int diff = m_columns.size() - size;
        for(int i = 0 ; i < diff ; ++i){
            m_columns.removeAt0(m_columns.size() - 1);
        }
    }
}
void TabContentDelegate::prepareRowCount(int size){
    int s = m_columns.size();
    for(int i = 0 ; i < s ; ++i){
        m_columns.get(i)->prepareSize(size);
    }
}
void TabContentDelegate::getRows(GroupS& list, ListI* colIdxes){
    if(colIdxes == nullptr || colIdxes->isEmpty()){
        getRows(list);
        return;
    }
    int rc = getRowCount();
    int col_size = colIdxes->size();
    list.resize(rc);
#pragma omp parallel for
    for(int i = 0 ; i < rc ; ++i){
        sk_sp<ListS> ret = sk_make_sp<ListS>();
        for(int j = 0 ; j < col_size; ++j ){
            ret->add(m_columns[j]->get(i));
        }
        list.set0(i, ret);
    }
}
void TabContentDelegate::getRows(GroupS& list){
    int rc = getRowCount();
    int cc = getColumnCount();
    list.resize(rc);
#pragma omp parallel for
    for(int i = 0 ; i < rc ; ++i){
        sk_sp<ListS> ret = sk_make_sp<ListS>();
        for(int j = 0 ; j < cc ; ++j){
            ret->add(m_columns[j]->get(i));
        }
        list.set0(i, ret);
    }
}
void TabContentDelegate::setRows(GroupS& _list){
    GroupS& list = _list;
    int rc = list.size();
    m_columns.clear();
    if(rc > 0){
        int cc = list.get(0)->size();
        prepareColumnCount(cc);
        prepareRowCount(rc);
#pragma omp parallel for
        for(int i = 0 ; i < cc ; ++i){
            auto& col = m_columns.get(i);
            for(int k = 0 ; k < rc ; ++k){
                col->add(list.get(k)->get(i));
            }
        }
    }
}
void TabContentDelegate::setRows(GroupS* list){
    int rc = list->size();
    m_columns.clear();
    if(rc > 0){
        int cc = list->get(0)->size();
        prepareColumnCount(cc);
        prepareRowCount(rc);
#pragma omp parallel for
        for(int i = 0 ; i < cc ; ++i){
            auto& col = m_columns.get(i);
            for(int k = 0 ; k < rc ; ++k){
                col->add(list->get(k)->get(i));
            }
        }
    }
}

void TabContentDelegate::removeRow(int row_idx){
    MED_ASSERT(row_idx < getRowCount());
    int cc = m_columns.size();
    for(int i = 0 ; i < cc ; ++i){
        m_columns.get(i)->removeAt0(row_idx);
    }
}

void TabContentDelegate::removeColumn(int col_idx){
    MED_ASSERT(col_idx < getColumnCount());
    m_columns.removeAt0(col_idx);
}

void TabContentDelegate::addContentByRow(TabContentDelegate* sp){
    if(sp->getColumnCount() == 0){
       return;
    }
    if(getColumnCount() == 0){
       int cc = sp->m_columns.size();
       for(int i = 0 ; i < cc ; ++i){
           m_columns.add(sp->m_columns.get(i)->copy());
       }
       return;
    }
    MED_ASSERT(getColumnCount()== sp->getColumnCount());
    int cc = getColumnCount();
#pragma omp parallel for
    for(int i = 0 ; i < cc ; ++i){
        int diff = sp->m_columns.get(i)->size();
        m_columns.get(i)->prepareDiffSize(diff);
        m_columns.get(i)->addAll(sp->m_columns.get(i));
    }
}

void TabContentDelegate::addContentByColumn(TabContentDelegate* sp){
    int cc = sp->getColumnCount();
    m_columns.prepareSize(cc);
    for(int i = 0 ; i < cc ; ++i){
        m_columns.add(sp->m_columns.get(i));
    }
}
void TabContentDelegate::addRowAsFull(std::vector<String>& list){
    //ensure column count
    int rc = getRowCount();
    int diff = list.size() - m_columns.size();
    for(int i = 0 ; i < diff ; ++i){
        m_columns.add(sk_make_sp<ListS>());
    }
    //add
    int cc = list.size();
    for(int i = 0 ; i < cc ; ++i){
        m_columns.get(i)->resize(rc);
        m_columns.get(i)->add(list[i]);
    }
}
void TabContentDelegate::addRowAsFullDirect(std::vector<String>& list){
    MED_ASSERT(getColumnCount() == (int)list.size());
    int cc = list.size();
    for(int i = 0 ; i < cc ; ++i){
        m_columns.get(i)->add(list[i]);
    }
}
void TabContentDelegate::addRowAsFull(ListS* list){
    //ensure column count
    int rc = getRowCount();
    int diff = list->size() - m_columns.size();
    for(int i = 0 ; i < diff ; ++i){
        m_columns.add(sk_make_sp<ListS>());
    }
    //add
    int cc = list->size();
    for(int i = 0 ; i < cc ; ++i){
        m_columns.get(i)->resize(rc);
        m_columns.get(i)->add(list->get(i));
    }
}

void TabContentDelegate::swapColumn(int idx1, int idx2){
    m_columns.swap(idx1, idx2);
}
void TabContentDelegate::swapRow(int idx1, int idx2){
    int cc = getColumnCount();
    for(int i = 0 ; i < cc ; ++i){
        m_columns.get(i)->swap(idx1, idx2);
    }
}

void TabContentDelegate::sortRow(int index, std::function<int(String&, String&)> func,
                                 bool desc){
    GroupS gs(getRowCount());
    getRows(gs);
    std::sort(gs.list.begin(), gs.list.end(), [index, desc, &func](
              sk_sp<ListS>& l1,  sk_sp<ListS>& l2
              ){
        auto& s1 = l1->get(index);
        auto& s2 = l2->get(index);
        auto ret = func(s1, s2);
        return desc ? ret > 0 : ret < 0;
    });
    setRows(gs);
}
void TabContentDelegate::sortRow(std::function<int(ListS*, ListS*)> func){
    GroupS group(getRowCount());
    getRows(group);
    using SPList = sk_sp<ListS>;
    std::sort(group.list.begin(), group.list.end(), [func](SPList l1, SPList l2){
        return func(l1.get(), l2.get()) < 0;
    });
    setRows(group);
}

void TabContentDelegate::sortRowByFloatColumn(int col_index,
                        std::function<int(float, float)> func){
    GroupS group;
    getRows(group);
    //
    using SPList = sk_sp<ListS>;
    std::sort(group.list.begin(), group.list.end(),
              [col_index, func](SPList l1, SPList l2){
        auto& v1 = l1->get(col_index);
        auto& v2 = l2->get(col_index);
        return func(utils::getFloat(v1), utils::getFloat(v2)) < 0;
    });
    setRows(group);
}

void TabContentDelegate::sortRowByFloatColumn(int col_index, bool aesc){
    if(aesc){
        std::function<int(float, float)> func = [](float a, float b){
            return a > b ? 1 : (a < b ? -1 : 0);
        };
        sortRowByFloatColumn(col_index, func);
    }else{
        std::function<int(float, float)> func = [](float a, float b){
            return a > b ? -1 : (a < b ? 1 : 0);
        };
        sortRowByFloatColumn(col_index, func);
    }
}

sk_sp<TabContentDelegate> TabContentDelegate::copy(){
    sk_sp<TabContentDelegate> ret = sk_make_sp<TabContentDelegate>();
    int cc = getColumnCount();
    ret->m_columns.resize(cc);
#pragma omp parallel for
    for(int i = 0 ; i < cc ; ++i){
        ret->m_columns.set0(i, m_columns.get(i)->copy());
    }
    return ret;
}

void TabContentDelegate::align(int expectColumnCount){
    //align column
    if(expectColumnCount > 0){
        int size = m_columns.size();
        if(size < expectColumnCount){
            int c = expectColumnCount - size;
            for (int i = 0; i < c; ++i) {
                m_columns.add(sk_make_sp<ListS>());
            }
        }else if( size > expectColumnCount){
            m_columns.resize(expectColumnCount);
        }
   }
   int size = m_columns.size();
    //align row
   int maxSize = 0;
   for (int i = 0; i < size; ++i) {
       maxSize = HMAX(maxSize, m_columns.get(i)->size());
   }
   if(maxSize > 0){
       for (int i = 0; i < size; ++i) {
           m_columns.get(i)->resize(maxSize);
       }
   }
}

const String TabContentDelegate::toString(bool oneLine){
    std::stringstream out;
    if(oneLine){
        out << "[";
        int rc = getRowCount();
        for(int i = 0 ; i < rc ; ++i){
            out << getRow(i)->toStringFullly();
            if(i != rc - 1){
                out << ", ";
            }
        }
    }else{
        out << "[" << h7::utils::newLineStr();
        int rc = getRowCount();
        for(int i = 0 ; i < rc ; ++i){
            out << getRow(i)->toStringFullly();
            if(i != rc - 1){
                out << "," << h7::utils::newLineStr();
            }
        }
    }
    out << "]";
    return out.str();
}

}
