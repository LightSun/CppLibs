#pragma once

#include <stdlib.h>
#include <atomic>
#include <stdio.h>
#include <thread>

namespace fqa
{
constexpr int DefQALength = 512 * 1024;

//Size必须是2^n
template <typename T, int Size= DefQALength>
class CNonLockQueueArray
{
public:
    typedef T Obj_Type;
public:
    int m_buf_mask;
    //! 存储空间
    char pad[56] = {0};
    T m_array[Size];
    //! 头尾
    char pad1[56] = {0};
    std::atomic<uint64_t> m_head;
    char pad2[56] = {0};
    std::atomic<uint64_t> m_tail;
    //! 当前最大索引数(多个线程写时必须保证按时间序递增)
    char pad3[56] = {0};
    std::atomic<uint64_t> m_max_index;
    char pad4[56] = {0};
    std::atomic<uint64_t> m_min_index;
public:
    CNonLockQueueArray() : m_buf_mask(Size - 1), m_head(0), m_tail(0), m_max_index(0), m_min_index(0)
    {
        memset(m_array, 0, sizeof(m_array));
    }
    CNonLockQueueArray(const CNonLockQueueArray&) = delete;
    CNonLockQueueArray(CNonLockQueueArray&&) = delete;
    CNonLockQueueArray& operator&=(const CNonLockQueueArray&) = delete;
    CNonLockQueueArray& operator&=(CNonLockQueueArray&&) = delete;

public:
    //sp
    bool spush_back(T&& val);
    //mp
    bool push_back(T&& val);
    //sc
    T pop_front();
    //mc
    T mpop_front();
};

template <typename T, int Size>
bool CNonLockQueueArray<T, Size>::spush_back(T&& val)
{
    do
    {
        uint64_t max_index = m_tail.load(std::memory_order_relaxed);
        uint64_t min_index = m_head.load(std::memory_order_acquire);
        if (max_index - min_index >= Size)
        {
            continue;
        }
        //写值
        m_array[max_index & m_buf_mask] = std::move(val);
        std::atomic_signal_fence(std::memory_order_release);
        m_tail.store(max_index + 1, std::memory_order_release);
        break;
    } while (1);
    return true;
}

template <typename T, int Size>
bool CNonLockQueueArray<T, Size>::push_back(T&& val)
{
    do
    {
        uint64_t max_index = m_tail.load(std::memory_order_acquire);
        uint64_t min_index = m_head.load(std::memory_order_acquire);
        uint64_t next_index = max_index + 1;
        if (max_index - min_index >= Size)
        {
            continue;
        }
        if (!m_max_index.compare_exchange_strong(max_index, next_index, std::memory_order_acquire))
        {
            continue;
        }
        //写值
        m_array[max_index & m_buf_mask] = std::move(val);
        std::atomic_signal_fence(std::memory_order_release);
        m_tail.store(max_index + 1, std::memory_order_release);
        break;
    } while (1);
    return true;
}

template <typename T, int Size>
T CNonLockQueueArray<T, Size>::pop_front()
{
    uint64_t index = 0;
    uint64_t order_index = 0;
    T val;
    index = m_head.load(std::memory_order_relaxed);
    order_index = m_tail.load(std::memory_order_acquire);
    if (index >= order_index)
    {
        return val;
    }
    //取值
//        val = std::move(m_array[index & m_buf_mask]);
    val = m_array[index & m_buf_mask];
    m_head.store(index + 1, std::memory_order_release);
    return val;
}

template <typename T, int Size>
T CNonLockQueueArray<T, Size>::mpop_front()
{
    uint64_t index = 0;
    uint64_t order_index = 0;
    T val;
    do
    {
        order_index = m_tail.load(std::memory_order_acquire);
        index = m_head.load(std::memory_order_acquire);
        if (index >= order_index)
        {
            return T();
        }
        if (!m_min_index.compare_exchange_strong(index, index + 1, std::memory_order_acquire))
        {
            continue;
        }
        //取值
//        val = std::move(m_array[index & m_buf_mask]);
        val = m_array[index & m_buf_mask];
        m_head.store(index + 1, std::memory_order_release);
        break;
    } while (1);
    return val;
}
}

