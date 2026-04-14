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
#ifndef __NON_LOCK_LIST_QUEUE_H__
#define __NON_LOCK_LIST_QUEUE_H__
#include <stdlib.h>
#include <atomic>
#include <stdio.h>
#include <thread>

// 链表节点
template<typename T>
struct CNonLockListNode
{
    char pad[56] = {0};
    T* m_data = NULL;
    std::atomic<CNonLockListNode*> m_next = { 0 };
};

//无锁单链表(先进先出)
template<typename T>
class CNonLockListQueue
{
    typedef CNonLockListNode<T> Node;
    //! 指示头节点的节点
    Node m_ptr_head;
    //! 指示尾节点的节点
    char pad[64] = {0};
    Node m_ptr_tail;
    //! 元素个数
    char pad0[64] = {0};
    std::atomic<long long> m_count = { 0 };
    //! 最大长度 0表示不限长
    char pad1[56] = {0};
    long long m_max_num = 0;
public:
    CNonLockListQueue(long long max_num = 0);
    ~CNonLockListQueue();
public:
    //链尾插入
    void push_back(T* data);
    //链头取
    T* pop_front();
    //表长
    long long Size() {return m_count.load();}
};

template<typename T> CNonLockListQueue<T>::CNonLockListQueue(long long max_num)
    : m_max_num(max_num)
{
    //一个无data的起点
    Node* pNode = new Node;
    m_ptr_head.m_next = pNode;
    m_ptr_tail.m_next = pNode;
}

template<typename T> CNonLockListQueue<T>::~CNonLockListQueue()
{
    while (m_count.load())
    {
        T* val = pop_front();
        delete val;
    }
    if (m_ptr_head.m_next)
    {
        Node* pNode = m_ptr_head.m_next.load();
        delete pNode;
    }
}

template<typename T>
void CNonLockListQueue<T>::push_back(T* data)
{
    Node* node = new Node;
    Node* ptr_node = NULL;
    Node* ptr_tail = NULL;
    do
    {
        //限制长度
        if (m_count >= m_max_num && m_max_num > 0)
        {
            continue;
        }
        //取空了 要等ptr_tail.next被设成一个有效指针才能解除锁定
        ptr_tail = m_ptr_tail.m_next.load();
        if (!ptr_tail)
        {
            continue;
        }
        ptr_node = NULL;
        //让ptr_tail.next暂时为NULL 配合上面的代码 形成一个资源锁
        if (!m_ptr_tail.m_next.compare_exchange_weak(ptr_tail, ptr_node))
        {
            continue;
        }
        //node加入链表后 让tail标记到这个node
        if (!ptr_tail->m_data)                        //处理慢速的时候 只有一个有效element的会先取个NULL-Element
        {
            ptr_tail->m_data = data;
            m_ptr_tail.m_next.compare_exchange_strong(ptr_node, ptr_tail);
            m_count++;
            delete node;
            break;
        }
        node->m_data = data;
        if (ptr_tail->m_next.compare_exchange_weak(ptr_node, node))
        {
            //ptr_node = NULL;
            m_ptr_tail.m_next.compare_exchange_strong(ptr_node, ptr_tail->m_next.load());           //tail指向新节点
            m_count++;
            break;
        }
    } while (1);
}

template<typename T> T* CNonLockListQueue<T>::pop_front()
{
    T* ptr_data = NULL;
    Node* ptr_node = NULL;
    Node* ptr_test_node = NULL;
    Node* ptr_tail = NULL;
    do
    {
        //资源锁定
        ptr_node = m_ptr_head.m_next.load();
        if (!ptr_node)
        {
           // break;
            continue;
        }
        //配合上面的代码形成资源锁
        if (!m_ptr_head.m_next.compare_exchange_weak(ptr_node, NULL))
        {
            continue;
        }
        ptr_test_node = ptr_node->m_next.load();
        //此刻看到的是有效的最后一个节点  后续操作可能链表中又加入了数据
        if (!ptr_test_node)
        {
            //去锁一下tail
            do
            {
                ptr_tail = m_ptr_tail.m_next.load();
                if (!ptr_tail)
                {
                    continue;
                }
                //确定tail在锁定的时候 无新插入的节点
                if (!m_ptr_tail.m_next.compare_exchange_weak(ptr_tail, NULL))
                {
                    continue;
                }
                //锁成功
                break;
            } while (1);
        }
        //得到数据
        ptr_data = ptr_node->m_data;
        //锁成功的
        if (!ptr_test_node)
        {
            //因为是当前视界内的最后一个节点 这个节点就不删了
            ptr_node->m_data = NULL;
            m_ptr_head.m_next.store(ptr_node);
            m_ptr_tail.m_next.store(ptr_tail);
        }
        else
        {
            m_ptr_head.m_next.store(ptr_test_node);
            //等到写入线程释放后 再释放节点
            while (!m_ptr_tail.m_next.load());
            delete ptr_node;
        }
        break;
    } while (1);
    if (ptr_data)
        m_count--;
    return ptr_data;
}
#endif
