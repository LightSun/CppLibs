#pragma once

#include <vector>
#include <algorithm>
#include <functional>

/*
* 基于vector的二分查找 key-value结构
*/
template <typename Key, typename Value, typename Compare = std::less<Key>,
         typename Alloc = std::allocator<std::pair<Key, Value> > >
class CVectorMap : public std::vector<std::pair<Key, Value>, Alloc >
{
    using Elem = std::pair<Key, Value>;
    using ContainType = std::vector<Elem, Alloc >;
    //! 比较函数
    Compare m_keyCompare;
    class ValCompare
    {
        public:
        Compare key_compare;
        ValCompare(Compare keyComp) : key_compare(keyComp) {}
        //必须是const引用  不然编译有问题
        bool operator () (const Elem& elem1, const Elem& elem2)
        {
            return key_compare(elem1.first, elem2.first);
        }
    };
    ValCompare m_val_compare{m_keyCompare};

public:
    CVectorMap() { }
    CVectorMap(std::initializer_list<Elem>&& values, Alloc&& _alloc = Alloc())
     : std::vector<Elem, Alloc>(values, _alloc)
    { }

public:
    //添加. false for insert failed.
    bool push_back(Key&& _key, Value&& val)
    {
        //插入前先判断是否有重复元素
        Elem _element = std::make_pair<Key, Value>(std::forward<Key>(_key), std::forward<Value>(val));
        ContainType::emplace_back(std::move(_element));
        typename ContainType::iterator lower = std::lower_bound(ContainType::begin(), ContainType::end(), ContainType::back(), m_val_compare);
        if (*lower == ContainType::back() && lower != ContainType::end() - 1)   //有重复元素
        {
            ContainType::pop_back();
            return false;
        }
        if (lower == ContainType::end() - 1)  //最后一个 插入的就是当前最大的
            return true;
        //将末尾元素（新插入）通过反向旋转移动到 lower 位置，其余元素依次后移
        std::rotate(ContainType::rbegin(), ContainType::rbegin() + 1, typename ContainType::reverse_iterator(lower));
        return true;
    }

    bool emplace(Key&& _key, Value&& val)
    {
        return push_back(std::forward<Key>(_key), std::forward<Value>(val));
    }

    //查找
    typename ContainType::iterator find(const Key& _key)
    {
        auto iter_lower = std::lower_bound(ContainType::begin(), ContainType::end(), _key,
                                           [&](const Elem& element, const Key& comp_key){return m_keyCompare(element.first, comp_key);});
        if (ContainType::end() != iter_lower && iter_lower->first == _key)
            return iter_lower;
        return ContainType::end();
    }

    typename ContainType::iterator end()
    {
        return ContainType::end();
    }

    Value& operator[](const Key& _key)
    {
        static Value val;
        typename ContainType::iterator iter = find(_key);
        return iter != ContainType::end() ? iter->second : val;
    }

    //更新
    bool update(const Key& _key, const Value& val)
    {
        auto iter = find(_key);
        if (iter != ContainType::end())
        {
            iter->second = val;
            return true;
        }
        return false;
    }

    //删除
    bool erase(const Key& _key)
    {
        auto iter = find(_key);
        if (iter != ContainType::end())
        {
            ContainType::erase(iter);
            return true;
        }
        return false;
    }
};
