#pragma once
#include <memory>
#include <atomic>
#include <memory.h>
#include <vector>
#include <bitset>
#include <assert.h>

//简单的固定对象大小的对象池
namespace OMP
{
constexpr int Mode_S = 0;
constexpr int Mode_M = 1;

template <int Num = 256, int Mask = Num - 1>
class Ids
{
protected:
    typedef unsigned long long UINT64;
    //! 存储可用下标
    unsigned short m_ids[Num];
    //! 用来控制obj-arr的可用下标的
    std::atomic<UINT64> m_id_h;
    std::atomic<UINT64> m_id_t;
    std::atomic<UINT64> m_id_t_max;
public:
    Ids()
        : m_id_h(0), m_id_t(0)
        , m_id_t_max(0)
    {
        for (int i = 0; i < Num; ++i)
        {
            m_ids[i] = i;
        }
    }

    Ids(Ids&& _ids)
    {
        memcpy(m_ids, _ids.m_ids, sizeof(m_ids));
        m_id_h = _ids.m_id_h.load();
        m_id_t = _ids.m_id_t.load();
        m_id_t_max = _ids.m_id_t_max.load();
    }

    Ids& operator=(Ids&& _ids)
    {
        memcpy(m_ids, _ids.m_ids, sizeof(m_ids));
        m_id_h = _ids.m_id_h.load();
        m_id_t = _ids.m_id_t.load();
        m_id_t_max = _ids.m_id_t_max.load();
        return *this;
    }
public:
    //归还可用id
    void relId(int id)
    {
        UINT64 min_index = 0;
        do
        {
            min_index = m_id_t.load(std::memory_order_acquire);
            if (m_id_t_max.compare_exchange_strong(min_index, min_index + 1, std::memory_order_acquire))
            {
                break;
            }
        } while (1);
        m_ids[min_index & Mask] = id;
        m_id_t.fetch_add(1, std::memory_order_release);
    }

    //单:获取可用id
    int getId()
    {
        UINT64 index = 0;
        UINT64 order_index = 0;
        int id = -1;
        index = m_id_h.load(std::memory_order_relaxed);
        order_index = m_id_t.load(std::memory_order_relaxed);
        if (index - order_index >= (UINT64)Num)
        {
            return id;
        }
        //取值
        id = m_ids[index & Mask];
        m_id_h.store(index + 1, std::memory_order_release);
        return id;
    }
};

template <typename T, int Num = 256, int Mask = Num - 1>
class CSimpleObjPool
{
public:
    typedef T OBJ;
    typedef unsigned long long UINT64;
    struct OBJBlock
    {
        char _obj[sizeof(OBJ)] = {0};
        int id = 0;
        int pid = 0;
        int tid = 0;
    };
private:
    //! 此pool的id
    int m_id;
    int m_tid;
    //! 对象缓存池
    char* m_buf;
    //! 存储可用下标
    Ids<Num, Mask> m_obj_ids;
public:
    //! 默认256个
    CSimpleObjPool()
        : m_id(0), m_buf(NULL)
    {
        m_buf = (char*)malloc(sizeof(OBJBlock) * Num);
        memset(m_buf, 0, sizeof(OBJBlock) * Num);
    }
    ~CSimpleObjPool()
    {
        if (m_buf)
        {
            free(m_buf);
            m_buf = NULL;
        }
    }

    CSimpleObjPool(CSimpleObjPool&& pool)
    {
        m_id = pool.m_id;
        m_tid = pool.m_tid;
        m_buf = pool.m_buf;
        pool.m_buf = NULL;
        m_obj_ids = std::move(pool.m_obj_ids);
    }

    CSimpleObjPool& operator=(CSimpleObjPool&& pool)
    {
        m_id = pool.m_id;
        m_tid = pool.m_tid;
        m_buf = pool.m_buf;
        pool.m_buf = NULL;
        m_obj_ids = std::move(pool.m_obj_ids);
        return *this;
    }

    void setId(int id)
    {
        m_id = id;
    }

    void settid(int id)
    {
        m_tid = id;
    }

public:
    //! 获取
    template<typename ...Args>
    OBJ* accqure(Args&& ...params)
    {
        int id = m_obj_ids.getId();
        if (-1 == id)
        {
            return NULL;
        }
        //
        OBJBlock* ptr = (OBJBlock*)m_buf;
        ptr = ptr + id;
        OBJ* _obj = new(ptr->_obj)OBJ(std::forward<Args>(params)...);
        ptr->id = id;
        ptr->pid = m_id;
        ptr->tid = m_tid;
        return _obj;
    }
    //! 归还
    void release(OBJ* p)
    {
        OBJBlock* ptr = (OBJBlock*)p;
        int id = ptr->id;
        ptr->id = 0;
        ptr->pid = 0;
        ptr->tid = 0;
        p->~OBJ();
        //归还obj-index
        m_obj_ids.relId(id);
    }
};

//扩张池
template <typename T, int Num = 1024, int Mask = Num - 1>
class CSimpleObjPoolSet
{
public:
    typedef T OBJ;
    //!
    constexpr static int idNum = 4096 * 4096 / Num;
    int m_tid;
    //!
    int m_cur_pool_id;
    std::atomic<int> m_next_id;
    //!
    std::vector<CSimpleObjPool<T, Num, Mask> > m_pools;
public:
    CSimpleObjPoolSet(int tid) : m_tid(tid), m_cur_pool_id(0), m_next_id(0)
    {
        m_pools.reserve(idNum);
        CSimpleObjPool<T, Num, Mask> pool;
        pool.setId(0);
        pool.settid(m_tid);
        m_pools[0] = std::forward<CSimpleObjPool<T, Num, Mask>>(pool);
        m_next_id.fetch_add(1);
    }

public:
    //! 获取
    template<typename ...Args>
    OBJ* accqure(Args&& ...params)
    {
        std::atomic_thread_fence(std::memory_order_acquire);
        int pid = m_cur_pool_id;
        OBJ* _obj = NULL;
        while ((_obj = m_pools[pid].accqure(std::forward<Args>(params)...)) == NULL)          //没有就扩展空间
        {
            pid = m_next_id.fetch_add(1);
            CSimpleObjPool<T, Num, Mask> pool;
            pool.setId(pid);
            pool.settid(m_tid);
            m_pools[pid] = std::forward<CSimpleObjPool<T, Num, Mask>>(pool);
//            m_cur_pool_id.store(pid);
            m_cur_pool_id = pid;
        }
        assert(pid < idNum);
        return _obj;
    }
    //! 归还
    void release(OBJ* p)
    {
        typename CSimpleObjPool<T, Num, Mask> ::OBJBlock* ptr = (typename CSimpleObjPool<T, Num, Mask>::OBJBlock*)p;
        int pid = ptr->pid;
        m_pools[pid].release(p);
        //归还的 就是可用
        m_cur_pool_id = pid;
        std::atomic_thread_fence(std::memory_order_release);
    }
};

//线程分离的分配
template <typename T, int Num = 16, int Mask = Num - 1>
class CSimpleThreadMemPool
{
    typedef T OBJ;
    thread_local static CSimpleObjPoolSet<T, Num, Mask>* pool;

    std::atomic<int> tid;
    CSimpleObjPoolSet<T, Num, Mask>* pools[1024];             //第0个 不用
public:
    CSimpleThreadMemPool() : tid(1)
    {
        memset(pools, 0 , sizeof(pools));
    }
protected:
    CSimpleObjPoolSet<T, Num, Mask>* check()
    {
        static thread_local int _tid = 0;
        if (_tid == 0)
        {
            _tid = tid.fetch_add(1);
            pools[_tid] = new CSimpleObjPoolSet<T, Num, Mask>(_tid);
        }
        return pools[_tid];
    }
public:
    template<typename ...Args>
    OBJ* accqure(Args&& ...params)
    {
        return check()->accqure(std::forward<Args>(params)...);
    }

    //! 归还
    void release(OBJ* p)
    {
        if (!p)
            return;
        typename CSimpleObjPool<T, Num, Mask> ::OBJBlock* ptr = (typename CSimpleObjPool<T, Num, Mask>::OBJBlock*)p;
        int tid = ptr->tid;
        pools[tid]->release(p);
    }
};
}

