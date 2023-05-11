#include "handler-os/Message.h"
#include <mutex>
#include <sstream>
#include "handler-os/Handler.h"
#include "handler-os/Object.h"
#include "_common_pri.h"
#include "_time.h"

#define MAX_POOL_SIZE 20
#define FLAGS_TO_CLEAR_ON_COPY_FROM kFlag_IN_USE

#define _DEBUG 1
#ifdef _DEBUG
static unsigned int _C_CREATE = 0;
static unsigned int _C_DELETE = 0;
#endif

//for friend class. must be the same namespace.
namespace h7_handler_os {

class StaticObj{
public:
    std::mutex mutex;
    Message* sPool {nullptr};
    unsigned int sPoolSize {0};

    using MsgPtr = Message*;

    Message* obtain(){
        std::unique_lock<std::mutex> lck(mutex);
        if(sPool != nullptr){
            MsgPtr m = sPool;
            sPool = m->next;
            m->next = nullptr;
            m->flags = 0; //clear
            sPoolSize --;
            return m;
        }
#ifdef _DEBUG
    _C_CREATE ++;
#endif
        return new Message();
    }
    void recycle(Message* cur){
        std::unique_lock<std::mutex> lck(mutex);
        if(sPoolSize < MAX_POOL_SIZE){
            cur->next = sPool;
            sPool = cur;
            sPoolSize ++;
        }else{
#ifdef _DEBUG
    _C_DELETE ++;
#endif
            delete cur;
        }
    }
};

static StaticObj sOBJ;
}

namespace h7_handler_os {
Message* Message::obtain(){
    return sOBJ.obtain();
}
Message* Message::obtain(Message* src){
    auto m = obtain();
    m->what = src->what;
    m->arg1 = src->arg1;
    m->arg2 = src->arg2;
    m->obj.copyFrom(&src->data);
    m->data.copyFrom(&src->data);
    m->target = src->target;
    m->callback = src->callback;
    return m;
}
Message* Message::obtain(Handler* h){
    auto m = obtain();
    m->target = h;
    return m;
}
Message* Message::obtain(Handler* h, Func_Callback cb, CString cb_name){
    auto m = obtain(h);
    m->target = h;
    m->callback.cb = cb;
    m->callback.name = cb_name;
    return m;
}
Message* Message::obtain(Handler* h, std::function<void()> cb, CString cb_name){
    auto ccb = std::make_shared<std::packaged_task<void()>>(std::bind(cb));
    auto m = obtain(h);
    m->target = h;
    m->callback.cb = ccb;
    m->callback.name = cb_name;
    return m;
}
Message* Message::obtain(Handler* h, int what){
    auto m = sOBJ.obtain();
    m->target = h;
    m->what = what;
    return m;
}
Message* Message::obtain(Handler* h, int what, Object* obj){
    auto m = sOBJ.obtain();
    m->target = h;
    m->what = what;
    m->obj.copyFrom(obj);
    return m;
}
Message* Message::obtain(Handler* h, int what, int arg1, Arg2Type arg2){
    auto m = sOBJ.obtain();
    m->target = h;
    m->what = what;
    m->arg1 = arg1;
    m->arg2 = arg2;
    return m;
}
Message* Message::obtain(Handler* h, int what, int arg1, Arg2Type arg2,
                         Object* obj){
    auto m = sOBJ.obtain();
    m->target = h;
    m->what = what;
    m->arg1 = arg1;
    m->arg2 = arg2;
    m->obj.copyFrom(obj);
    return m;
}

void Message::recycleUnchecked(){
    // Mark the message as in use while it remains in the recycled object pool.
    // Clear out all other details.
    flags = kFlag_IN_USE;
    what = 0;
    arg1 = 0;
    arg2 = 0;
    obj.release();
    // replyTo = null;
    // sendingUid = -1;
    when = 0;
    target = nullptr;
    data.release();
    callback.cb = nullptr;
    //_HANDLER_ASSERT(!callback.cb);
    //
    sOBJ.recycle(this);
}

Message::String Message::getTraceName(){
    char buf[256];
    if(callback.cb){
        snprintf(buf, 256, "%p: %p(%s) #%d", this, callback.cb.get(),
                 callback.name.data(), what);
    }else{
        snprintf(buf, 256, "%p: #%d", this, what);
    }
    return String(buf);
}

void Message::sendToTarget(){
    target->sendMessage(this);
}
void Message::copyFrom(Message* src){
    flags = src->flags & (~FLAGS_TO_CLEAR_ON_COPY_FROM);
    what = src->what;
    arg1 = src->arg1;
    arg2 = src->arg2;
    obj.copyFrom(&src->obj);
    data.copyFrom(&src->data);
}

Message::String Message::toString(){
    std::stringstream ss;
    ss << "{ when=" << (when > 0 ? formatTime(when) : "0");
    if(target != nullptr){
        if(callback.cb){
            ss << " callback=";
            ss << callback.cb.get();
            ss << "(" << callback.name << ")";
        }else{
            ss << " what=";
            ss << what;
        }

        if(arg1 != 0){
            ss << " arg1=" << arg1;
        }
        if(arg2 != 0){
            ss << " arg2=" << arg2;
        }
        if(obj.ptr != nullptr){
            ss << " obj=" << obj.tag << "-" << obj.ptr;
        }
        ss << " target= " << target;
    }else{
        ss << " barrier=" << arg1;
    }
    ss << " }";
    return ss.str();
}

void Message::logDebugInfo(){
#ifdef _DEBUG
    std::unique_lock<std::mutex> lck(sOBJ.mutex);
    char buf[1024];
    snprintf(buf, 1024, "Message >> size: (create, delete, pool) = (%d, %d, %d)",
             _C_CREATE, _C_DELETE, sOBJ.sPoolSize);
    _LOG_INFO(String(buf));
#else
    _LOG_INFO("logDebugInfo >> debug is not opened.");
#endif
}

}
