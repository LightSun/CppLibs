#pragma once

#include <string>
#include "core/experimental/GMP.h"
#include "core/utils/h_atomic.h"
#include "core/utils/_time.h"
#include "core/utils/FriendAtomic.h"
#include "core/utils/InsertionOrderMap.h"

namespace h7 {

// MESI状态枚举
enum class CacheState {
    MODIFIED,   // M - 修改，数据只存在于当前缓存且已被修改
    EXCLUSIVE,  // E - 独占，数据只存在于当前缓存且未被修改
    SHARED,     // S - 共享，数据存在于多个缓存
    INVALID     // I - 无效，数据不可用
};

std::string stateToString(CacheState state) {
    switch (state) {
    case CacheState::MODIFIED: return "M";
    case CacheState::EXCLUSIVE: return "E";
    case CacheState::SHARED: return "S";
    case CacheState::INVALID: return "I";
    default: return "?";
    }
}

template<typename Addr, typename Data>
struct CacheLine{
    FriendAtomicI32 state {(int)CacheState::INVALID};
    FriendAtomicU32 useCnt {0};
    Addr addr;
    Data data;

    void setData(const Data& d){
        this->data = d;
    }
    bool isTargetLine(const Addr& addr, int neqState){
        return this->addr == addr && state != neqState;
    }
    bool isTargetLine(const Addr& addr){
        return this->addr == addr;
    }
    void setAll(const Addr& addr, const Data& val, int state){
        this->useCnt = 1;
        this->addr = addr;
        this->data = val;
        this->state = state;
    }
    void set(const Data& val, int state){
        this->useCnt.add(1);
        this->data = val;
        this->state = state;
    }
    Addr& getAddress(){
        return addr;
    }
    Data& getData(){
        return data;
    }
};

//TODO need sync
template<typename Addr, typename Data>
class MainCache{
public:
    Data* get(const Addr& addr){
        return m_cache.get(addr);
    }
    bool get(const Addr& addr, Data& out){
        return m_cache.get(addr, out);
    }
    void set(const Addr& addr, const Data& t){
        m_cache.insert(addr, t);
    }
private:
    InsertionOrderMap<Addr,Data> m_cache;
};

template<typename Addr, typename Data>
class MultiCoreProcessor{
    typedef CacheLine<Addr, Data> CacheLineImpl;
    typedef CacheLineImpl* CacheLinePtr;
public:
    std::vector<std::vector<CacheLineImpl>> m_L1_cache;  // 4个核的私有L1缓存
    std::vector<std::vector<CacheLinePtr>> m_L1_cachePtrs;
    std::vector<CacheLineImpl> m_L2_shared;    // 共享L2缓存
    std::mutex m_bus_mutex;                    // 模拟总线仲裁
    MainCache<Addr, Data> m_mainCache;           // 主内存

    MultiCoreProcessor(int coreSize, int l1_cache_size, int l2_cache_size){
        m_L1_cache.resize(coreSize);
        m_L1_cachePtrs.resize(coreSize);
        for (int i = 0; i < coreSize; i++){
            m_L1_cache[i].resize(l1_cache_size);
        }
        for (int i = 0; i < coreSize; i++){
            for(auto& line: m_L1_cache[i]){
                m_L1_cachePtrs[i].push_back(&line);
            }
        }
        m_L2_shared.resize(l2_cache_size);
    }

    // 总线事务处理器
    void busTransaction(int core_id, const Addr& address, bool op_write, Data* value = nullptr) {
        std::lock_guard<std::mutex> lock(m_bus_mutex);  // 模拟总线仲裁

        // 通知其他核心进行监听
        for (int i = 0; i < 4; i++) {
            if (i != core_id) {
                snoopCache(i, address, op_write, core_id);
            }
        }
        // 处理内存访问
        if (!op_write) {
            if (value) {
                m_mainCache.get(address, *value);
            }
        } else{
            if (value) {
                m_mainCache.set(address, *value);
            }
        }
    }

    // 缓存监听
    void snoopCache(int core_id, const Addr& address, bool op_write, int requestor_id) {
        for(auto& line: m_L1_cache[core_id]){
            if(line.isTargetLine(address, CacheState::INVALID)){
                line.useCnt.add(1);
                auto oldState = line.state.load();
                if(op_write){
                    //总线写请求（读无效）
                    line.state = CacheState::INVALID;
                }else{
                    if(oldState == CacheState::MODIFIED){
                        // 写回数据到内存
                        //TODO data need atomic ?
                        m_mainCache.set(address, line.getData());
                    }else if(oldState == CacheState::EXCLUSIVE){
                        line.state = CacheState::SHARED;
                    }
                }
                break;
            }
        }
    }
    Data read(int core_id, const Addr& address){
        for(auto& line: m_L1_cache[core_id]){
            if(line.isTargetLine(address)){
                line.useCnt.add(1);
                auto state = line.state.load();
                if(state != CacheState::INVALID){
                    //缓存命中
                    return line.getData();
                }
            }
        }
        // 缓存未命中，发起总线事务
        Data value;
        busTransaction(core_id, address, false, &value);
        // 分配缓存行
        allocateCacheLine(core_id, address, value, CacheState::EXCLUSIVE);
        return value;
    }
    //写入数据（模拟线程写入共享变量）
    void write(int core_id, const Addr& address, const Data& value){
        for(auto& line: m_L1_cache[core_id]){
            if(line.isTargetLine(address)){
                auto state = line.state.load();
                switch (state) {
                case CacheState::MODIFIED:
                case CacheState::EXCLUSIVE:{
                    // 可以本地写入
                    line.set(value, state);
                    return;
                }break;

                case CacheState::SHARED:{
                    //需要升级到MODIFIED，发送总线无效
                    busTransaction(core_id, address, true);
                    line.set(value, state);
                    return;
                }break;
                }
            }
        }
        // 写未命中，写分配策略
        busTransaction(core_id, address, true);
        //
        allocateCacheLine(core_id, address, value, CacheState::MODIFIED);
    }
private:
    void allocateCacheLine(int core_id, const Addr& address, const Data& value, CacheState state){
         //替换第一个无效行，否则替换 少使用的那个
        for (auto& line : m_L1_cache[core_id]) {
            if (line.state.load() == CacheState::INVALID) {
                line.setAll(address, value, state);
                return;
            }
        }
        auto& ptrs = m_L1_cachePtrs[core_id];
        std::sort(ptrs.begin(), ptrs.end(), [](const CacheLinePtr& p1, const CacheLinePtr& p2){
            return p1->useCnt.load() > p2->useCnt.load();
        });
        auto& line = *ptrs.back();
        // 如果被替换的行是MODIFIED，需要写回
        if(line.state == CacheState::MODIFIED){
            m_mainCache.set(line.getAddress(), line.getData());
        }
        line.setAll(address, value, state);
    }
};

//for MESI: every ProcessorCore should have one GlobalImpl
template<typename Addr, typename Data>
class ProcessorCore{
    typedef CacheLine<Addr, Data> CacheLineImpl;
    typedef CacheLineImpl* CacheLinePtr;
    typedef IGlobalManager<CacheLineImpl> GlobalImpl;
    typedef ILocalManager<CacheLineImpl> LocalImpl;
public:
    ProcessorCore(GlobalImpl* global, size_t cacheInitSize = 0):m_global(global){
        m_local.attach(global);
        if(cacheInitSize > 0){
            m_local.resizeQueue(cacheInitSize);
        }
    }
    ~ProcessorCore(){
        m_local.detach();
    }
    //initData: not null means need init if not exist.
    //return local-cache hit or not.
    //here not focus on sync with all core and mem.
    bool read(const Addr& addr, Data& outData, const Data* initData){
        auto cachePtr = m_local.getElementByLocal([&addr](CacheLineImpl& fd){
            return fd.addr == addr && fd.state != CacheState::INVALID;
        });
        if(cachePtr != nullptr){
            cachePtr->useCnt ++;
            outData = cachePtr->data;
            return true;
        }
        auto otherPtr = m_local.getElementByGlobal([&addr](CacheLineImpl& fd){
            return fd.addr == addr && fd.state != CacheState::INVALID;
        });
        if(otherPtr){
            otherPtr->useCnt ++;
            outData = otherPtr->data;
        }else{
            outData = *initData;
        }
        //
        cachePtr = m_local.getElementByLocal([](CacheLineImpl& fd){
            return fd.state == CacheState::INVALID;
        });
        if(cachePtr == nullptr){
            //LRU. flush to main-memory. then reset it state to INVALID
            //DESC
            m_local.getQueue()->sort([](const CacheLineImpl& c1, const CacheLineImpl& c2){
                return c1.useCnt > c2.useCnt;
            });
            cachePtr = m_local.getQueue()->back();
            //flush to main-mem
            if(cachePtr->state == CacheState::MODIFIED){
                auto mainItem = m_global->getQueue()->get([&addr](CacheLineImpl& fd){
                    return fd.addr == addr;
                });
                if(mainItem != nullptr){
                    while (mainItem->isLocked()){
                    }
                    //check update.
                    mainItem->markLocked();
                    mainItem->setData(cachePtr->data);
                    mainItem->markUnLocked();
                }
            }
            cachePtr->useCnt = 1;
        }else{
            cachePtr->useCnt ++;
        }
        //cachePtr->markLocked();
        cachePtr->setData(outData);
        cachePtr->addr = addr;
        cachePtr->state = CacheState::EXCLUSIVE;
        //cachePtr->markUnLocked();
        return false;
    }
    bool write(const Addr& addr, const Data& newData){
        auto cachePtr = m_local.nextElementByLocal([&addr](CacheLineImpl& fd){
            return fd.addr == addr && fd.state != CacheState::INVALID;
        });
        if(cachePtr != nullptr){
            cachePtr->state = CacheState::MODIFIED;
            cachePtr->data = newData;
        }
        if(cachePtr == nullptr){
            cachePtr = m_local.nextElement([](CacheLineImpl& fd){
                return fd.state == CacheState::INVALID && !fd.isLocked();
            });
        }
        if(cachePtr){
//            if (line->state == CacheState::SHARED) {
//                // 如果是共享状态，需要升级到修改状态
//                std::cout << "Core " << coreId << ": WRITE to shared line, "
//                          << "invalidating other caches, state M->S->M" << std::endl;
//            } else if (line->state == CacheState::EXCLUSIVE) {
//                std::cout << "Core " << coreId << ": WRITE to exclusive line, "
//                          << "state E->M" << std::endl;
//            }
            if(cachePtr->state != CacheState::INVALID){
                while (cachePtr->isLocked()) {
                }
                cachePtr->markLocked();
                cachePtr->state = CacheState::MODIFIED;
                cachePtr->data = newData;
                cachePtr->markUnLocked();
            }else{
                while (cachePtr->isLocked()) {
                }
                if(cachePtr->state != CacheState::INVALID){
                    return nullptr;
                }
                cachePtr->markLocked();
                cachePtr->state = CacheState::MODIFIED;
                cachePtr->data = newData;
                cachePtr->addr = addr;
                cachePtr->markUnLocked();
            }
            return cachePtr;
        }
        return nullptr;
    }

    void handleBusRequest(const Addr& addr, bool write0){
        auto line = m_local.getElementByLocal([&addr](CacheLineImpl& fd){
            return fd.addr == addr;
        });
        if(!line){
            return;
        }
        if(write0){
            if (line->state == CacheState::MODIFIED ||
                line->state == CacheState::EXCLUSIVE ||
                line->state == CacheState::SHARED) {
                line->state = CacheState::INVALID;
            }
        }else{
            if (line->state == CacheState::MODIFIED) {
                //std::cout << " -> providing data, state M->S" << std::endl;
                line->state = CacheState::SHARED;
            } else if (line->state == CacheState::EXCLUSIVE) {
                //std::cout << " -> providing data, state E->S" << std::endl;
                line->state = CacheState::SHARED;
            } else if (line->state == CacheState::SHARED) {
                //std::cout << " -> already shared" << std::endl;
            }
        }
    }

private:
    LocalImpl m_local;
    GlobalImpl* m_global {nullptr};
};



}
