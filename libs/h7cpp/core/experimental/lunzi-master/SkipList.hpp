#include <stdint.h>
#include <stdlib.h>
#include <functional>
#include <mutex>
#include <string.h>

/*
struct TestKey
{
      int k1;
      int k2;
      TestKey& operator=(const TestKey& k)
      {
            k1 = k.k1;
            k2 = k.k2;
      }
};

//升序
ValComp fun1 = [](const TestKey& k1, const TestKey& k2)
{ 
     if (k1.k1 != k2.k1)
        return k1.k1 > k2.k1 ? -1 : 0;
    if (k1.k2 != k2.k2)
        return k1.k2 > k2.k2 ? -1 : 0;
     return 0;
}

//降序
ValComp fun1 = [](const TestKey& k1, const TestKey& k2)
{ 
     if (k1.k1 != k2.k1)
        return k1.k1 < k2.k1 ? -1 : 0;
    if (k1.k2 != k2.k2)
        return k1.k2 < k2.k2 ? -1 : 0;
     return 0;
}
*/

namespace PSK0
{

constexpr int max_skip_list_level = 32;                //最大总层数 容量 2^32
constexpr float  SKIPLISTPARAM = 0.25;

//概率分层 尽量使得建索引时 每个节点的在各层的概率是均衡的
static int randomSkipListLevel()
{
    int level = 1;
    while ((rand() & 0xFFFF) < (SKIPLISTPARAM * 0xFFFF))
        level += 1;
    return (level < max_skip_list_level) ? level : max_skip_list_level;
}

template <typename VALUE>
struct CISkipListNode;

//层
template <typename VALUE>
struct CISkipNodeLevel
{
    CISkipListNode<VALUE>* next = NULL;           //同层起点 --- 实际是最低层链表节点的指针副本
};

template <typename VALUE>
struct CISkipListNode
{
    typedef CISkipNodeLevel<VALUE> LvType;
    VALUE val;
    CISkipListNode* next = NULL;
    //    int level;                   //不需要 因为查找时总是从高层向低层  不会有边界问题
    CISkipNodeLevel<VALUE> levels[];
};

template <typename VALUE>
struct SkipValComp
{
    typedef std::function<int(const VALUE& refer, const VALUE& real)> ValComp;
};

template <typename KEY, typename VALUE>
class CISkipList
{
public:
    typedef std::pair<KEY, VALUE> _Node;
    typedef CISkipListNode<_Node> _SKNode;
public:
    int level;
    int length;
    _SKNode* head;
    _SKNode* tail;
    typename SkipValComp<KEY>::ValComp valCompare;    //值类型比较函数 外部定义
public:
    CISkipList(typename SkipValComp<KEY>::ValComp&& fun) : level(0), length(0), tail(NULL), valCompare(fun)
    {
        head = createSkipNode(max_skip_list_level, _Node());
        typename _SKNode::LvType* levels = (typename _SKNode::LvType*)(head + 1);
        for (int i = 0; i < max_skip_list_level; ++i)
        {
            levels[i].next = NULL;
        }
    }
public:
    //迭代器
    struct Iterator
    {
        _SKNode* node = NULL;
        Iterator(_SKNode* pNode) : node(pNode)
        {}
        _SKNode* operator++()
        {
            if (node)
            {
                typename _SKNode::LvType* levels_ = (typename _SKNode::LvType*)(node + 1);
                node = levels[0]_.next;
            }
            return node;
        }
        bool operator != (const Iterator& iter)
        {
            return this->node != iter.node ? true : false;
        }
        bool operator -= (const Iterator& iter)
        {
            return this->node == iter.node ? true : false;
        }
        _SKNode& operator*()
        {
            return *node;
        }
    };

    Iterator begin()
    {
        typename _SKNode::LvType* levels_ = (typename _SKNode::LvType*)(head + 1);
        return Iterator(levels_[0].next);
    }

    Iterator end()
    {
        return Iterator(NULL);
    }
    
    Iterator erase()
    {
        typename _SKNode::LvType* levels_ = (typename _SKNode::LvType*)(iter.node + 1);
        Iterator iter_(levels_[0].next);
        remove(std::forward<KEY>(iter.node->val.first), NULL);
        return iter_;
    }

    Iterator pos(KEY&& k)
    {
        return Iterator(find(std::forward<KEY>(k)));
    }

    template <typename F>
    Iterator pos(KEY&& k, F&& f_)
    {
        return Iterator(find(std::forward<KEY>(k), f_));
    }
public:
    // 创建节点
    _SKNode* createSkipNode(int iLevel, _Node&& val)
    {
        int nSize = sizeof(_SKNode) + iLevel * sizeof(CISkipNodeLevel<_Node>);
        _SKNode* pNode = (_SKNode*)malloc(nSize);
        memset(pNode, 0, nSize);
        pNode->val = val;
        return pNode;
    }

    void insert(KEY&& k, VALUE&& val)
    {
        _SKNode* node = head;
        _SKNode* update[max_skip_list_level] = { 0 };
        for (int i = max_skip_list_level - 1; i >= 0; --i)
        {
            typename _SKNode::LvType* levels = (typename _SKNode::LvType*)(node + 1);
            while (levels[i].next && valCompare(std::forward<KEY>(k), levels[i].next->val.first) < 0)
            {
                node = levels[i].next;
            }
            update[i] = node;
        }
        int nLevel = randomSkipListLevel();
        if (nLevel > this->level)
        {
            for (int i = this->level; i < nLevel; ++i)
            {
                update[i] = head;
            }
            this->level = nLevel;
        }
        //创建节点并插入
        node = createSkipNode(nLevel, std::make_pair(std::forward<KEY>(k), std::forward<VALUE>(val)));
        for (int i = 0; i < nLevel; ++i)     //每一层的链表都得变动
        {
            typename _SKNode::LvType* levels_ = (typename _SKNode::LvType*)(node + 1);
            typename _SKNode::LvType* levels_1 = (typename _SKNode::LvType*)(update[i] + 1);
            levels_[i].next = levels_1[i].next;
            levels_1[i].next = node;
        }
        ++this->length;
    }

    bool remove(KEY&& k, _SKNode** pNode)
    {
        _SKNode* node = head;
        _SKNode* update[max_skip_list_level] = { 0 };
        //确定需要操作的层
        for (int i = this->level - 1; i >= 0; --i)
        {
            typename _SKNode::LvType* levels_ = (typename _SKNode::LvType*)(node + 1);
            while (levels_[i].next && valCompare(std::forward<KEY>(k), levels_[i].next->val.first) < 0)
            {
                node = levels_[i].next;
                levels_ = (typename _SKNode::LvType*)(node + 1);
            }
            update[i] = node;
        }
        //确定删除节点
        typename _SKNode::LvType* levels_ = (typename _SKNode::LvType*)(node + 1);
        node = levels_[0].next;
        if (node && valCompare(node->val.first, std::forward<KEY>(k)) == 0)
        {
            removeNode(node, update);
            if (!pNode)
                freeNode(node);
            else
                *pNode = node;
            return true;
        }
        return false;
    }

    //联合键的左前缀查找
    template <typename F>
    _SKNode* find(KEY&& k, F&& f_)
    {
        _SKNode* node = head;
        //确定需要操作的层
        for (int i = this->level - 1; i >= 0; --i)
        {
            typename _SKNode::LvType* levels_ = (typename _SKNode::LvType*)(node + 1);
            while (levels_[i].next && f_(std::forward<KEY>(k), levels_[i].next->val.first) < 0)
            {
                node = levels_[i].next;
                levels_ = (typename _SKNode::LvType*)(node + 1);
            }
        }
        //确定删除节点
        typename _SKNode::LvType* levels_ = (typename _SKNode::LvType*)(node + 1);
        node = levels_[0].next;
        if (node && f_(node->val.first, std::forward<KEY>(k)) == 0)
        {
            return node;
        }
        return NULL;
    }

    _SKNode* find(KEY&& k)
    {
        return find(std::forward<KEY>(k), valCompare);
    }

    std::pair<_SKNode*, _SKNode* > findRange(KEY&& k1, KEY&& k2)
    {
        std::pair<_SKNode*, _SKNode* > pair;
        _SKNode* pFirst = find(std::forward<KEY>(k1));
        _SKNode* pSecond = find(std::forward<KEY>(k2));
        return std::pair<_SKNode*, _SKNode* >(pFirst, pSecond);
    }

    void release(_SKNode* node)
    {
        freeNode(node);
    }
private:
    //remove调用的内部函数
    void removeNode(_SKNode* node, _SKNode** update)
    {
        for (int i = 0; i < this->level; i++)                //每层都要逻辑移除
        {
            if (update[i]->levels[i].next == node)
            {
                update[i]->levels[i].next = node->levels[i].next;
            }
        }
        while (this->level > 1 && this->head->levels[this->level - 1].next == NULL)
        {
            this->level--;
        }
        this->length--;
    }
    //释放节点
    void freeNode(_SKNode* node)
    {
        free(node);
    }
};

template <typename KEY, typename VALUE>
CISkipList<KEY, VALUE>* CreateSkipList(typename SkipValComp<KEY>::ValComp&& fun)
{
    CISkipList<KEY, VALUE>* pList = new CISkipList<KEY, VALUE>(std::move(fun));
    return pList;
}

}
