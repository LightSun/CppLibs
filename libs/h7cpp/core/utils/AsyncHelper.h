#pragma once

#include <vector>
#include <functional>
#include <thread>
#include <future>
#include "core/utils/locks.h"
#include "core/utils/h_atomic.h"

namespace h7 {

//every task run in loop.
class AsyncMultiTask{
public:
    using Func = std::function<void()>;

    /**
     * @brief start start tasks .
     * @param vec every task will run until stop.
     * @return true if start ok.
     */
    bool start(const std::vector<Func>& vec){
        if(h_atomic_cas(&m_started, 0, 1)){
            m_vec = vec;
            h_atomic_set(&m_runCount, vec.size());
            const int size = vec.size();
            for(int i = 0 ; i < size ; ++i){
                const int idx = i;
                std::thread thd([this, idx](){
                    while (h_atomic_get(&m_reqStop) == 0) {
                        m_vec[idx]();
                    }
                    if(h_atomic_add(&m_runCount, -1) == 1){
                        mMLock.notify(true);
                    }
                });
                thd.detach();
            }
            return true;
        }
        return false;
    }
    //func: called on stop success.
    void stop(Func func = nullptr){
        if(h_atomic_cas(&m_reqStop, 0, 1)){
            mMLock.wait();
            reset();
            if(func){
                func();
            }
        }
    }
private:
    void reset(){
        h_atomic_set(&m_runCount, 0);
        h_atomic_set(&m_started, 0);
        h_atomic_set(&m_reqStop, 0);
    }

private:
    std::vector<Func> m_vec;
    MutexLock mMLock;
    int volatile m_runCount {0};
    int volatile m_started {0};
    int volatile m_reqStop {0};
};
}
