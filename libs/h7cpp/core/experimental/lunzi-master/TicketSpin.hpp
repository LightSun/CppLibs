#pragma once

#include <atomic>
#define _TICKET_SPIN

namespace px
{
class _TICKET_SPIN CSpinLock
{
#if 0
    union Ticket
    {
        unsigned int used = 0;
        struct
        {
            unsigned short _tickets;
            unsigned short _users;
        };
    };

    Ticket m_ticket;
#endif
    std::atomic<unsigned short> m_users;
    unsigned short m_ticket;
public:
    CSpinLock() : m_users(0), m_ticket(0) {}
    CSpinLock(const CSpinLock&) = delete;
    CSpinLock& operator=(const CSpinLock&) = delete;

public:
    void lock()
    {
        unsigned short _this = m_users.fetch_add(1, std::memory_order_acquire);
        while (_this != m_ticket)
        {
            std::atomic_thread_fence(std::memory_order_acquire);
        }
    }

    void unlock()
    {
        ++m_ticket;
        std::atomic_thread_fence(std::memory_order_release);
    }
};
}
