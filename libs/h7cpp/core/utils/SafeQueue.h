#pragma once

#include <list>
#include <memory>
#include <shared_mutex>
#include "utils/SaveQueue.h"
#include "utils/locks.h"

namespace h7 {
/**
 *relative to SaveQueue. this queue support auto increate buffer_size. by << 1.
 */
    template<typename T>
    class SafeQueue{
    public:
        using SQ = SaveQueue<T>;

        SafeQueue(size_t buffer_size){
            m_queue = new SQ(buffer_size);
        }
        ~SafeQueue(){
            if(m_queue){
                delete m_queue;
                m_queue = nullptr;
            }
        }
        size_t size(){
            while (m_expanding.load(std::memory_order_acquire)) {
                //wait
            }
            return m_queue->size();
        }
        size_t bufferSize(){
            while (m_expanding.load(std::memory_order_acquire)) {
                //wait
            }
            return m_queue->bufferSize();
        }

        bool enqueue(T const& data){
            while (m_expanding.load(std::memory_order_acquire)) {
                //wait
            }
            int failedC = 0;
            while (failedC < 3) {
                if(m_queue->enqueue(data)){
                    return true;
                }
                failedC ++;
            }
            m_expanding.store(true, std::memory_order_relaxed);
            m_queue->expand();
            m_expanding.store(false, std::memory_order_relaxed);
            return enqueue(data);
        }
        bool dequeue(T& data){
            while (m_expanding.load(std::memory_order_acquire)) {
                //wait
            }
            return m_queue->dequeue(data);
        }

    private:
        static size_t const     cacheline_size = 64;
        typedef char            cacheline_pad_t [cacheline_size];
        cacheline_pad_t pad0_;
        SQ* m_queue {nullptr};
        cacheline_pad_t pad1_;
        std::atomic_bool m_expanding {false};
        cacheline_pad_t pad2_;
    };
}
