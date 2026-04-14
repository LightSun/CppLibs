/*****************************************************************************
* @author  : windsPx                                                         *
* @date    :                                                                 *
* @file    :                                                                 *
* @brief   :                                                                 *
*----------------------------------------------------------------------------*
* Date        | Version   | Author         | Description                     *
*----------------------------------------------------------------------------*
*             |           |  windsPx              |                          *
*****************************************************************************/
#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif
#include <atomic>
#include <array>
#include <memory>
#include <assert.h>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

//用户空间的读写锁(基于非mutex 支持递归调用)  Thread_Cnt线程数
//三态 读 写 中立 读和写之间转换必须经过中立态
template <int Thread_Cnt = 8>
class CSharedCriticalSection
{
    //! 写锁 独占
//	char tmp1[64] = { 0 };               //尽力避免false-sharing
    std::atomic<int> m_x_lock = { 0 };
    char tmp1[64] = { 0 };               //尽力避免false-sharing
    std::atomic<int> m_r_lock = { 0 };
    //! 递归结构
    struct rw_flag
    {
        char tmp3[56] = { 0 };               //尽力避免false-sharing
        std::atomic<unsigned int> m_thread_id = { 0 };     //当前的被分配的线程id
        std::atomic<int> m_val = { 0 };
    };
    using rw_threads_tags = std::array<rw_flag, Thread_Cnt>;
    char tmp4[64] = { 0 };                  //尽力避免false-sharing
    std::shared_ptr<rw_threads_tags> m_rw_tags;
    char tmp5[64] = { 0 };                  //尽力避免false-sharing
    rw_threads_tags& tags_ref;
    enum INNERRW
    {
        SC_UNDEF = 0,
        SC_READ = 1,
        SC_WRITE = 2,
    };
public:
    CSharedCriticalSection() :
        m_rw_tags(std::make_shared<rw_threads_tags>()), tags_ref(*m_rw_tags)
    {
    }
private:
    static unsigned int GetThreadId()
    {
#ifdef _WIN32
        return ::GetCurrentThreadId();
#else
        return pthread_self();
#endif
    }
    //某个线程注册或获取自身对应的被分配的id
    unsigned int reg_or_get_thread_id(unsigned int thread_id)
    {
        unsigned int older_inner_thread_id = 0;
        unsigned int new_inner_thread_id = 0;
        unsigned int thread_index = 0;
        //遍历
        unsigned int i = 0;
        for (; i < Thread_Cnt; ++i)
        {
            if (tags_ref[i].m_thread_id.load(std::memory_order_relaxed) == thread_id)           //说明有啊
            {
                thread_index = i;
                return thread_index;
            }
        }
        //如果没有注册过 则去占一个槽位
        for (i = 0; i < Thread_Cnt; ++i)
        {
            older_inner_thread_id = 0;
            new_inner_thread_id = thread_id;
            if (tags_ref[i].m_thread_id.compare_exchange_strong(older_inner_thread_id, new_inner_thread_id, std::memory_order_release))
            {
                thread_index = i;
                tags_ref[i].m_val.store(0, std::memory_order_relaxed);
                break;
            }
        }
        assert(i < Thread_Cnt);                      //超过线程数限制了
        return thread_index;
    }

    //卸载挂接的线程 -- 基本不用
    void unregister_thread_id(unsigned int thread_id)
    {
        for (unsigned int i = 0; i < Thread_Cnt; ++i)
        {
            if (tags_ref[i].m_thread_id == thread_id)           //说明有啊
            {
                tags_ref[i].m_thread_id.store(0, std::memory_order_relaxed);
                break;
            }
        }
    }

public:
    //读锁
    void lock_shared()
    {
        unsigned int thread_id = GetThreadId();
        int yield_wait = 0;
        int rw = 0;
        //注册线程
        unsigned int thread_index = reg_or_get_thread_id(thread_id);
        int recurse_cnt = tags_ref[thread_index].m_val.load(std::memory_order_relaxed);
        if (recurse_cnt >= 1)
        {
            tags_ref[thread_index].m_val.fetch_add(1, std::memory_order_relaxed);
            m_r_lock.fetch_add(1, std::memory_order_release);
        }
        else
        {
            do
            {
                m_r_lock.fetch_add(1, std::memory_order_release);
                rw = SC_UNDEF;
                if (m_x_lock.compare_exchange_strong(rw, SC_READ, std::memory_order_acquire))
                {
                    tags_ref[thread_index].m_val.fetch_add(1, std::memory_order_acquire);
                    break;
                }
                rw = SC_READ;
                if (m_x_lock.compare_exchange_strong(rw, SC_READ, std::memory_order_release))
                {
                    tags_ref[thread_index].m_val.fetch_add(1, std::memory_order_acquire);
                    break;
                }
                m_r_lock.fetch_sub(1, std::memory_order_release);
                std::this_thread::yield();
            } while (1);
        }
    }

    void unlock_shared()
    {
        unsigned int thread_id = GetThreadId();
        unsigned int thread_index = reg_or_get_thread_id(thread_id);
        //减计数
        tags_ref[thread_index].m_val.fetch_sub(1, std::memory_order_release);
        m_r_lock.fetch_sub(1, std::memory_order_release);
        int r_lock = m_r_lock.load(std::memory_order_acquire);
        if (r_lock < 1)
        {
            int rw = SC_READ;
            m_x_lock.compare_exchange_strong(rw, SC_UNDEF, std::memory_order_acquire);
        }
    }

    //写锁
    void lock()
    {
        unsigned int thread_id = GetThreadId();
        unsigned int thread_index = reg_or_get_thread_id(thread_id);
        int yield_wait = 0;
        int x_lock = 0;
        int recurse_cnt = tags_ref[thread_index].m_val.load(std::memory_order_relaxed);
        if (recurse_cnt >= 1)
        {
            tags_ref[thread_index].m_val.fetch_add(1, std::memory_order_relaxed);
        }
        else
        {
            //必须等到悬置态 才能去锁定
            int rw = 0;
            do
            {
                if (m_r_lock.load(std::memory_order_acquire) > 0)
                    continue;
                rw = SC_UNDEF;
                if (m_x_lock.compare_exchange_strong(rw, SC_WRITE, std::memory_order_acquire))
                {
                    tags_ref[thread_index].m_val.fetch_add(1, std::memory_order_relaxed);
                    break;
                }
                std::this_thread::yield();
            } while (1);
            while (m_r_lock.load(std::memory_order_acquire) > 0);
        }
    }

    void unlock()
    {
        unsigned int thread_id = GetThreadId();
        unsigned int thread_index = reg_or_get_thread_id(thread_id);
        tags_ref[thread_index].m_val.fetch_sub(1, std::memory_order_release);
        if (tags_ref[thread_index].m_val.load(std::memory_order_acquire) < 1)
        {
            int rw = SC_WRITE;
            m_x_lock.compare_exchange_strong(rw, SC_UNDEF, std::memory_order_seq_cst);
        }
    }
};

using CDefaultRWSharedLock = CSharedCriticalSection<>;
