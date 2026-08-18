#pragma once

#include <functional>
#include <memory>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <atomic>
#include <future>

namespace h7_component {

enum Format{
    kFormat_NONE = -1,
    kFormat_JSON,
    kFormat_BIN,
};

struct ShareData{
    std::shared_ptr<void> data;
    std::string tag;
    int format {kFormat_NONE};
};
typedef const ShareData& CSpContext;
typedef ShareData SpContext;

using String = std::string;
using CString = const std::string&;
template<typename T>
using List = std::vector<T>;
//
class System;

enum ServiceEventEnum{
    kServiceEvent_NONE = -1,
    kServiceEvent_INIT,
    kServiceEvent_LOAD,
    kServiceEvent_UNLOAD,
    kServiceEvent_RUN,
    //
    kServiceEvent_REGISTER = 100,
    kServiceEvent_UNREGISTER,
    //
    kServiceEvent_RUN_ALL,
    kServiceEvent_RUN_ONE,
    kServiceEvent_RUN_MULTI_THREAD,
};

struct ServiceEnvent{
    int event {kServiceEvent_NONE};
    //for simple-event, this indicate success or failed.
    //for group-event, this indicate open or close.
    bool state {false};
    String name;
    SpContext ctx;
    SpContext saveState;
};
using CServiceEnvent = const ServiceEnvent&;

class IServiceListener{
public:
    virtual ~IServiceListener(){}

    //saveState only for run event
    virtual void onEvent(System* sys, CServiceEnvent event) = 0;
    virtual void onGroupEvent(System*, CServiceEnvent){};
};

struct ServiceInfo{
    String name;
    String group;
};

class IService{
public:
    virtual ~IService(){}

    String& getName(){
        return getServiceInfo()->name;
    }
    String& getGroup(){
        return getServiceInfo()->group;
    }
    //should called after init
    virtual ServiceInfo* getServiceInfo(){
        return &m_info;
    }
    virtual bool init(CSpContext data) = 0;

    virtual bool load(CSpContext ctx) = 0;

    virtual bool unload(CSpContext ctx) = 0;

    virtual bool run(CSpContext ctx, CSpContext saveState) = 0;

protected:
    ServiceInfo m_info;
};

class WrapService: public IService{
public:
    WrapService(System* l, std::shared_ptr<IService> base): system(l), base(base){}

    ServiceInfo* getServiceInfo()final{
        return base->getServiceInfo();
    }

    bool init(CSpContext ctx) final;

    bool load(CSpContext ctx) final;

    bool unload(CSpContext ctx) final;

    bool run(CSpContext ctx, CSpContext saveState) final;

public:
    System* system;
    std::shared_ptr<IService> base;

private:
    friend class WrapServiceHolder;
    friend class GroupService;
    std::atomic_bool m_busy {false};
};

struct WrapServiceHolder{
    WrapService* s;

    WrapServiceHolder(WrapService* s): s(s){}
    ~WrapServiceHolder(){
        if(locked){
            s->m_busy = false;
        }
    }
    bool casBusy(){
        bool busy = false;
        if(s->m_busy.compare_exchange_strong(busy, true)){
            locked = true;
            return true;
        }
        return false;
    }

private:
    bool locked {false};
};

class IScheduler{
public:
    virtual ~IScheduler(){}

    virtual void schedule(std::function<void()> func) = 0;
};

struct ThreadParameter{
    enum{
        kState_UNKNOWN = -1, //may be break by same-thread failed service.
        kState_FAILED,
        kState_OK,
    };
    SpContext ctx;
    SpContext saveState;
    int tcnt {0};  //thread cnt
    int scnt {0};  //service cnt
   // int tidx {-1}; //thread idx. >= 0
   // int sidx {-1}; //service idx. >= 0
    List<int> serviceStates;
    List<std::shared_ptr<IService>> services;
    std::atomic_int runnedCnt {0}; //the runned cnt of service, but may contains failed or unknown.

    List<std::shared_ptr<IService>> getFailedServices(){
        List<std::shared_ptr<IService>> vec;
        int size = services.size();
        for(int i = 0 ; i < size ; ++i){
            if(serviceStates[i] == kState_FAILED){
                vec.push_back(services[i]);
            }
        }
        return vec;
    }
    List<std::shared_ptr<IService>> getNonSuccessServices(){
        List<std::shared_ptr<IService>> vec;
        int size = services.size();
        for(int i = 0 ; i < size ; ++i){
            if(serviceStates[i] != kState_OK){
                vec.push_back(services[i]);
            }
        }
        return vec;
    }
    //return runned cnt
    void addRunCnt(int delta = 1){
        runnedCnt.fetch_add(delta);
    }
    bool isAllSuccess()const{
        for(auto& s : serviceStates){
            if(s != 1){
                return false;
            }
        }
        return true;
    }
    int getRunCnt(){
        return runnedCnt.load();
    }
    bool isAllRunned(){
        return getRunCnt() == (int)services.size();
    }
};

class GroupService{
public:
    typedef std::shared_ptr<IService> ServiceApi;
    using SPTP = std::shared_ptr<ThreadParameter>;

    GroupService(System* sys, List<ServiceApi> apis): sys(sys),m_apis(apis){}

    int getServiceCnt()const{return m_apis.size();}

    List<ServiceApi> runAll(CSpContext ctx, CSpContext saveState){
        dispatchGroupEvent(sys, kServiceEvent_RUN_ALL, true);
        List<ServiceApi> failedVec;
        for(auto& s : m_apis){
            if(!s->run(ctx, saveState)){
                failedVec.push_back(s);
            }
        }
        dispatchGroupEvent(sys, kServiceEvent_RUN_ALL, false);
        return failedVec;
    }
    bool runOne(CSpContext ctx, CSpContext saveState){
        dispatchGroupEvent(sys, kServiceEvent_RUN_ONE, true);
        for(auto& s : m_apis){
            if(s->run(ctx, saveState)){
                return true;
            }
        }
        dispatchGroupEvent(sys, kServiceEvent_RUN_ONE, false);
        return false;
    }
    /**
     * @brief runMultiThread
     * @param ctx : the context of run
     * @param saveState :the mediator-state of run
     * @param tc  :the thread cnt
     * @param scheduler: the thread scheduler
     * @param func : the func to run. args[1] is thread idx(>=0), args[2] is service idx(>=0).
     * @param final : the final task, if all task runned(can be failed/unknown).
     *                be called from the any-scheduler thread.
     * @param breakIfAnyFailed: should break if any service failed
     * @return the run param.
     */
    SPTP runMultiThread(CSpContext ctx, CSpContext saveState, int tc, IScheduler* scheduler,
                        std::function<bool(SPTP,int,int,ServiceApi)> func,
                        std::function<void(SPTP)> final,
                        bool breakIfAnyFailed){
        dispatchGroupEvent(sys, kServiceEvent_RUN_MULTI_THREAD, true);
        //TODO check tc > 0
        const int c = m_apis.size();
        tc = c < tc ? c : tc;
        //
        const int every = c / tc;
        const int left = c - tc * every;
        //
        using PKT = std::packaged_task<bool(int,ServiceApi)>;
        auto tp = std::make_shared<ThreadParameter>();
        tp->ctx = ctx;
        tp->saveState = saveState;
        tp->tcnt = tc;
        tp->scnt = c;
        tp->serviceStates = List<int>(c, ThreadParameter::kState_UNKNOWN);
        tp->services = m_apis;
        //
        auto finalTask = std::make_shared<std::packaged_task<void(SPTP)>>(final);
        auto anyFailed = std::make_shared<std::atomic_bool>(false);
        System* sys = this->sys;
        for(int i = 0, si = 0; i < tc ; ++i){
            const int ki = si;
            const int act_count = i < left ? every + 1 : every;
            si += act_count;
            int tidx = i;
            //
            auto func0 = [func, tp, tidx](int sidx, ServiceApi api){
                func(tp, tidx, sidx, api);
            };
            auto task = std::make_shared<PKT>(
                std::bind(func0, std::placeholders::_1, std::placeholders::_2)
                );
            //
            scheduler->schedule([sys, finalTask, task, tp, ki, act_count, anyFailed, breakIfAnyFailed](){
                for(int k = ki, kend = ki + act_count; k < kend ; ++k){
                    if(breakIfAnyFailed && anyFailed->load()){
                        tp->addRunCnt(kend - k);
                        break;
                    }
                    auto fut = task->get_future();
                    (*task)(k, tp->services[k]);
                    bool ret = fut.get();
                    task->reset();
                    //printf(" k = %d, states_i = %d\n", k, k - start);
                    tp->serviceStates[k] = ret ? ThreadParameter::kState_OK
                                               : ThreadParameter::kState_FAILED;
                    tp->addRunCnt();
                    //check
                    if(breakIfAnyFailed && !ret){
                        anyFailed->store(true);
                        break;
                    }
                }
                if(tp->isAllRunned()){
                    dispatchGroupEvent(sys, kServiceEvent_RUN_MULTI_THREAD, false);
                    (*finalTask)(tp);
                }
            });
        }
        return tp;
    }
    inline static void dispatchGroupEvent(System* sys,int event, bool beginOrEnd);

private:
    System* sys;
    List<ServiceApi> m_apis;
};

class System{
    typedef std::shared_ptr<IService> ServiceApi;
    typedef std::shared_ptr<WrapService> WrapServiceApi;
public:
    void setServiceListener(std::unique_ptr<IServiceListener> l){
        this->m_listener = std::move(l);
    }
    //register and return the service name.
    //pair: state,msg
    std::pair<bool,String> registerService(ServiceApi service, SpContext init_env){
        if(service->init(init_env)){
            auto& name = service->getServiceInfo()->name;
            auto preService = getService(name);
            if(preService == nullptr){
                std::unique_lock<std::mutex> mtx(m_serviceMapMux);
                m_serviceMap[name] = std::make_shared<WrapService>(this, service);
            }else{
                dispatchEvent(name, kServiceEvent_REGISTER, false, init_env);
                return std::make_pair<>(false, name);
            }
            dispatchEvent(name, kServiceEvent_REGISTER, true, init_env);
            return std::make_pair<>(true, name);
        }
        return std::make_pair<>(false, "");;
    }
    ServiceApi unregisterService(CString name){
        ServiceApi api;
        {
            std::unique_lock<std::mutex> mtx(m_serviceMapMux);
            auto it = m_serviceMap.find(name);
            if(it != m_serviceMap.end()){
                api = it->second->base;
                m_serviceMap.erase(it);
                dispatchEvent(name, kServiceEvent_UNREGISTER, true, SpContext());
            }
        }
        return api;
    }
    ServiceApi getService(CString name){
        std::unique_lock<std::mutex> mtx(m_serviceMapMux);
        auto it = m_serviceMap.find(name);
        if(it != m_serviceMap.end()){
            return it->second->base;
        }
        return nullptr;
    }
    GroupService group(CString group){
        List<ServiceApi> apis;
        {
            std::unique_lock<std::mutex> mtx(m_serviceMapMux);
            for(auto& [k,v] : m_serviceMap){
                if(v->getGroup() == group){
                    apis.push_back(v);
                }
            }
        }
        return GroupService(this, apis);
    }
    GroupService groupContains(CString group){
        List<ServiceApi> apis;
        {
            std::unique_lock<std::mutex> mtx(m_serviceMapMux);
            for(auto& [k,v] : m_serviceMap){
                if(v->getGroup().find(group) != String::npos){
                    apis.push_back(v);
                }
            }
        }
        return GroupService(this, apis);
    }
    void dispatchEvent(CString serviceName, int event, bool state,
                       CSpContext ctx, CSpContext saveState = SpContext()){
        if(m_listener){
            ServiceEnvent e;
            e.name = serviceName;
            e.state = state;
            e.event = event;
            e.ctx = ctx;
            e.saveState = saveState;
            m_listener->onEvent(this, e);
        }
    }
    void dispatchGroupEvent(int event, bool beginOrEnd){
        if(m_listener){
            ServiceEnvent e;
            e.event = event;
            e.state = beginOrEnd;
            m_listener->onGroupEvent(this, e);
        }
    }
private:
    std::unordered_map<String, WrapServiceApi> m_serviceMap;
    std::unique_ptr<IServiceListener> m_listener;
    std::mutex m_serviceMapMux;
};
//------
//----------------- impl ------------
//
bool WrapService::init(CSpContext ctx){
    bool result = false;
    {
        WrapServiceHolder holder(this);
        if(holder.casBusy()){
            result = base->init(ctx);
        }
    }
    system->dispatchEvent(getName(), kServiceEvent_INIT, result, ctx);
    return result;
}
bool WrapService::load(CSpContext ctx){
    bool result = false;
    {
        WrapServiceHolder holder(this);
        if(holder.casBusy()){
            result = base->load(ctx);
        }
    }
    system->dispatchEvent(getName(), kServiceEvent_LOAD, result, ctx);
    return result;
}
bool WrapService::unload(CSpContext ctx){
    bool result = false;
    {
        WrapServiceHolder holder(this);
        if(holder.casBusy()){
            result = base->unload(ctx);
        }
    }
    system->dispatchEvent(getName(), kServiceEvent_UNLOAD, result, ctx);
    return result;
}
bool WrapService::run(CSpContext ctx, CSpContext saveState){
    bool result = false;
    {
        WrapServiceHolder holder(this);
        if(holder.casBusy()){
            result = base->run(ctx, saveState);
        }
    }
    system->dispatchEvent(getName(), kServiceEvent_RUN, result, ctx, saveState);
    return result;
}
//----------------
void GroupService::dispatchGroupEvent(System* sys,int event, bool beginOrEnd){
    sys->dispatchGroupEvent(event, beginOrEnd);
}
}
