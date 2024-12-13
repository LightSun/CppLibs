#pragma once

#include <thread>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <shared_mutex>
#include <functional>
#include "utils/h_atomic.h"
#include "utils/locks.h"
#include "handler-os/src/_time.h"

namespace h7 {

using String = std::string;
using CString = const String&;

enum MonitorState{
    kMonitor_START = 1,
    kMonitor_END,
};
struct MonitorTask;

struct MonitorCategoryItem{
    using Func_StateProcessor = std::function<void(void*, MonitorTask*)>;

    String name {"Default"};
    int alarmDurationMs {500};
    int maxAlarmCount {3};

    void* ctx {nullptr};
    Func_StateProcessor funcState;
};

struct MonitorConfig{
    std::vector<MonitorCategoryItem> items;
};

struct MonitorTask{
    MonitorCategoryItem* cate {nullptr};
    String uid;
    long long startTime;
    int alarmCount {0};
    volatile int state {kMonitor_START};

    String getCategory(){
        return cate ? cate->name : "";
    }
    void doCallback(){
        if(cate && cate->funcState){
            cate->funcState(cate->ctx, this);
        }
    }
};

template<typename IMap = std::map<String,std::shared_ptr<MonitorTask>>>
struct RuntimeCategoryItem{
    using Func_StateProcessor = std::function<void(void*, MonitorTask*)>;
    using Func_AlarmProcessor = std::function<void(void*, MonitorTask*)>;

    String name;
    IMap m_map;
    mutable std::shared_mutex m_mutex;
    Func_AlarmProcessor m_funcAlarm;

    RuntimeCategoryItem(CString name): name(name){}

    bool isEmpty()const{
        std::shared_lock<std::shared_mutex> lck(m_mutex);
        return m_map.empty();
    }

    void reportStart(std::shared_ptr<MonitorTask> task){
        auto& uid = task->uid;
        std::unique_lock<std::shared_mutex> lck(m_mutex);
        auto it = m_map.emplace(uid, task);
        if(!it.second){
            fprintf(stderr, "reportStart >> failed. for (cate,uid) = (%s,%s)\n",
                    task->cate->name.data(), uid.data());
        }
    }
    void reportEnd(CString uid){
        std::shared_lock<std::shared_mutex> lck(m_mutex);
        auto it = m_map.find(uid);
        if(it != m_map.end()){
            h_atomic_set(&it->second->state, kMonitor_END);
        }
    }
    void reportOther(CString uid, int state){
        std::shared_lock<std::shared_mutex> lck(m_mutex);
        auto it = m_map.find(uid);
        if(it != m_map.end()){
            h_atomic_set(&it->second->state, state);
        }
    }
    void doLoop(){
        auto sysTime = h7_handler_os::getCurTime();
        std::vector<String> rmUids;
        {
            std::shared_lock<std::shared_mutex> lck(m_mutex);
            auto it = m_map.begin();
            for(; it != m_map.end(); ++it){
                int state = h_atomic_get(&it->second->state);
                switch (state) {
                case kMonitor_END:{
                    rmUids.push_back(it->second->uid);
                }break;

                case kMonitor_START:{
                    auto delta = sysTime - it->second->startTime;
                    if(delta >= it->second->cate->alarmDurationMs){
                        ++ it->second->alarmCount;
                        doAlarm(it->second.get());
                    }
                    if(it->second->alarmCount >= it->second->cate->maxAlarmCount){
                        rmUids.push_back(it->second->uid);
                    }
                }break;

                default:{
                    it->second->doCallback();
                }break;

                }
            }
        }
        {
            std::unique_lock<std::shared_mutex> lck(m_mutex);
            for(auto& ruid : rmUids){
                m_map.erase(ruid);
            }
        }
    }

private:
    void doAlarm(MonitorTask* alarm){
        if(m_funcAlarm){
            m_funcAlarm(alarm->cate->ctx, alarm);
        }else{
            String& uid = alarm->uid;
            fprintf(stderr, "[WARN] CommonMonitor >> Alarm-OverTime: cate,uid ="
                            "(%s, %s)\n",
                    name.data(),uid.data());
        }
    }
};

/**
 * common monitor used to listen some event,
 * like func-blocking, when occurs we have chance to known it.
 */
template<typename IMap = std::map<String,std::shared_ptr<MonitorTask>>>
class CommonMonitor{
public:
    using RTItem = RuntimeCategoryItem<IMap>;
    using UPRuntimeCategoryItem = std::unique_ptr<RTItem>;
    using SPMTask = std::shared_ptr<MonitorTask>;
    using KUMap_ = std::unordered_map<String, MonitorCategoryItem>;
    using Func_StateProcessor = std::function<void(void*, MonitorTask*)>;
    using Func_AlarmProcessor = std::function<void(void*, MonitorTask*)>;

    ~CommonMonitor(){
        stop();
    }
    CommonMonitor(const MonitorConfig& c = MonitorConfig()){
        if(!c.items.empty()){
            for(auto& it : c.items){
                m_catesMap.emplace(it.name, it);
            }
        }
        m_runtimeMap.emplace(m_defaultCate.name, std::make_unique<RTItem>(
                                 m_defaultCate.name));
    }
    void setStateCallback(CString cateName, Func_StateProcessor cb, void* ctx = nullptr){
        MonitorCategoryItem* ci = getCategoryItem(cateName);
        ci->ctx = ctx;
        ci->funcState = cb;
    }
    void setAlarmCallback(Func_AlarmProcessor fp){
        m_funcAlarm = fp;
    }
    MonitorCategoryItem& getDefaultCategoryItem(){
        return m_defaultCate;
    }
    void start(){
        if(h_atomic_cas(&m_started, 0, 1)){
            std::thread thd([this](){
                while (h_atomic_get(&m_reqStop) == 0) {
                    loopImpl();
                }
                h_atomic_set(&m_started, 0);
                h_atomic_set(&m_reqStop, 0);
                m_stopLock.notify();
            });
            thd.detach();
        }else{
            //already start
            fprintf(stderr, "HsyVideoMonitor >> aleady start.\n");
        }
    }
    void stop(){
        if(h_atomic_cas(&m_reqStop, 0, 1)){
            m_stopLock.wait();
        }
    }
    /**
     * @brief reportStart
     * @param cate empty means default category
     * @param uid unique id of this event.
     */
    void reportStart(CString cate, CString uid){
        MonitorCategoryItem* ci = getCategoryItem(cate);
        auto task = std::make_shared<MonitorTask>();
        task->uid = uid;
        task->cate = ci;
        task->startTime = h7_handler_os::getCurTime();
        auto item = getRuntimeCategoryItem(ci->name, true);
        item->reportStart(task);
    }
    void reportEnd(CString cate, CString uid){
        auto item = getRuntimeCategoryItem(cate.empty() ? m_defaultCate.name : cate, false);
        if(item != nullptr){
            item->reportEnd(uid);
        }
    }
    void reportOther(CString cate, CString uid, int state){
        auto item = getRuntimeCategoryItem(cate.empty() ? m_defaultCate.name : cate, false);
        if(item != nullptr){
            item->reportOther(uid, state);
        }
    }

private:
    MonitorCategoryItem* getCategoryItem(CString cate){
        MonitorCategoryItem* ci = nullptr;
        {
            auto it = m_catesMap.find(cate);
            if(it != m_catesMap.end()){
                ci = &it->second;
            }else{
                ci = &m_defaultCate;
            }
        }
        return ci;
    }
    RTItem* getRuntimeCategoryItem(CString cate, bool autoCreate){
        if(autoCreate){
            std::unique_lock<std::shared_mutex> lck(m_mutex);
            auto it = m_runtimeMap.find(cate);
            if(it != m_runtimeMap.end()){
                return it->second.get();
            }else{
                auto ins = m_runtimeMap.emplace(cate, std::make_unique<RTItem>(cate))
                        ->first.get();
                ins->m_funcAlarm = m_funcAlarm;
                return ins;
            }
        }else{
            std::shared_lock<std::shared_mutex> lck(m_mutex);
            auto it = m_runtimeMap.find(cate);
            if(it != m_runtimeMap.end()){
                return it->second.get();
            }else{
                return nullptr;
            }
        }
    }
    void loopImpl(){
        std::vector<RTItem*> items;
        {
            std::shared_lock<std::shared_mutex> lck(m_mutex);
            auto it = m_runtimeMap.begin();
            for(; it != m_runtimeMap.end() ; ++it){
                items.push_back(it->second.get());
            }
        }
        for(auto& rtItem : items){
            rtItem->doLoop();
            //no need remove empty RTItem.
            //if(rtItem->isEmpty()){}
        }
    }

private:
    MonitorCategoryItem m_defaultCate;
    KUMap_ m_catesMap;
    std::unordered_map<String, UPRuntimeCategoryItem> m_runtimeMap;
    mutable std::shared_mutex m_mutex;
    h7::MutexLock m_stopLock;
    volatile int m_reqStop {0};
    volatile int m_started {0};
    Func_AlarmProcessor m_funcAlarm;
};
}
