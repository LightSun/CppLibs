#include <stdint.h>
#include <stdlib.h>
#include <functional>
#include <atomic>
#include <string.h>
#include <thread>
#include <vector>
#include <mutex>

namespace PSK
{
constexpr int max_sk_level = 32;
constexpr float  _param = 0.25;

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
    RemoveMark = 0x10,       //标记将要删除
    StayMark = 0x0100,       //常驻节点
};

enum LockStatus
{
    UnLock = 0x00,
    Lock = 0x01,
};

template <typename T>
struct SkNode
{
    typedef SkNode<T> InType;
    int level = 0;
    std::atomic<int> flag = {0x00};
    std::atomic<int> lock_ = {UnLock};
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

//节点锁定
template <typename Node>
inline bool lock(Node* n)
{
    int ulock_ = UnLock;
    bool take_ = false;
    while (!take_)
    {
        for (int step = 0; step <= 100000 && !(take_ = n->lock_.compare_exchange_strong(ulock_, Lock, std::memory_order_release)); ++step)
            ulock_ = UnLock;
        if (!take_)
            std::this_thread::yield();
    }
    return true;
}

template <typename Node>
inline void unLock(Node* n)
{
    n->lock_.store(UnLock, std::memory_order_release);
}

template <typename Node>
inline bool isLock(Node* n)
{
    return n->lock_.load(std::memory_order_acquire) & Lock;
}
//节点锁定

//设置和判断节点状态
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
//设置和判断节点状态

//回收
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

//GC回收
template <typename Node>
class SkNodeRecycle
{
public:
    typedef std::function<void(Node* )> _NodeRel;
    std::atomic<int> m_ref;
    std::atomic<int> m_lock;
    std::vector<Node* > m_nodes;
    _NodeRel m_dealloc;
public:
    SkNodeRecycle() : m_ref(0), m_lock(UnLock), m_dealloc(NULL)
    {
        m_nodes.reserve(max_sk_level);
    }
    SkNodeRecycle(_NodeRel dealloc) : m_ref(0), m_lock(UnLock), m_dealloc(dealloc)
    {
        m_nodes.reserve(max_sk_level);
    }
protected:
    void lock()
    {
        int ulock_ = UnLock;
        bool take_ = false;
        while (!take_)
        {
            for (int step = 0; step <= 100000 && !(take_ = m_lock.compare_exchange_strong(ulock_, Lock, std::memory_order_release)); ++step)
                ulock_ = UnLock;
            if (!take_)
                std::this_thread::yield();
        }
    }
    void unlock()
    {
        m_lock.store(UnLock, std::memory_order_release);
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
        //释放
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

template <typename Key, typename Val, typename LOCK = std::mutex>
class CSklist
{
public:
    typedef Key SKey;
    typedef Val SVal;
    typedef std::pair<Key, Val> _Val;
    typedef SkNode<_Val> _Node;
    _Node* head_;
    std::atomic<int> _level;
    typename SkComp<Key>::Comp _comp;
    SkNodeRecycle<_Node> recycle_;
public:
    CSklist(typename SkComp<Key>::Comp&& fun)
        : head_(NULL), _level(0), _comp(fun)
    {
        init();
    }
private:
    void init()
    {
        head_ = buildNode(max_sk_level, _Val());
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
    std::pair<int, SkNode<_Val>*> insert(Key&& k, Val&& v);
    std::pair<int, SkNode<_Val>*> update(Key&& k, Val&& v);
    SkNode<_Val>* find(Key&& k);
    bool contains(Key&& k);
    int remove(Key&& k);
    void free(SkNode<_Val>* node);
protected:
    SkNode<_Val>* buildNode(int lv, _Val&& val);
    int findOperatePos(int cur_level, Key&& k, _Node* prefix[], _Node* suffix[])
    {
        _Node* node = head_;
        _Node* pre = node;
        _Node* node1 = NULL;
        int first_max_layer = -1;
        while (cur_level >= 0)
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

    bool accquireBeChangedNodes(int cur_level, int insert, _Node* prefix[], _Node* suffix[])
    {
        bool accquire = false;
        _Node* pre = NULL;
        _Node* suf = NULL;
        _Node* pre1 = NULL;
        for (int lv = 0; lv <= cur_level; ++lv)
        {
            if (!prefix[lv])
            {
                prefix[lv] = head_;
            }
            pre = prefix[lv];
            suf = suffix[lv];
            if (pre != pre1)
            {
                lock(pre);
                pre1 = pre;
            }
            accquire = false;
            _Node* n = layerNode(pre, lv);
            if (!hasThisFlag(pre, RemoveMark) && suf == layerNode(pre, lv))
            {
                accquire = true;
            }
            if (insert && accquire)
            {
                accquire &= (suf == NULL || !hasThisFlag(suf, RemoveMark));
            }
            if (!accquire)
            {
                break;
            }
        }
        return accquire;
    }

    void releaseBeChangedNodes(int cur_level, _Node* prefix[])
    {
        _Node* pre = NULL;
        _Node* pre1 = NULL;
        for (int lv = 0; lv <= cur_level; ++lv)
        {
            pre = prefix[lv];
            if (!pre)
            {
                continue;
            }
            if (pre != pre1)
            {
                unLock(pre);
                pre1 = pre;
            }
        }
    }
};

template <typename Key, typename Val>
SkNode<typename CSklist<Key, Val>::_Val>* CSklist<Key, Val>::buildNode(int lv, _Val&& val)
{
    typedef SkNode<typename CSklist<Key, Val>::_Val> _Node;
    int nSize = sizeof(_Node) + lv * sizeof(_Node);
    _Node* pNode = (_Node*)malloc(nSize);
    memset(pNode, 0, nSize);
    pNode->level = lv;
    pNode->val = val;
    return pNode;
}

template <typename Key, typename Val>
std::pair<int, SkNode<typename CSklist<Key, Val>::_Val>*> CSklist<Key, Val>::insert(Key&& k, Val&& v)
{
    typedef typename CSklist<Key, Val>::_Val Pair;
    typedef SkNode<typename CSklist<Key, Val>::_Val> _Node;
    _Node* prefix[max_sk_level] = {head_};
    _Node* suffix[max_sk_level] = {0};
    _Node* new_ = NULL;
    int new_level = 0;
    do
    {
        int cur_max_lever = _level.load(std::memory_order_relaxed);
        int level = findOperatePos(cur_max_lever, std::forward<Key>(k), prefix, suffix);
        if (level >= 0)
        {
            return std::pair<int, _Node*>(Failed, NULL);
        }
        new_level = randomSkLevel();
        if (!accquireBeChangedNodes(new_level, 1, prefix, suffix))
        {
            releaseBeChangedNodes(new_level, prefix);
            continue;
        }
        if (new_level > _level.load(std::memory_order_acquire))
        {
            _level.store(new_level, std::memory_order_release);
        }
        new_ = buildNode(new_level, Pair(std::forward<Key>(k), std::forward<Val>(v)));
        for (int i = 0; i <= new_level; ++i)
        {
            setLayerNode(prefix[i], i, new_);   //前驱
            setLayerNode(new_, i, suffix[i]);   //后置
        }
        setFlag(new_, StayMark);
        releaseBeChangedNodes(new_level, prefix);
        break;
    } while(1);
    return std::pair<int, _Node*>(Success, new_);
}

template <typename Key, typename Val>
std::pair<int, SkNode<typename CSklist<Key, Val>::_Val>*> CSklist<Key, Val>::update(Key&& k, Val&& v)
{
    typedef SkNode<typename CSklist<Key, Val>::_Val> _Node;
    _Node* prefix[max_sk_level] = {head_};
    _Node* suffix[max_sk_level] = {0};
    _Node* after = NULL;
    do
    {
        int cur_max_lever = _level.load(std::memory_order_relaxed);
        int level = findOperatePos(cur_max_lever, std::forward<Key>(k), prefix, suffix);
        if (level >= 0)
        {
            after = suffix[level];
            if (hasThisFlag(after, RemoveMark))
            {
                return std::pair<int, _Node*>(Failed, after);
            }
            bool bc = false;
            do
            {
                if (hasThisFlag(after, StayMark))
                    break;
                if (hasThisFlag(after, RemoveMark))
                {
                    bc = true;
                    break;
                }
            } while(1);
            if (bc)
                continue;
        }
        break;
    } while(1);
    if (after)
    {
        after->val.second = std::forward<Val>(v);
    }
    return std::pair<int, _Node*>(after ? Success : Failed, after);
}

template <typename Key, typename Val>
int CSklist<Key, Val>::remove(Key&& k)
{
    typedef SkNode<typename CSklist<Key, Val>::_Val> _Node;
    _Node* prefix[max_sk_level] = {head_};
    _Node* suffix[max_sk_level] = {0};
    _Node* del_ = NULL;
    int complete = Failed;
    do
    {
        complete = false;
        int cur_max_lever = _level.load(std::memory_order_relaxed);
        int level = findOperatePos(cur_max_lever, std::forward<Key>(k), prefix, suffix);
        if (level < 0)
        {
            break;
        }
        if (!(hasThisFlag(suffix[level], StayMark) && !hasThisFlag(suffix[level], RemoveMark) && suffix[level]->level == level))
        {
            break;
        }
        if (!accquireBeChangedNodes(level, 0, prefix, suffix))
        {
            releaseBeChangedNodes(level, prefix);
            continue;
        }
        if (!del_)
        {
            del_ = suffix[level];
            level = del_->level;
            lock(del_);
            if (hasThisFlag(del_, RemoveMark))
            {
                unLock(del_);
                releaseBeChangedNodes(level, prefix);
                break;
            }
            setFlag(del_, RemoveMark);
        }
        for (int i = level; i >= 0; --i)
        {
            _Node* n = layerNode(del_, i);
            setLayerNode(prefix[i], i, n);
        }
        unLock(del_);
        releaseBeChangedNodes(level, prefix);
        complete = Success;
        break;
    } while(1);
    free(del_);
    return complete;
}

template <typename Key, typename Val>
SkNode<typename CSklist<Key, Val>::_Val>* CSklist<Key, Val>::find(Key&& k)
{
    typedef SkNode<typename CSklist<Key, Val>::_Val> _Node;
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

template <typename Key, typename Val>
bool CSklist<Key, Val>::contains(Key&& k)
{
    if (find(std::forward<Key>(k)))
        return true;
    return false;
}

template <typename Key, typename Val>
void CSklist<Key, Val>::free(SkNode<typename CSklist<Key, Val>::_Val>* node)
{
    if (node)
    {
        recycle_.add(node);
    }
}

//一般访问
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
    std::pair<int, typename SKL::_Node*> insert(typename SKL::SKey&& k, typename SKL::SVal&& v)
    {
        std::pair<int, typename SKL::_Node*> ret = sklist->insert(std::forward<typename SKL::SKey>(k), std::forward<typename SKL::SVal>(v));
        return ret;
    }

    std::pair<int, typename SKL::_Node*> update(typename SKL::SKey&& k, typename SKL::SVal&& v)
    {
        std::pair<int, typename SKL::_Node*> ret = sklist->update(std::forward<typename SKL::SKey>(k), std::forward<typename SKL::SVal>(v));
        return ret;
    }

    int remove(typename SKL::SKey&& k)
    {
        int ret = sklist->remove(std::forward<typename SKL::SKey>(k));
        return ret;
    }

    typename SKL::_Node* find(typename SKL::SKey&& k)
    {
        return sklist->find(std::forward<typename SKL::SKey>(k));
    }
};

//迭代器
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

