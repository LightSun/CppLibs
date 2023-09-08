#include <map>
#include <QApplication>
#include <QTimer>
#include "handler-os/qt_pri.h"
#include "handler-os/MessageQueue.h"
#include "handler-os/Message.h"
#include "handler-os/Object.h"
#include "_time.h"

#include "_common_pri.h"
#include "Locker.h"

#ifdef BUILD_WITH_QT

#define G_CUR_TIME() h7_handler_os::getCurTime()
//#define synchronized(tag) \
//    std::unique_lock<std::mutex> lck(mtx);

#define synchronized(tag) \
    Locker lck(tag, &mtx);

#define _PRINTF(fmt,...) printf(fmt, ##__VA_ARGS__)
#define HE_MAX_POOL_SIZE 10

namespace h7_handler_os {

class StaticObj{
public:
    std::mutex mutex;
    HEvent* sPool {nullptr};
    unsigned int sPoolSize {0};

    HEvent* obtain(){
        std::unique_lock<std::mutex> lck(mutex);
        if(sPool != nullptr){
            HEvent* m = sPool;
            sPool = m->next;
            m->next = nullptr;
            sPoolSize --;
            return m;
        }
        return new HEvent();
    }
    HEvent* obtain(QEvent* ev){
        HEvent* he = obtain();
        he->event = ev;
        return he;
    }

    void recycle(HEvent* cur){
        std::unique_lock<std::mutex> lck(mutex);
        if(sPoolSize < HE_MAX_POOL_SIZE){
            cur->recyleMsg();
            cur->next = sPool;
            sPool = cur;
            sPoolSize ++;
        }else{
            delete cur;
        }
    }

    static StaticObj* get(){
        static StaticObj sOBJ;
        return &sOBJ;
    }
};
//---------------------

void HEvent::recyleMsg(){
    if(event){
        __Event* le = (__Event*)event;
        if(le->msg->target == nullptr){
            //barrier
            delete event;
        }
        event = nullptr;
    }
}
Message* HEvent::msgPtr(){
    if(event){
        __Event* le = (__Event*)event;
        return le->msg;
    }
    return nullptr;
}
void HEvent::recycleUnchecked(){
    StaticObj::get()->recycle(this);
}

//------------------------------------------
bool _QTApplication_ctx::hasEventThenRemove(QEvent* event){
    synchronized("hasEventThenRemove"){
        __Event* le = (__Event*)event;
        auto p = mMessages;
        while (p != nullptr) {
            if(p->msgPtr() == le->msg){
                auto n = p->next;
                mMessages = n;
                p->recycleUnchecked();
                p = n;
                return true;
            }else{
                p = p->next;
            }
        }
    }
    return false;
}
void _QTApplication_ctx::addEvent(QObject* receiver, QEvent* event){
    __Event* le = (__Event*)event;
    le->rec = receiver;
    {
    synchronized("addEvent"){
        auto msg = StaticObj::get()->obtain(event);
        auto when = le->msg->when;

        auto p = mMessages;
        if (p == nullptr || when == 0 || when < p->msgPtr()->when) {
           // New head, wake up the event queue if blocked.
           msg->next = p;
           mMessages = msg;
        } else {
           // Inserted within the middle of the queue.
           HEPtr prev;
           for (;;) {
               prev = p;
               p = p->next;
               if (p == nullptr || when < p->msgPtr()->when) {
                   break;
               }
           }
           msg->next = p; // invariant: p == prev.next
           prev->next = msg;
        }

    }
    }
    //barrier no need send to qt.
    if(receiver == nullptr){
        return ;
    }
    //printf("le.time = %lld\n", le->time);
    int delay = le->msg->when - G_CUR_TIME();
    if(delay > 0){
        Message* msg = le->msg;
        //printf("addEvent >> delay_time = %d, when = %lld\n", delay, le->msg->when);
        QTimer::singleShot(delay, receiver, [this, msg]() {
            //has msg means not removed.
            auto _le = findEvent(msg);
            if(_le){
                QApplication::postEvent((QObject*)_le->rec, _le);
            }
        });
    }else{
        //printf("addEvent >> when = %lld\n", le->msg->when);
        QApplication::postEvent(receiver, event);
    }
}

bool _QTApplication_ctx::isIdle(){
    synchronized("isIdle"){
        auto now = getCurTime();
        return mMessages == nullptr || now < mMessages->msgPtr()->when - mIdleThreshold;
    }
}

void _QTApplication_ctx::removeMessages(Handler* h, int what,
                    Object* object, bool allowEquals){
    synchronized("removeMessages_what"){
        auto p = mMessages;
        while (p != nullptr && p->msgPtr()->target == h && p->msgPtr()->what == what
               && (object == nullptr ||
                   p->msgPtr()->obj.equals(object, allowEquals)
                            )) {
                auto n = p->next;
                mMessages = n;
                p->recycleUnchecked();
                p = n;
        }
        // Remove all messages after front.
        while (p != nullptr) {
            auto n = p->next;
            if (n != nullptr) {
                if (n->msgPtr()->target == h && n->msgPtr()->what == what
                        && (object == nullptr ||
                            n->msgPtr()->obj.equals(object, allowEquals
                                          ))) {
                    auto nn = n->next;
                    n->recycleUnchecked();
                    p->next = nn;
                    continue;
                }
            }
            p = n;
        }
    }
}

void _QTApplication_ctx::removeMessages(Handler* h, Message* target,
                    std::function<int(MsgPtr, MsgPtr)> comparator){
    synchronized("removeMessages_cmp"){
        auto p = mMessages;

        // Remove all messages at front.
        while (p != nullptr && p->msgPtr()->target == h
               && comparator(p->msgPtr(), target) == 0) {
            auto n = p->next;
            mMessages = n;
            p->recycleUnchecked();
            p = n;
        }

        // Remove all messages after front.
        while (p != nullptr) {
            auto n = p->next;
            if (n != nullptr) {
                if (n->msgPtr()->target == h && comparator(p->msgPtr(), target) == 0) {
                    auto nn = n->next;
                    n->recycleUnchecked();
                    p->next = nn;
                    continue;
                }
            }
            p = n;
        }
    }
}

void _QTApplication_ctx::removeMessages(Handler* h, Func_Callback r,
                    Object* object){
    synchronized("removeMessages_func_CB")
    {
        auto p = mMessages;

        // Remove all messages at front.
        while (p != nullptr && p->msgPtr()->target == h
               && p->msgPtr()->callback.cb == r &&
               (object == nullptr || p->msgPtr()->obj == *object)) {
            auto n = p->next;
            mMessages = n;
            p->recycleUnchecked();
            p = n;
        }

        // Remove all messages after front.
        while (p != nullptr) {
            auto n = p->next;
            if (n != nullptr) {
                if (n->msgPtr()->target == h && p->msgPtr()->callback.cb == r
                        && (object == nullptr || n->msgPtr()->obj == *object)) {
                    auto nn = n->next;
                    n->recycleUnchecked();
                    p->next = nn;
                    continue;
                }
            }
            p = n;
        }
    }
}

void _QTApplication_ctx::removeCallbacksAndMessages(Handler* h, Object* object){
    synchronized("removeCallbacksAndMessages"){
        auto p = mMessages;

        // Remove all messages at front.
        while (p != nullptr && p->msgPtr()->target == h
               && (object == nullptr || p->msgPtr()->obj == *object)) {
            auto n = p->next;
            mMessages = n;
            p->recycleUnchecked();
            p = n;
        }

        // Remove all messages after front.
        while (p != nullptr) {
            auto n = p->next;
            if (n != nullptr) {
                if (n->msgPtr()->target == h && (object == nullptr || n->msgPtr()->obj == *object)) {
                    auto nn = n->next;
                    n->recycleUnchecked();
                    p->next = nn;
                    continue;
                }
            }
            p = n;
        }
    }
}
bool _QTApplication_ctx::hasMessages(Handler* h, int what, Object* object){
    synchronized("hasMessages_what"){
        auto p = mMessages;
        while (p != nullptr) {
           if (p->msgPtr()->target == h && p->msgPtr()->what == what
                   && (object == nullptr || p->msgPtr()->obj == *object)) {
               return true;
           }
           p = p->next;
        }
        return false;
    }
}
bool _QTApplication_ctx::hasMessages(Handler* h, Func_Callback r, Object* object){
    synchronized("hasMessages_func_CB"){
        auto p = mMessages;
        while (p != nullptr) {
            if (p->msgPtr()->target == h && p->msgPtr()->callback.cb == r
                    && (object == nullptr || p->msgPtr()->obj == *object)) {
                return true;
            }
            p = p->next;
        }
        return false;
    }
}
bool _QTApplication_ctx::enqueueMessage(Message* msg, long long when){
    {
    synchronized ("enqueueMessage")
    {
        if (mQuitting) {
            char buf[128];
            snprintf(buf, 128, "%p sending message to a Handler on a dead thread",
                     msg->target);
            String str(buf);
            _LOG_INFO(str);
            msg->recycle();
            return false;
        }else{
            msg->markInUse();
            msg->when = when;
            //
        }
    }
    }
    handler_qt_post_msg(msg);
    return true;
}

int _QTApplication_ctx::postSyncBarrier(long long when){
    MsgPtr msg = Message::obtain();
    {
    synchronized("postSyncBarrier"){
        int token = mNextBarrierToken++;
        msg->markInUse();
        msg->when = when;
        msg->arg1 = token;
    }
    }
    addEvent(nullptr, new __Event(msg));
    return msg->arg1;
}

bool _QTApplication_ctx::removeSyncBarrier(int token){
    synchronized("removeSyncBarrier"){
        HEPtr prev = nullptr; //the parent of p
        auto p = mMessages;
        while (p != nullptr && (p->msgPtr()->target != nullptr || p->msgPtr()->arg1 != token)) {
            prev = p;
            p = p->next;
        }
        if (p == nullptr) {
            _LOG_ERR("removeSyncBarrier >> The specified message queue synchronization "
                    " barrier token has not been posted or has already been removed.");
            return false;
        }
        if (prev != nullptr) {
            prev->next = p->next;
        } else {
            mMessages = p->next;
        }
        p->recycleUnchecked();
    }
    return true;
}

void _QTApplication_ctx::quit(bool safe){
    synchronized ("quit") {
        if (mQuitting) {
            return;
        }
        mQuitting = true;

        if (safe) {
            removeAllFutureMessagesLocked();
        } else {
            removeAllMessagesLocked();
        }
    }
}
void  _QTApplication_ctx::removeAllMessagesLocked(){
    auto p = mMessages;
    for (; p != nullptr;) {
       auto n = p->next;
       p->recycleUnchecked();
       p = n;
    }
    mMessages = nullptr;
}
void  _QTApplication_ctx::removeAllFutureMessagesLocked(){
    auto now = getCurTime();
    auto p = mMessages;
    if (p != nullptr) {
       if (p->msgPtr()->getWhen() > now) {
           removeAllMessagesLocked();
       } else {
           HEPtr n;
           for (;;) {
               n = p->next;
               if (n == nullptr) {
                   return;
               }
               if (n->msgPtr()->getWhen() > now) {
                   break;
               }
               p = n;
           }
           p->next = nullptr;
           do {
               p = n;
               n = p->next;
               p->recycleUnchecked();
           } while (n != nullptr);
       }
    }
}
__Event* _QTApplication_ctx::findEvent(MsgPtr ptr){
    auto p = mMessages;
    while (p != nullptr) {
        if(p->msgPtr() == ptr){
            return (__Event*)p->event;
        }
        p = p->next;
    }
    return nullptr;
}

void _QTApplication_ctx::dump(std::stringstream& ss, CString prefix){
    int n = 0 ;
    {
    synchronized ("dump") {
        for(auto msg = mMessages ; msg != nullptr ; msg = msg->next){
            ss << prefix << "Message " << n << ": " << msg->msgPtr()->toString()
               << _NEW_LINE;
            n ++;
        }
    }
    }
    ss << prefix << "(Total messages: " << n
       <<  ", polling=" << isPollingLocked()
        << ", quitting=" << mQuitting << ")" << _NEW_LINE;
}

}

#endif
