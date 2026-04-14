#include <stdint.h>
#include <stdlib.h>
#include <functional>
#include <atomic>
#include <string.h>
#include <vector>
#include <thread>

namespace PSK2
{
constexpr int max_sk_level = 32;                //最大总层数 容量 2^32
constexpr float  _param = 0.25;

//概率分层 尽量使得建索引时 每个节点的在各层的概率是均衡的
inline int randomSkLevel()
{
    int level = 1;
    while ((rand() & 0xFFFF) < (_param * 0xFFFF))
        level += 1;
    return (level < max_sk_level) ? level : max_sk_level - 1;
}

enum RstFlag
{
    Failed = -1,
    Success = 0
};

enum SkNodeFlag
{
    RemoveMark = 0x10,       //删除
    StayMark = 0x0100,       //存在
};

enum LockStatus
{
    UnLock = 0x00,
    Lock = 0x01,
};

class CSkLock
{
    std::atomic<int> lock_ = {UnLock};
public:
    void lock()
    {
        int ulock_ = UnLock;
        bool take_ = false;
        while (!take_)
        {
            for (int step = 0; step <= 100000 && !(take_ = lock_.compare_exchange_strong(ulock_, Lock, std::memory_order_release)); ++step)
                ulock_ = UnLock;
            if (!take_)
                std::this_thread::yield();
        }
    }

    void unlock()
    {
        lock_.store(UnLock, std::memory_order_release);
    }
};

template <typename T, typename LOCK = CSkLock>
struct SkNode
{
    typedef SkNode<T,LOCK> InType;
    int level = 0;
    std::atomic<int> flag = {0};
    LOCK lock_;
    T val;
    SkNode* wall[];
};

template <typename T>
inline void setLayerNode(T* n, int layer, T* n1)
{
    typename T::InType** wall_ = (typename T::InType**)(n + 1);
    wall_[layer] = n1;
    std::atomic_thread_fence(std::memory_order_release);
}

template <typename T>
inline T* layerNode(T* n, int layer)
{
    typename T::InType** wall_ = (typename T::InType**)(n + 1);
    std::atomic_thread_fence(std::memory_order_acquire);
    T* n_ = wall_[layer];
    return n_;
}

template <typename Node>
inline bool lock(Node* n)
{
    n->lock_.lock();
    return true;
}

template <typename Node>
inline void unLock(Node* n)
{
    n->lock_.unlock();
}

template <typename Node>
inline bool isLock(Node* n)
{
    return n->lock_.load(std::memory_order_acquire) & Lock;
}

template <typename Node>
inline int getFlag(Node* n)
{
    return n->flag.load(std::memory_order_acquire);
}

template <typename Node>
inline void setFlag(Node* n, int f)
{
    n->flag.store(getFlag(n) | f, std::memory_order_release);
}

template <typename Node>
inline bool hasThisFlag(Node* n, int f)
{
    return getFlag(n) & f;
}

template <typename Node, typename DeAlloc>
struct SkDeAlloc
{
    DeAlloc deAllocFun;
public:
    void dealloc(Node* n)
    {
        deAllocFun(n);
    }
};

template <typename Node>
struct DftDealloc
{
    void operator()(Node* n)
    {
        free(n);
    }
};

template <typename Node, typename LOCK = CSkLock>
class SkNodeRecycle
{
public:
    typedef std::function<void(Node* )> _NodeRel;
    std::atomic<int> m_ref;
    LOCK m_lock;
    std::vector<Node* > m_nodes;
    _NodeRel m_dealloc;
public:
    SkNodeRecycle() : m_ref(0), m_dealloc(NULL)
    {
        m_nodes.reserve(max_sk_level);
    }
    SkNodeRecycle(_NodeRel dealloc) : m_ref(0), m_dealloc(dealloc)
    {
        m_nodes.reserve(max_sk_level);
    }
protected:
    void lock()
    {
        m_lock.lock();
    }
    void unlock()
    {
        m_lock.unlock();
    }
    int* dirty()
    {
        static thread_local int dirty_ = 0;
        return &dirty_;
    }
    void setDirty(int d)
    {
        int* p = dirty();
        *p = d;
    }
public:
    void add(Node* n)
    {
        lock();
        m_nodes.push_back(n);
        unlock();
        setDirty(1);
    }
    void addRef()
    {
        m_ref.fetch_add(1, std::memory_order_acq_rel);
    }
    void relRef()
    {
        if (m_ref.load(std::memory_order_relaxed) > 1)
        {
            m_ref.fetch_sub(1, std::memory_order_relaxed);
            return;
        }

        std::vector<Node* > rm_nodes;
        int ret = m_ref.fetch_sub(1, std::memory_order_release);
        if (ret == 1 && *dirty() == 1)
        {
            lock();
            rm_nodes.swap(m_nodes);
            unlock();
            setDirty(0);
        }
        auto iter = rm_nodes.begin();
        for (; iter != rm_nodes.end(); iter++)
        {
            if (m_dealloc)
            {
                m_dealloc(*iter);
            }
            else
            {
                free(*iter);
            }
        }
    }
};


template <typename T>
struct SkComp
{
    typedef std::function<int(const T& refer, const T& real)> Comp;
};

template <typename Key, typename Val, typename LOCK = CSkLock>
class CSklist
{
public:
    typedef Key SKey;
    typedef Val SVal;
    typedef std::pair<Key, Val> _Val;
    typedef SkNode<_Val, LOCK> _Node;
    _Node* head_;
    std::atomic<int> _level;
    std::atomic<int> size_;
    typename SkComp<Key>::Comp _comp;
    SkNodeRecycle<_Node, LOCK> recycle_;
public:
    CSklist(typename SkComp<Key>::Comp&& fun)
        : head_(NULL), _level(0), size_(0), _comp(fun)
    {
        init();
    }
private:
    void init()
    {
        head_ = buildNode(max_sk_level - 1, _Val());
    }
    bool less(Key&& k, _Node* n)
    {
        return !n || _comp(std::forward<Key>(k), n->val.first);
    }
    bool greater(Key&& k, _Node* n)
    {
        return n && _comp(n->val.first, std::forward<Key>(k));
    }
public:
    std::pair<int, SkNode<_Val, LOCK>*> insert(Key&& k, Val&& v);
    std::pair<int, SkNode<_Val, LOCK>*> update(Key&& k, Val&& v);
    SkNode<_Val, LOCK>* find(Key&& k);
    bool contains(Key&& k);
    int remove(Key&& k);
    int size()
    {
        return size_.load(std::memory_order_relaxed);
    }
protected:
    void free(SkNode<_Val, LOCK>* node);
    SkNode<_Val, LOCK>* buildNode(int lv, _Val&& val);
    int findOperatePos(int cur_level, int end_level, Key&& k, _Node* prefix[], _Node* suffix[])
    {
        _Node* node = head_;
        _Node* pre = node;
        _Node* node1 = NULL;
        int first_max_layer = -1;
        while (cur_level >= end_level)
        {
            node = layerNode(pre, cur_level);
            while (greater(std::forward<Key>(k), node))
            {
                pre = node;
                node = layerNode(node, cur_level);
            }
            if (-1 == first_max_layer && !less(std::forward<Key>(k), node))
            {
                first_max_layer = cur_level;
                node1 = node;
            }
            prefix[cur_level] = pre;
            suffix[cur_level] = node1 ? node1 : node;
            --cur_level;
        }
        return first_max_layer;
    }
};

template <typename Key, typename Val, typename LOCK>
SkNode<typename CSklist<Key, Val, LOCK>::_Val, LOCK>* CSklist<Key, Val, LOCK>::buildNode(int lv, _Val&& val)
{
    typedef SkNode<typename CSklist<Key, Val, LOCK>::_Val, LOCK> _Node;
    int nSize = sizeof(_Node) + lv * sizeof(_Node);
    _Node* pNode = (_Node*)malloc(nSize);
    memset(pNode, 0, nSize);
    pNode->level = lv;
    pNode->val = val;
    return pNode;
}

template <typename Key, typename Val, typename LOCK>
std::pair<int, SkNode<typename CSklist<Key, Val, LOCK>::_Val, LOCK>*> CSklist<Key, Val, LOCK>::insert(Key&& k, Val&& v)
{
    typedef typename CSklist<Key, Val, LOCK>::_Val Pair;
    typedef SkNode<typename CSklist<Key, Val, LOCK>::_Val, LOCK> _Node;
    _Node* prefix[max_sk_level] = {head_};
    _Node* suffix[max_sk_level] = {0};
    _Node* new_ = NULL;
    int new_level = randomSkLevel();
    int end_level = 0;
    do
    {
        int cur_max_lever = _level.load(std::memory_order_relaxed);
        int level = findOperatePos(cur_max_lever, end_level, std::forward<Key>(k), prefix, suffix);
        if (level >= 0)          
        {
            return std::pair<int, _Node*>(Failed, suffix[level]);
        }
        //创建新节点
        if (new_level > _level.load(std::memory_order_acquire))
        {
            _level.store(new_level, std::memory_order_release);
        }
        if (!new_)
        {
            new_ = buildNode(new_level, Pair(std::forward<Key>(k), std::forward<Val>(v)));
        }
        int i = end_level;
        for (; i <= new_level; ++i)
        {
            if (!prefix[i])
            {
                prefix[i] = head_;
            }
            lock(prefix[i]);
            if (suffix[i] != layerNode(prefix[i], i))                  //后置节点变化
            {
                unLock(prefix[i]);
                break;
            }
            if (!hasThisFlag(prefix[i], RemoveMark) && (suffix[i] == NULL || !hasThisFlag(suffix[i], RemoveMark)))
            {
                setLayerNode(prefix[i], i, new_);   //前驱
                setLayerNode(new_, i, suffix[i]);   //后置
                ++end_level;
                unLock(prefix[i]);
            }
            else
            {
                unLock(prefix[i]);
                break;
            }
        }
        if (i > new_level)
        {
            setFlag(new_, StayMark);
            break;
        }
    } while(1);
    size_.fetch_add(1, std::memory_order_relaxed);
    return std::pair<int, _Node*>(Success, new_);
}

template <typename Key, typename Val, typename LOCK>
std::pair<int, SkNode<typename CSklist<Key, Val, LOCK>::_Val, LOCK>*> CSklist<Key, Val, LOCK>::update(Key&& k, Val&& v)
{
    typedef SkNode<typename CSklist<Key, Val, LOCK>::_Val, LOCK> _Node;
    _Node* pre = head_;
    _Node* node = NULL;
    _Node* after = NULL;
    int cur_level = _level.load(std::memory_order_relaxed);
    while (cur_level >= 0)
    {
        node = layerNode(pre, cur_level);
        while (greater(std::forward<Key>(k), node))
        {
            pre = node;
            node = layerNode(node, cur_level);
        }
        if (!less(std::forward<Key>(k), node))
        {
            after = node;
            break;
        }
        --cur_level;
    }
    if (!after || hasThisFlag(after, RemoveMark))
    {
        return std::pair<int, _Node*>(Failed, after);
    }
    after->val.second = std::forward<Val>(v);
    return std::pair<int, _Node*>(Success, after);
}

template <typename Key, typename Val, typename LOCK>
int CSklist<Key, Val, LOCK>::remove(Key&& k)
{
    typedef SkNode<typename CSklist<Key, Val, LOCK>::_Val, LOCK> _Node;
    _Node* prefix[max_sk_level] = {head_};
    _Node* suffix[max_sk_level] = {0};
    _Node* del_ = NULL;
    int complete = Failed;
    int end_level = 0;
    int del_levels = 0;
    do
    {
        complete = Failed;
        int cur_max_lever = _level.load(std::memory_order_relaxed);
        int level = findOperatePos(cur_max_lever, end_level, std::forward<Key>(k), prefix, suffix);
        if (level < 0 && !del_)
        {
            break;
        }
        if (!del_ && !(hasThisFlag(suffix[level], StayMark) && !hasThisFlag(suffix[level], RemoveMark) && suffix[level]->level == level))
        {
            break;
        }
        if (!del_)
        {
            del_ = suffix[level];
            cur_max_lever = del_->level;
            del_levels = del_->level;
            lock(del_);
            if (hasThisFlag(del_, RemoveMark))
            {
                unLock(del_);
                return Success;
            }
            setFlag(del_, RemoveMark);
            unLock(del_);
        }
        if (del_)
        {
            for (; end_level <= del_levels; ++end_level)
            {
                lock(prefix[end_level]);
                _Node* p = layerNode(prefix[end_level], end_level);
                _Node* n = layerNode(del_, end_level);
                if (p != del_ || hasThisFlag(prefix[end_level], RemoveMark))
                {
                    unLock(prefix[end_level]);
                    break;
                }
                setLayerNode(prefix[end_level], end_level, n);
                unLock(prefix[end_level]);
            }
            if (end_level > del_levels)
            {
                break;
            }
        }
        continue;
    } while(1);
    free(del_);
    size_.fetch_sub(1, std::memory_order_relaxed);
    return complete;
}

template <typename Key, typename Val, typename LOCK>
SkNode<typename CSklist<Key, Val, LOCK>::_Val, LOCK>* CSklist<Key, Val, LOCK>::find(Key&& k)
{
    typedef SkNode<typename CSklist<Key, Val, LOCK>::_Val, LOCK> _Node;
    _Node* pre = head_;
    _Node* node_ = NULL;
    _Node* fnode_ = NULL;
    int lv = _level.load(std::memory_order_relaxed);
    do
    {
        while (lv >= 0)
        {
            node_ = layerNode(pre, lv);
            --lv;
            if (!less(std::forward<Key>(k), node_))
            {
                break;
            }
        }
        if (lv < 0)
        {
            break;
        }
        while (greater(std::forward<Key>(k), node_))
        {
            pre = node_;
            node_ = layerNode(node_, lv);
        }
        if (!less(std::forward<Key>(k), node_))
        {
            fnode_ = node_;
            break;
        }
    } while (1);
    return fnode_ && !hasThisFlag(fnode_, RemoveMark) ? fnode_ : NULL;
}

template <typename Key, typename Val, typename LOCK>
bool CSklist<Key, Val, LOCK>::contains(Key&& k)
{
    if (find(std::forward<Key>(k)))
        return true;
    return false;
}

template <typename Key, typename Val, typename LOCK>
void CSklist<Key, Val, LOCK>::free(SkNode<typename CSklist<Key, Val, LOCK>::_Val, LOCK>* node)
{
    if (node)
    {
        recycle_.add(node);
    }
}

template <typename SKL>
class SklVisitor
{
    SKL* sklist;
public:
    SklVisitor(SKL* list)
        : sklist(list)
    {
        sklist->recycle_.addRef();
    }
    ~SklVisitor()
    {
        sklist->recycle_.relRef();
    }

public:
    std::pair<int, typename SKL::_Node*> emplace(typename SKL::SKey&& k, typename SKL::SVal&& v)
    {
        return sklist->insert(std::forward<typename SKL::SKey>(k), std::forward<typename SKL::SVal>(v));
    }

    std::pair<int, typename SKL::_Node*> update(typename SKL::SKey&& k, typename SKL::SVal&& v)
    {
        return sklist->update(std::forward<typename SKL::SKey>(k), std::forward<typename SKL::SVal>(v));
    }

    int erase(typename SKL::SKey&& k)
    {
        return sklist->remove(std::forward<typename SKL::SKey>(k));
    }

    typename SKL::_Node* find(typename SKL::SKey&& k)
    {
        return sklist->find(std::forward<typename SKL::SKey>(k));
    }

    bool contains(typename SKL::SKey&& k)
    {
        return sklist->contains(std::forward<typename SKL::SKey>(k));
    }

    int size()
    {
        return sklist->size();
    }
};

template <typename SKL>
class SklIterator
{
    SKL* sklist;
public:
    SklIterator(SKL* list)
        : sklist(list)
    {
        sklist->recycle_.addRef();
    }
    ~SklIterator()
    {
        sklist->recycle_.relRef();
    }
public:
    struct Iterator
    {
        typename SKL::_Node* node_;
    public:
        Iterator(typename SKL::_Node* n) : node_(n)
        {}
    public:
        Iterator& operator++()
        {
            if (node_)
            {
                node_ = node_->wall[0];
            }
            return *this;
        }

        Iterator operator++(int)
        {
            if (node_)
            {
                node_ = node_->wall[0];
            }
            return Iterator(node_);
        }

        bool operator != (const Iterator& iter)
        {
            return node_ != iter.node_ ? true : false;
        }

        typename SKL::_Node* operator*()
        {
            return node_;
        }
    };

public:
    Iterator begin()
    {
        typename SKL::_Node* node_ = sklist->head_->wall[0];
        return Iterator(node_);
    }

    Iterator end()
    {
        return Iterator(NULL);
    }

    Iterator pos(typename SKL::SKey&& k)
    {
        typename SKL::_Node* node_ = sklist->find(std::forward<typename SKL::SKey>(k));
        return Iterator(node_);
    }
};

}
