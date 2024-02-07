#pragma once
#include "common/common.h"
#include "common/c_common.h"
#include "utils/h_atomic.h"

namespace h7 {

class Timer {

public:
    Timer() {
    }

    enum {
        kSTATE_IDLE,
        kSTATE_STARTED,
        kSTATE_REQ_STOP,
    };

    void schedulePeriodically(uint32 intervalMs, std::function<void()> task)
    {
        {
            auto val = h_atomic_get(&m_state);
            if(val == kSTATE_STARTED || val == kSTATE_REQ_STOP){
                return;
            }
        }
        h_atomic_set(&m_state, kSTATE_STARTED);
        FUNC_SHARED_PTR(void()) ptr = FUNC_MAKE_SHARED_PTR_0(void(), task);
        std::thread([this, intervalMs, ptr]() {

            while (h_atomic_get(&m_state) != kSTATE_REQ_STOP)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
                (*ptr)();
                ptr->reset();
            }
            h_atomic_set(&m_state, kSTATE_IDLE);
            if(m_ptr_af_stop){
                (*m_ptr_af_stop)();
                m_ptr_af_stop = nullptr;
            }
        }).detach();
    }

    int getState(){
        return h_atomic_get(&m_state);
    }
    void stop(){
        h_atomic_set(&m_state, kSTATE_REQ_STOP);
    }

    void stop(std::function<void()> task){
        if(task){
            m_ptr_af_stop = FUNC_MAKE_SHARED_PTR_0(void(), task);
        }
        h_atomic_set(&m_state, kSTATE_REQ_STOP);
    }

    static void scheduleDelay(uint32 intervalMs, std::function<void()> task){
        FUNC_SHARED_PTR(void()) ptr = FUNC_MAKE_SHARED_PTR_0(void(), task);
        std::thread([intervalMs, ptr]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
            (*ptr)();
        }).detach();
    }
private:
    volatile int m_state {kSTATE_IDLE};
    FUNC_SHARED_PTR(void()) m_ptr_af_stop;
 };

}
