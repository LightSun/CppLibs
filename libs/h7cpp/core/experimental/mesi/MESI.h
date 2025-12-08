#pragma once

#include <string>
#include "core/experimental/GMP.h"
#include "core/utils/h_atomic.h"
#include "core/utils/_time.h"

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
    volatile int locked {0};
    int state {(int)CacheState::INVALID};
    size_t useCnt {0};
    long long updateTime {0};
    Addr addr;
    Data data;

    bool isLocked(){
        return h_atomic_get(&locked) != 0;
    }
    void markLocked(){
        h_atomic_cas(&locked, 0, 1);
    }
    void markUnLocked(){
        h_atomic_cas(&locked, 1, 0);
    }
    void setData(const Data& d){
        this->data = d;
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
