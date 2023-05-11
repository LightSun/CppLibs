#ifndef TASK_LIST_HPP
#define TASK_LIST_HPP

#include "common/common.h"
#include "common/c_common.h"

namespace h7 {

    template <typename T>
    class TaskList{
    public:
        typedef std::chrono::time_point<std::chrono::steady_clock> ClockTimeUnit;
        typedef std::chrono::steady_clock DelayClock;

        template<typename R>
        struct Msg
        {
            String tag;
            DelayClock::time_point startTime;
            R task;
        };
        void clear(){
            std::unique_lock<std::mutex> lock(m_mutex);
            m_tasks.clear();
        }
        int size(){
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_tasks.size();
        }
        void addTask(String tag, const T& task){
            std::unique_lock<std::mutex> lock(m_mutex);
            Msg<T> msg;
            msg.tag = tag;
            msg.startTime = DelayClock::now();
            msg.task = task;
            m_tasks.push_back(std::move(msg));
        }
        void removeTask(const T& task){
            std::unique_lock<std::mutex> lock(m_mutex);
            int c = m_tasks.size();
            for(int i = c - 1; i >= 0 ; i--){
                if(m_tasks[i].task == task){
                    m_tasks.erase(m_tasks.begin() + i);
                    break;
                }
            }
        }
        void removeTaskIfDelay(float delay){
            std::unique_lock<std::mutex> lock(m_mutex);
            auto curTime = DelayClock::now();
            int c = m_tasks.size();
            for(int i = c - 1; i >= 0 ; i--){
                auto _delay = std::chrono::duration<float, std::milli>(
                           curTime - m_tasks[i].startTime).count();
                if(_delay >= delay){
                    m_tasks.erase(m_tasks.begin() + i);
                }
            }
        }
        bool hashTask(const T& task){
            std::unique_lock<std::mutex> lock(m_mutex);
            int c = m_tasks.size();
            for(int i = c - 1; i >= 0 ; i--){
                if(m_tasks[i].task == task){
                    return true;
                }
            }
            return false;
        }
        bool hashTag(CString tag){
            std::unique_lock<std::mutex> lock(m_mutex);
            int c = m_tasks.size();
            for(int i = c - 1; i >= 0 ; i--){
                if(m_tasks[i].tag == tag){
                    return true;
                }
            }
            return false;
        }
        int tagCount(CString tag){
            std::unique_lock<std::mutex> lock(m_mutex);
            int c = m_tasks.size();
            int ret = 0;
            for(int i = c - 1; i >= 0 ; i--){
                if(m_tasks[i].tag == tag){
                    ret ++;
                }
            }
            return ret;
        }
        void drainTagTasksTo(CString tag, std::vector<T>& out){
            std::unique_lock<std::mutex> lock(m_mutex);
            int c = m_tasks.size();
            for(int i = c - 1; i >= 0 ; i--){
                if(m_tasks[i].tag == tag){
                    out.push_back(m_tasks[i].task);
                }
            }
        }
    private:
        std::vector<Msg<T>> m_tasks;
        std::mutex m_mutex;
    };

}

#endif
