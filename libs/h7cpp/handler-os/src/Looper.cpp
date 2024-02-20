#include "handler-os/Looper.h"
#include "handler-os/Message.h"
#include "handler-os/MessageQueue.h"
#include "_common_pri.h"
#include "handler-os/Trace.h"
#include "handler-os/Handler.h"

#ifdef BUILD_WITH_QT
#include "handler-os/QTApplication.h"
#include "handler-os/qt_pri.h"
#endif

#define synchronized(a) std::unique_lock<std::mutex> lck(sOBJ.mutex_local);
#define synchronized_main(a) std::unique_lock<std::mutex> lck(sOBJ.mutex_main);

namespace h7_handler_os{

//c: __thread is like 'thread_local'
thread_local std::shared_ptr<Looper> sThreadLocal;

struct StaticObj0{
    std::mutex mutex_local;
    std::mutex mutex_main;
    std::shared_ptr<Looper> mainLooper;
};
static StaticObj0 sOBJ;

struct RAIIMsg{
    Message* msg;
    RAIIMsg(Message* m):msg(m){
    }
    ~RAIIMsg(){
        msg->recycleUnchecked();
    }
};

std::shared_ptr<Trace> Looper::sTrace;
Looper::Looper(bool quitAllowed){
    mQueue = new MessageQueue(quitAllowed);
    mTid = std::this_thread::get_id();
}
#ifdef BUILD_WITH_QT
Looper::Looper(_QTApplication_ctx* ctx){
    mQueue = new MessageQueue(ctx);
    mTid = std::this_thread::get_id();
}
#endif
Looper::~Looper(){
   if(mQueue){
       delete mQueue;
       mQueue = nullptr;
   }
}

void Looper::prepare(bool quitAllowed){
    synchronized(this){
        if(sThreadLocal){
            _LOG_ERR("Looper::prepare: already prepared.");
            return;
        }
#ifdef BUILD_WITH_QT
        auto app = QTApplication::get();
        if(app != nullptr && app->getAppCtx()->isCurrentThread()){
            sThreadLocal = std::shared_ptr<Looper>(new Looper(app->getAppCtx()));
            return;
        }
#endif
        sThreadLocal = std::shared_ptr<Looper>(new Looper(quitAllowed));
    }
}
void Looper::prepareMainLooper(){
#ifdef BUILD_WITH_QT
    _HANDLER_ASSERT(QTApplication::get() != nullptr);
#endif
    prepare(false);
    synchronized_main(this){
        if(sOBJ.mainLooper != nullptr){
            _LOG_ERR("prepareMainLooper: already had.");
            return;
        }
        sOBJ.mainLooper = myLooper();
    }
}
std::shared_ptr<Looper> Looper::myLooper(){
    synchronized(this){
        return sThreadLocal;
    }
}
std::shared_ptr<Looper> Looper::getMainLooper(){
    synchronized_main(this){
        return sOBJ.mainLooper;
    }
}
void Looper::loop(){
    auto me = myLooper();
    if(me == nullptr){
        _LOG_ERR("must call Looper::prepare()/prepareMainLooper().");
        return;
    }
#ifdef BUILD_WITH_QT
    auto app = QTApplication::get();
    if(app && me->mTid == app->getAppCtx()->mTid){
        //qt already loop internal
        return;
    }
#endif
    auto queue = me->mQueue;
    for(;;){
        Message* msg = queue->next();//may block
        if(msg == nullptr){
            // No message indicates that the message queue is quitting.
            return ;
        }
        RAIIMsg raMsg(msg);
        if(me->mDebug){
            char buf[256];
            if(msg->callback.cb){
                snprintf(buf, 256, ">>>>> Dispatching to %p %p(%s): %d",
                         msg->target, msg->callback.cb.get(),
                         msg->callback.name.data(), msg->what);
            }else{
                snprintf(buf, 256, ">>>>> Dispatching to %p: %d",
                         msg->target, msg->what);
            }
            String _m(buf);
            _LOG_INFO(_m);
        }

        auto traceTag = me->mTraceTag;
        const bool enable = sTrace && traceTag != 0
                && sTrace->isTagEnabled(traceTag);
        if(enable){
            sTrace->traceBegin(traceTag, msg->getTraceName());
        }
        msg->target->dispatchMessage(msg);
        if(enable){
            sTrace->traceEnd(traceTag);
        }

        if(me->mDebug){
            char buf[256];
            if(msg->callback.cb){
                snprintf(buf, 256, ">>>>> Finished to %p %p(%s): %d",
                         msg->target, msg->callback.cb.get(),
                         msg->callback.name.data(), msg->what);
            }else{
                snprintf(buf, 256, ">>>>> Finished to %p: %d",
                         msg->target, msg->what);
            }
            String _m(buf);
            _LOG_INFO(_m);
        }
    }
}

void Looper::quit(){
    mQueue->quit(false);
}
void Looper::quitSafely(){
    mQueue->quit(true);
}

void Looper::dump(std::stringstream& ss, CString prefix){
    char buf[256];
    snprintf(buf, 256, " Looper(%p): tid = ", this);
    ss << prefix << String(buf) << mTid << _NEW_LINE;
    mQueue->dump(ss, prefix);
}

//-------------------------------------------

#ifdef BUILD_WITH_QT
void handler_os_delete_msg(h7_handler_os::Message* msg){
    RAIIMsg rm(msg);
}
void handler_os_dispatch_msg(Message* m){
    m->getTarget()->dispatchMessage(m);
}
void handler_os_prepare_qtLooper(){
    Looper::prepareMainLooper();
}
#endif

}


