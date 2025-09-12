#ifdef __linux__
#include <sched.h>
#endif

#include "handler-os/HandlerThread.h"
#include "handler-os/Looper.h"
#include <thread>
#include "_common_pri.h"

#define synchronized(a) std::unique_lock<std::mutex> lck(mMutex);

namespace h7_handler_os{

static inline void setCpuFriend(int id){
#ifdef __linux__
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(id, &cpuset);
    int result = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    if(result != 0){
        _LOG_DEBUG("setCpuFriend >> failed");
    }
#else
#ifdef _WIN32
    _LOG_DEBUG("setCpuFriend >> windows latter support.");
#endif
#endif
}

void HandlerThread::run(){
    std::thread thd([this](){
        run_main();
    });
    thd.join();
}
void HandlerThread::start(){
    std::thread thd([this](){
        run_main();
    });
    thd.detach();
}

void HandlerThread::run_main(){
    if(mFriendCpuId >= 0){
        setCpuFriend(mFriendCpuId);
    }
    String _m = "HandlerThread >> start new thread(" + mName
            + "). tid = " + _cur_tid_tostring();
    _LOG_DEBUG(_m);

    Looper::prepare();
    {
        synchronized(this){
            mLooper = Looper::myLooper();
            mConv.notify_all();
        }
    }
    //Todo set priority
    //
    if(mOnPrepared){
        (*mOnPrepared)();
        //mOnPrepared->reset();
    }
    Looper::loop();
    mDestroied = true;
    {
        synchronized(this){
            mLooper = nullptr;
        }
    }
    if(mDeleteAfterQuit){
        _LOG_DEBUG("delete HandlerThread.");
        delete this;
    }else if(mAfterQuit){
        //sometimes we want delete HandlerThread in this callback.
        //if need 'ptr->reset()', it need async.
//        auto ptr = mAfterQuit;
//        std::thread thd([ptr](){
//            (*ptr)();
//            ptr->reset();
//        });
//        thd.detach();
        (*mAfterQuit)();
    }
    //_LOG_DEBUG("--- done ---");
}

std::shared_ptr<Looper> HandlerThread::getLooper(){
    if(mDestroied){
        return nullptr;
    }
    synchronized(this){
        while (!mLooper) {
            mConv.wait(lck);
        }
    }
    return mLooper;
}
bool HandlerThread::quit(){
    auto looper = getLooper();
    if (looper) {
        looper->quit();
        return true;
    }
    return false;
}
bool HandlerThread::quitSafely(){
    auto looper = getLooper();
    if (looper) {
        looper->quitSafely();
        return true;
    }
    return false;
}

}
