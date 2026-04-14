#include <stdint.h>
#include <stdlib.h>
#include <functional>
#include <mutex>
#include <atomic>
#include <string.h>
#include <vector>
#include <thread>
#include <condition_variable>

namespace PxHash
{
constexpr int hash_factor = 16;                  //表示平均每个桶带16个node
constexpr int def_bucket = 32;                   //基础32个桶
constexpr int ShardBits = 0;

enum SkNodeFlag
{
    RehashMark = 0x01,
    RemoveMark = 0x10,       //标记将要删除
    StayMark = 0x0100,       //常驻节点
};

enum RehashSt
{
    NotRehash = 0,
    PreRehash,
    Rehashing,
    Rehashed,
};

template <typename Value, typename Lock>
struct _Node
{
    _Node* next = NULL;
    std::atomic<int> flag = {0};
    Value val;
    Lock lock_;
};

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

template <typename T>
struct _Comp
{
    typedef std::function<int(const T& refer, const T& real)> Comp;
};

//GC回收
template <typename Node, typename LOCK = std::mutex>
class SkNodeRecycle
{
public:
    typedef std::function<void(Node* )> _NodeRel;
    //! 是否达到了真实删除的条件
    std::atomic<int> m_ref;
    //!
    LOCK m_lock;
    //! 待回收的节点
    std::vector<Node* > m_nodes;
    //!
    _NodeRel m_dealloc = NULL;
public:
    SkNodeRecycle() : m_ref(0), m_dealloc(NULL)
    {
        m_nodes.reserve(32);
    }
    SkNodeRecycle(_NodeRel dealloc) : m_ref(0), m_dealloc(dealloc)
    {
        m_nodes.reserve(32);
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
    //放入待回收
    void add(Node* n)
    {
        lock();
        m_nodes.push_back(n);
        unlock();
        setDirty(1);
    }
    //
    void addRef()
    {
        m_ref.fetch_add(1, std::memory_order_acq_rel);
    }
    //内存回收
    void relRef()
    {
        if (m_ref.load(std::memory_order_relaxed) > 1)
        {
            m_ref.fetch_sub(1, std::memory_order_relaxed);
            return;
        }

        std::vector<Node* > rm_nodes;
        int ret = m_ref.fetch_sub(1, std::memory_order_release);
        if (ret == 1 && *dirty() == 1)   //旧值是1
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

enum RstFlag
{
    Failed = -1,
    Success = 0
};

enum
{
    None = -1,
    Found = 0,
    Insert = 1,
};

template <typename Key, typename Value, typename Compare = std::less<Key>, typename Lock = std::mutex>
class CList
{
public:
    typedef std::pair<Key, Value> ValueType;
    typedef _Node<ValueType, Lock> Node;
private:
    Node head_;
    //! 回收器
    SkNodeRecycle<Node, Lock>* recycle_;
    //! 比较函数
    Compare comp_;
public:
    CList()
        : recycle_(NULL), comp_(Compare())
    {
    }
    CList(Compare comp)
        : recycle_(NULL), comp_(comp)
    {
    }
public:
    void setRecycle(SkNodeRecycle<Node, Lock>* recy)
    {
        if (!recycle_)
        {
            recycle_ = recy;
        }
    }
protected:
    bool less(Key&& k, Node* n)
    {
        return !n || comp_(std::forward<Key>(k), n->val.first);
    }
    bool greater(Key&& k, Node* n)
    {
        return n && comp_(n->val.first, std::forward<Key>(k));
    }

    int find(Key&& k, Node** pred, Node** suf)
    {
        int ret = None;
        Node* pred_ = &head_;
        Node* node = pred->next;
        while (node)
        {
            if (greater(std::forward<Key>(k), node))
            {
                pred_ = node;
                node = node->next;
                continue;
            }
            //
            *suf = node;
            ret = Insert;
            if (!less(std::forward<Key>(k), node))
            {
                ret = Found;
            }
            break;
        }
        return ret;
    }

    bool accquireLockNode(bool insert_, Node** pred, Node** suf)
    {
        bool accquire_ = false;
        Node* pred_ = *pred;
        Node* next = NULL;
        lock(pred_);
        next = pred_->next;
        if (!hasThisFlag(pred_, RemoveMark) && next == *suf)
        {
            accquire_ = true;
        }
        if (insert_ && accquire_)
        {
            accquire_ &= (*suf == NULL || !hasThisFlag(*suf, RemoveMark));
        }
        return accquire_;     //!accquire_: 只要后置节点变了 就得重新申请锁定
    }

    void releaseLockNode(Node** pred)
    {
        unlock(*pred);
    }
public:
    Node* find(Key&& k)
    {
        Node* pred = NULL;
        Node* suf = NULL;
        int ret = find(std::forward<Key>(k), &pred, &suf);
        if (ret == Found)
        {
            return suf;
        }
        return NULL;
    }

    std::pair<Node*, bool> insert(Key&& k, Value&& v)
    {
        Node* pred = NULL;
        Node* suf = NULL;
        Node* new_ = NULL;
        do
        {
            int ret = find(std::forward<Key>(k), &pred, &suf);
            if (ret == Found)
            {
                return std::pair<Node*, bool>(suf, false);
            }
            //
            if (!accquireLockNode(1, &pred, &suf))
            {
                releaseLockNode(&pred);
                continue;
            }
            //放入
            new_ = (Node*)malloc(sizeof(Node));
            memset(new_, 0, sizeof(Node));
            new_->val = ValueType(std::forward<Key>(k), std::forward<Value>(v));
            pred->next = new_;
            new_->next = suf;
            setFlag(new_, StayMark);
            releaseLockNode(&pred);
            break;
        } while(1);
        return std::pair<Node*, bool>(new_, true);
    }

    std::pair<Node*, bool> insert(Node* new_)
    {
        Node* pred = NULL;
        Node* suf = NULL;
        do
        {
            int ret = find(std::forward<Key>(new_->val.first), &pred, &suf);
            if (ret == Found)
            {
                return std::pair<Node*, bool>(suf, false);
            }
            //
            if (!accquireLockNode(1, &pred, &suf))
            {
                releaseLockNode(&pred);
                continue;
            }
            //放入
            pred->next = new_;
            new_->next = suf;
            setFlag(new_, StayMark);
            releaseLockNode(&pred);
            break;
        } while(1);
        return std::pair<Node*, bool>(new_, true);
    }

    int remove(Key&& k)
    {
        Node* pred = NULL;
        Node* suf = NULL;
        Node* del_ = NULL;
        int complete = Failed;
        do
        {
            int ret = find(std::forward<Key>(k), &pred, &suf);
            if (ret != Found)
            {
                break;
            }
            if (!(hasThisFlag(suf, StayMark) && !hasThisFlag(suf, RemoveMark)))
            {
                break;
            }
            //
            if (!accquireLockNode(1, &pred, &suf))
            {
                releaseLockNode(&pred);
                continue;
            }
            if (!del_)
            {
                del_ = suf;
                //锁定欲移除的节点
                lock(del_);
                if (hasThisFlag(del_, RemoveMark))
                {
                    unLock(del_);
                    releaseLockNode(pred);
                    break;
                }
                setFlag(del_, RemoveMark);
            }
            //
            pred->next = del_->next;
            //解锁
            unLock(del_);
            releaseLockNode(&pred);
            complete = Success;
            break;
        } while(1);
        //放入回收
        recycle_->add(del_);
        return complete;
    }

    std::pair<Node*, bool> update(Key&& k, Value&& v)
    {
        Node* pred = NULL;
        Node* suf = NULL;
        do
        {
            int ret = find(std::forward<Key>(k), &pred, &suf);
            if (ret != Found)
            {
                return std::pair<Node*, int>(Failed, NULL);  //-1 表示更新失败
            }
            break;
        } while(1);
        if (!suf || hasThisFlag(suf, RemoveMark))       //因为节点要被移除 则此节点必不是有效节点 需要重新找有效节点
        {
            return std::pair<Node*, int>(Failed, suf);  //-1 表示更新失败
        }
        suf->val.second = std::forward<Value>(v);
        return std::pair<Node*, int>(Success, suf);
    }

    Node* first()
    {
        Node* pred = &head_;
        Node* next = NULL;
        Node* n = NULL;
        Node* new_ = (Node*)malloc(sizeof(Node));
        memset(new_, 0, sizeof(Node));
        do
        {
            lock(pred);
            next = pred->next;
            if (!next)
            {
                unlock(pred);
                free(new_);
                new_ = NULL;
                break;
            }
            lock(next);
            if (hasThisFlag(next, RemoveMark))
            {
                unlock(next);
                unlock(pred);
                continue;
            }
            //复制节点
            n = next;
            new_->val = n->val;
            unlock(n);
            unlock(pred);
            break;
        } while(1);
        return new_;
    }
};

template <typename Key, typename Value, typename Hash = std::hash<Key>, typename Compare = std::less<Key>, typename Lock = std::mutex>
class CHashTable
{
public:
    typedef std::pair<Key, Value> ValueType;
    typedef CList<Key, Value, Compare, Lock> Elements;
    typedef typename Elements::Node Node;
private:
    //! 桶数量
    int bucket_ = 0;
    //! 元素数量
    std::atomic<uint32_t> ele_cnt = {0};
    //! 回收器
    SkNodeRecycle<Node, Lock>* recycle_;
    //!
    Hash hash_fun;
    //!
    int rehash_index = 0;
    std::vector<Elements> datas;
public:
    CHashTable()
    {
    }
    CHashTable(int bucket, SkNodeRecycle<Node, Lock>* recycle)
        : bucket_(bucket), recycle_(recycle), hash_fun(Hash())
    {
        datas.reserve(bucket);
    }
public:
    bool canRehash()
    {
        int factor_ = ele_cnt.load(std::memory_order_relaxed) / (double)bucket_;
        if (factor_ >= hash_factor)
            return true;
        return false;
    }
    std::vector<Elements>& buckets()
    {
        return datas;
    }
    int bucketNum()
    {
        return bucket_;
    }
    void rehashIncrease()
    {
        ++rehash_index;
    }
    int rehashIndex()
    {
        return rehash_index;
    }
protected:
    uint32_t getIdx(size_t bucket_count, size_t hash)
    {
        return (hash >> ShardBits) & (bucket_count - 1);
    }
public:
    std::pair<Node*, bool> insert(Key&& k, Value&& val)
    {
        size_t hash_ = hash_fun(std::forward<Key>(k));
        uint32_t index = getIndex(bucket_, hash_);
        Elements& set = datas[index];
        Node* n = (Node*)malloc(sizeof(Node));
        std::pair<Node*, bool> pair = set.insert(std::forward<Key>(k), std::forward<Value>(val));
        ele_cnt.fetch_add(1, std::memory_order_relaxed);
        return pair;
    }
    std::pair<Node*, bool> insert(Node* n)
    {
        size_t hash_ = hash_fun(std::forward<Key>(n->val.first));
        uint32_t index = getIndex(bucket_, hash_);
        Elements& set = datas[index];
        std::pair<Node*, bool> pair = set.insert(n);
        ele_cnt.fetch_add(1, std::memory_order_relaxed);
        return pair;
    }
    int remove(Key&& k)
    {
        size_t hash_ = hash_fun(std::forward<Key>(k));
        uint32_t index = getIndex(bucket_, hash_);
        Elements& set = datas[index];
        set.setRecycle(recycle_);
        ele_cnt.fetch_sub(1, std::memory_order_relaxed);
        return set.remove(std::forward<Key>(k));
    }
    Node* find(Key&& k)
    {
        Node* n = NULL;
        size_t hash_ = hash_fun(std::forward<Key>(k));
        uint32_t index = getIndex(bucket_, hash_);
        Elements& set = datas[index];
        n = set.find(std::forward<Key>(k));
        return n;
    }
};

//
template <typename Key, typename Value, typename Hash = std::hash<Key>, typename Compare = std::less<Key>, typename Lock = std::mutex>
class CHashMap
{
    typedef CHashTable<Key, Value, Hash, Compare, Lock> HashMap;
    typedef CHashMap<Key, Value, Hash, Compare, Lock> HashTable;
    typedef typename HashMap::Node Node;
private:
    //!
    uint32_t bucket_ = 0;
    //! 1可以 2正在
    std::atomic<int> rehash_;
    //!
    std::atomic<int> m_stop;
    std::thread* thread_;
    //!
    HashMap* m1;
    HashMap* m2;
    //! 回收器
    SkNodeRecycle<Node, Lock> recycle_;
public:
    CHashMap()
        : bucket_(def_bucket), rehash_(NotRehash), m_stop(0), thread_(NULL), m1(new HashMap(def_bucket, &recycle_)), m2(NULL)
    {
    }
    CHashMap(Hash hash_, int bucket = def_bucket)
        : bucket_(bucket), rehash_(NotRehash), m_stop(0), thread_(NULL), m1(new HashMap(bucket, &recycle_)), m2(NULL)
    {
    }
    ~CHashMap()
    {
        m_stop.store(1, std::memory_order_relaxed);
        if (thread_)
        {
            thread_->join();
        }
    }
public:
    struct Protector
    {
        HashTable* ref_map;
    public:
        Protector(HashTable* map_) : ref_map(map_)
        {
            ref_map->recycle_.addRef();
        }
        ~Protector()
        {
            ref_map->recycle_.relRef();
        }
    };
    struct Pair
    {
        Node* first_;
        int second_;
        HashTable* ref_map;
    public:
        Pair(HashTable* map_, Node* n, int tag) : first_(n), second_(tag), ref_map(map_)
        {
            ref_map->recycle_.addRef();
        }
        ~Pair()
        {
            ref_map->recycle_.relRef();
        }
    public:
        const Key& first()
        {
            return first_->val.first;
        }

        Value& second()
        {
            return first_->val.second;
        }
    };
public:
    std::pair<Node*, bool> emplace(Key&& k, Value&& val)
    {
        //
        std::pair<Node*, bool> ret;
        Protector(this);
        if (rehash_.load() == Rehashing && m2)
        {
            ret = m2->insert(std::forward<Key>(k), std::forward<Value>(val));
        }
        else
        {
            ret = m1->emplace(std::forward<Key>(k), std::forward<Value>(val));
            if (m1->canRehash())
            {
                int old_val = rehash_.load(std::memory_order_acquire);
                if (old_val == 0)
                {
                    if (rehash_.compare_exchange_strong(old_val, PreRehash, std::memory_order_release))
                    {
                        //rehash的过程是异步的
                        if (!thread_)
                        {
                            thread_ = new std::thread([this]()
                            {
                                std::mutex m_lock;
                                std::condition_variable m_wait;
                                while (0 == m_stop.load(std::memory_order_relaxed))
                                {
                                    if (rehash_.load(std::memory_order_relaxed) == NotRehash)
                                    {
                                        std::unique_lock<std::mutex> lock(m_lock);
                                        m_wait.wait_for(lock, std::chrono::microseconds(10));
                                    }
                                    if (rehash_.load(std::memory_order_relaxed) == PreRehash)
                                    {
                                        bucket_ *= 2;
                                        m2 = new HashMap(bucket_, &recycle_);
                                        rehash_.store(Rehashing, std::memory_order_acq_rel);
                                    }
                                    if (rehash_.load(std::memory_order_relaxed) == Rehashing)
                                    {
                                        rehash();
                                    }
                                    if (rehash_.load(std::memory_order_relaxed) == Rehashed)
                                    {
                                        rehash_.store(NotRehash, std::memory_order_relaxed);
                                    }
                                }
                            });
                        }
                    }
                }
            }
        }
        return ret;
    }

    Pair find(Key&& k)
    {
        Node* n = NULL;
        Protector(this);
        if (rehash_.load() == Rehashing)
        {
            n = m1->find(std::forward<Key>(k));
            if (!n)
            {
                n = m2->find(std::forward<Key>(k));
            }
        }
        else
        {
            n = m1->find(std::forward<Key>(k));
        }
        return Pair(n, this, n ? 1 : 0);
    }

    int erase(Key&& k)
    {
        int ret = 0;
        Protector(this);
        if (rehash_.load() == Rehashing)
        {
            ret = m1->remove(std::forward<Key>(k));
            if (ret == Failed)
            {
                ret = m2->remove(std::forward<Key>(k));
            }
        }
        else
        {
            ret = m1->remove(std::forward<Key>(k));
        }
        return ret;
    }

    bool clear()
	{
		Protector(this);
		if (rehash_.load() == Rehashing)
		{
			return false;
		}
		auto fun = [](HashMap* map)
		{
            auto& buckets = map->buckets();
			auto iter = buckets.begin();
			for (; iter != buckets.end(); iter++)
			{
				while (1)
				{
					Node* n = (*iter).first();
					if (!n)
					{
						break;
					}
					Key k = n->val.first;
					map->remove(std::forward<Key>(k));
				}
			}
		};
		fun(m1);
		fun(m2);
		return true;
	}
public:
    //渐进式rehash
    int rehash()
    {
        Protector(this);
        auto& buckets = m1->buckets();
        //一次只挪一个桶
        auto iter = buckets.begin();
        if (iter != buckets.end())
        {
            while (1)
            {
                Node* n = (*iter).first();
                if (!n)
                {
                    m1->rehashIncrease();
                    break;
                }
                Key k = n->val.first;
                m2->insert(n);
                //如果已经删了 则需要完全删除
                if (Failed == m1->remove(std::forward<Key>(k)))
                {
                    m2->remove(std::forward<Key>(k));
                }
            }
        }
        if (m1->rehashIndex() < m1->bucketNum() - 1)
        {
            return Rehashing;
        }
        //遍历一遍 查看是否有漏的
        if (m1->rehashIndex() == m1->bucketNum() - 1)
        {
            buckets = m1->buckets();
            auto iter = buckets.begin();
            for (; iter != buckets.end(); iter++)
            {
                while (1)
                {
                    Node* n = (*iter).first();
                    if (!n)
                    {
                        break;
                    }
                    Key k = n->val.first;
                    m2->insert(n);
                    //如果已经删了 则需要完全删除
                    if (Failed == m1->remove(std::forward<Key>(k)))
                    {
                        m2->remove(std::forward<Key>(k));
                    }
                }
            }
        }
        HashMap* m_ = m1;
        m1 = m2;
        rehash_.store(NotRehash);
        delete m_;
        return Rehashed;
    }
};
}
