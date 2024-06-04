#pragma once

#include <list>
#include <memory>
#include <shared_mutex>
#include "utils/SaveQueue.h"
#include "utils/locks.h"

namespace h7 {
    template<typename T>
    class SafeQueue{
    public:
        using SaveQueuePtr = SaveQueue<T>*;
        SafeQueue(size_t buffer_size){
            m_queue = new SaveQueue<T>(buffer_size);
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
        SaveQueuePtr m_queue {nullptr};
        cacheline_pad_t pad1_;
        std::atomic_bool m_expanding {false};
        cacheline_pad_t pad2_;
    };
}
