#include "utils/Executor.h"

using namespace h7;

void Executor::prepare(){
    for(int i = 0 ; i < m_threadCount ; i ++){
        m_threads.emplace_back([this](){
            while (h_atomic_get(&m_cmd) != kReqStop) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->m_mutex);
                    this->m_condition.wait(lock,
                        [this]{ return h_atomic_get(&m_cmd) == kReqStop
                                || !this->m_tasks.empty(); });
                    if(h_atomic_get(&m_cmd) == kReqStop)
                        return;
                    if(this->m_tasks.empty()){
                        continue;
                    }
                    task = std::move(this->m_tasks.front());
                    this->m_tasks.pop();
                }
                task();
            }
        });
    }
}
