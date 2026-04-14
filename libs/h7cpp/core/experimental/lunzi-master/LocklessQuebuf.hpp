#pragma once

#include <stdlib.h>
#include <atomic>
#include <stdio.h>
#include <thread>
#include <string.h>

namespace fqa
{
constexpr int DefQALength = 512 * 1024;

//Size必须是2^n
template <typename T, int Size= DefQALength>
class CLockLessQueBuf
{
public:
    typedef T Obj_Type;
public:
    int m_buf_mask;
    //! 头尾
    char pad1[64] = {0};
    std::atomic<uint64_t> m_head;
    char pad2[64] = {0};
    std::atomic<uint64_t> m_tail;
    //! 当前最大索引数(多个线程写时必须保证按时间序递增)
    char pad3[64] = {0};
    std::atomic<uint64_t> m_max_index;
    char pad4[64] = {0};
    std::atomic<uint64_t> m_min_index;
    //! 存储空间
    char pad[64] = {0};
    T m_array[Size];
public:
    CLockLessQueBuf() : m_buf_mask(Size - 1), m_head(0), m_tail(0), m_max_index(0), m_min_index(0)
    {
        memset(m_array, 0, sizeof(m_array));
    }
    CLockLessQueBuf(const CLockLessQueBuf&) = delete;
    CLockLessQueBuf(CLockLessQueBuf&&) = delete;
    CLockLessQueBuf& operator&=(const CLockLessQueBuf&) = delete;
    CLockLessQueBuf& operator&=(CLockLessQueBuf&&) = delete;

public:
    //sp
    T* getWriteBuf();
    //mp
    T* preemtWriteBuf();
    void commitWrite();
    //sc
    T* getReadBuf();
    //mc
    T* preemtReadBuf();
    void commitRead();
};

template <typename T, int Size>
T* CLockLessQueBuf<T, Size>::getWriteBuf()
{
    T* ptr = NULL;
    uint64_t max_index = m_tail.load(std::memory_order_relaxed);
    uint64_t min_index = m_head.load(std::memory_order_acquire);
    if (max_index - min_index >= Size)
    {
        return ptr;
    }
    ptr = &m_array[max_index & m_buf_mask];
    return ptr;
}

template <typename T, int Size>
T* CLockLessQueBuf<T, Size>::preemtWriteBuf()
{
    T* ptr = NULL;
    uint64_t max_index = m_tail.load(std::memory_order_acquire);
    uint64_t min_index = m_head.load(std::memory_order_acquire);
    uint64_t next_index = max_index + 1;
    if (max_index - min_index >= Size)
    {
        return ptr;
    }
    if (!m_max_index.compare_exchange_strong(max_index, next_index, std::memory_order_acquire))
    {
        return ptr;
    }
    ptr = &m_array[max_index & m_buf_mask];
    return ptr;
}

template <typename T, int Size>
void CLockLessQueBuf<T, Size>::commitWrite()
{
    m_tail.fetch_add(1, std::memory_order_release);
}

template <typename T, int Size>
T* CLockLessQueBuf<T, Size>::getReadBuf()
{
    T* ptr = NULL;
    uint64_t index = 0;
    uint64_t order_index = 0;
    order_index = m_tail.load(std::memory_order_acquire);
    index = m_head.load(std::memory_order_relaxed);
    if (index >= order_index)
    {
        return ptr;
    }
    //取值
    ptr = &m_array[index & m_buf_mask];
    return ptr;
}

template <typename T, int Size>
T* CLockLessQueBuf<T, Size>::preemtReadBuf()
{
    T* ptr = NULL;
    uint64_t index = 0;
    uint64_t order_index = 0;
    order_index = m_tail.load(std::memory_order_acquire);
    index = m_head.load(std::memory_order_acquire);
    if (index >= order_index)
    {
        return ptr;
    }
    if (!m_min_index.compare_exchange_strong(index, index + 1, std::memory_order_acquire))
    {
        T* ptr = NULL;
    }
    ptr = &m_array[index & m_buf_mask];
    return ptr;
}

template <typename T, int Size>
void CLockLessQueBuf<T, Size>::commitRead()
{
    m_head.fetch_add(1, std::memory_order_release);
}

}
