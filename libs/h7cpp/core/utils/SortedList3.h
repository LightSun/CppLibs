#pragma once

#include <vector>
#include <queue>
#include <algorithm>

namespace h7 {

template<typename T, typename _Compare = std::less<typename std::vector<T>::value_type>>
class SortedList3{
public:
    SortedList3(bool autoTrim):m_autoTrim(autoTrim){
    }
    void setMaxSize(int max_size){
        m_maxSize = max_size;
    }
    void add(const T& t){
        m_list.push_back(t);
        std::sort(m_list.begin(), m_list.end(), m_cmp);
        if(m_autoTrim){
            trimToSize();
        }
    }
    int size()const{return m_list.size();}

    void removeAt(int idx){
        if(idx < m_list.size()){
            m_list.erase(m_list.begin() + idx);
        }
    }
    T& get(int idx){return m_list[idx];}
    std::vector<T>& getVector(){
        return m_list;
    }
    void trimToSize(){
        //trim tail
        if(m_maxSize > 0 && m_list.size() > m_maxSize){
            m_list.resize(m_maxSize);
        }
    }
private:
    std::vector<T> m_list;
    _Compare m_cmp;
    int m_maxSize {0};
    bool m_autoTrim {true};
};
}
