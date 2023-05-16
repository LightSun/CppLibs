#pragma once

#include <mutex>
#include <condition_variable>
#include "utils/h_atomic.h"

namespace h7 {

    class Barrier{
    public:
        Barrier(int reqCount):m_reqCount(reqCount){};

        bool reset(){
            if(h_atomic_cas(&m_finishCount, m_reqCount, 0)){
                h_atomic_set(&m_reached, 0);
                return true;
            }
            return false;
        }
        int getReqCount(){
            return m_reqCount;
        }
        int getFinishCount(){
            return h_atomic_get(&m_finishCount);
        }
        void wait(){
            await();
        }
        void await(){
            int old = h_atomic_add(&m_finishCount, 1);
            if(old == m_reqCount - 1){
                h_atomic_set(&m_reached, 1);
                {
                    std::unique_lock<std::mutex> lck(m_mutex);
                    m_cv.notify_all();
                }
            }else{
                std::unique_lock<std::mutex> lck(m_mutex);
                //use 'm_reached' avoid virtual notify.
                m_cv.wait(lck, [this](){
                   return h_atomic_get(&m_reached);
                });
            }
        }

    private:
        std::mutex m_mutex;
        std::condition_variable m_cv;
        int m_reqCount;
        volatile int m_finishCount {0};
        volatile int m_reached {0};
    };
}
