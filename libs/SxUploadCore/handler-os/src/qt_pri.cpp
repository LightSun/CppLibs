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

namespace h7_handler_os {

__Events::~__Events(){
    synchronized("EVS_~__Events"){
        for(int i = events.size() - 1; i >= 0 ; i--){
            __Event* le = (__Event*)(events[i]);
            MsgPtr p = le->msg;
            if (p != nullptr && p->target == nullptr){
                //delete barrier events.
                delete le;
            }
        }
        events.clear();
    }
}
int __Events::removeMessages(Handler* h, int what,
                   Object* object, bool allowEquals){
    synchronized("EVS_removeMessages_what"){
        for(int i = events.size() - 1; i >= 0 ; i--){
            __Event* le = (__Event*)(events[i]);
            MsgPtr p = le->msg;
            if (p != nullptr && p->target == h && p->what == what
                   && (object == nullptr ||
                       p->obj.equals(object, allowEquals)
                                )) {
                events.erase(events.begin() + i);
            }
        }
        return events.size();
    }
}
int __Events::removeMessages(Handler* h, Message* target,
                   std::function<int(MsgPtr, MsgPtr)> comparator){
    synchronized("EVS_removeMessages_cmp"){
        for(int i = events.size() - 1; i >= 0 ; i--){
            __Event* le = (__Event*)(events[i]);
            MsgPtr p = le->msg;
            if (p != nullptr && p->target == h
                    && comparator(p, target) == 0) {
                events.erase(events.begin() + i);
            }
        }
        return events.size();
    }
}
int __Events::removeMessages(Handler* h, Func_Callback r,
                   Object* object){
    synchronized("EVS_removeMessages_cb"){
        for(int i = events.size() - 1; i >= 0 ; i--){
            __Event* le = (__Event*)(events[i]);
            MsgPtr p = le->msg;
            if (p != nullptr && p->target == h
                    && p->callback.cb == r &&
                    (object == nullptr || p->obj == *object)
                    ){
                events.erase(events.begin() + i);
            }
        }
        return events.size();
    }
}
int __Events::removeCallbacksAndMessages(Handler* h, Object* object){
    synchronized("EVS_removeCallbacksAndMessages"){
        for(int i = events.size() - 1; i >= 0 ; i--){
            __Event* le = (__Event*)(events[i]);
            MsgPtr p = le->msg;
            if (p != nullptr && p->target == h
                    && (object == nullptr || p->obj == *object)
                    ){
                events.erase(events.begin() + i);
            }
        }
        return events.size();
    }
}
bool __Events::hasMessages(Handler* h, int what, Object* object){
    synchronized("EVS_hasMessages_what"){
        for(int i = events.size() - 1; i >= 0 ; i--){
            __Event* le = (__Event*)(events[i]);
            MsgPtr p = le->msg;
            if (p != nullptr && p->target == h && p->what == what
                    && (object == nullptr || p->obj == *object)) {
                return true;
            }
        }
    }
    return false;
}

bool __Events::hasMessages(Handler* h, Func_Callback r, Object* object){
    synchronized("EVS_hasMessages_cb"){
        for(int i = events.size() - 1; i >= 0 ; i--){
            __Event* le = (__Event*)(events[i]);
            MsgPtr p = le->msg;
            if (p != nullptr && p->target == h && p->callback.cb == r
                    && (object == nullptr || p->obj == *object)) {
                return true;
            }
        }
    }
    return false;
}
int __Events::removeSyncBarrier(int token){
    __Event* le_rm = nullptr;
    int ret;
    {
    synchronized("EVS_removeSyncBarrier"){
        for(int i = events.size() - 1; i >= 0 ; i--){
            __Event* le = (__Event*)(events[i]);
            MsgPtr p = le->msg;
            if (p != nullptr && p->target == nullptr
                    && p->arg1 == token){
                le_rm = le;
                removeAt(i);
                break;
            }
        }
        ret = events.size();
    }
    }
    //for barrier we need delete event.
    if(le_rm != nullptr){
        delete le_rm;
    }
    return ret;
}

void __Events::dump(std::stringstream& ss, CString prefix, int& n){
    synchronized("EVS_dump"){
        for( auto it = events.begin(); it != events.end(); ){
            __Event* le = (__Event*)(*it);
            MsgPtr msg = le->msg;
            ss << prefix << "Message " << n << ": " << msg->toString()
               << _NEW_LINE;
            n ++;
        }
    }
}

//------------------------------------------
bool _QTApplication_ctx::hasEventThenRemove(QEvent* event){
    synchronized("hasEventThenRemove"){
        __Event* le = (__Event*)event;
        auto it = events.find(le->msg->when);
        if(it != events.end()){
            //_PRINTF("QTApp >> hasEvent = true, when = %lld\n", le->msg->when);
            if(it->second->removeByMsg(le->msg) == 0){
                events.erase(it);
            }
            return true;
        }
    }
    return false;
}
void _QTApplication_ctx::addEvent(QObject* receiver, QEvent* event){
    __Event* le = (__Event*)event;
    le->rec = receiver;
    ShareEvents evs;
    {
    synchronized("addEvent")
        {
        auto it = events.find(le->msg->when);
        if(it != events.end()){
            evs = it->second;
        }else{
            evs = makeShareEvents();
            events.insert(std::make_pair<>(le->msg->when, evs));
        }
    }
    }
    evs->add(event);
    //barrier no need send to qt.
    if(receiver == nullptr){
        return;
    }
    //printf("le.time = %lld\n", le->time);
    int delay = le->msg->when - G_CUR_TIME();
    if(delay > 0){
        Message* msg = le->msg;
        //printf("addEvent >> delay_time = %d, when = %lld\n", delay, le->msg->when);
        QTimer::singleShot(delay, receiver, [this, msg]() {
            ShareEvents _evs;
            {
            synchronized("addEvent_delay"){
                auto it = events.find(msg->when);
                if(it != events.end()){
                    _evs = it->second;
                }
            }
            }
            if(_evs){
                auto ev = _evs->findEvent(msg);
                if(ev != nullptr){
                    QApplication::postEvent((QObject*)ev->rec, ev);
                }
            }
        });
    }else{
        //printf("addEvent >> when = %lld\n", le->msg->when);
        QApplication::postEvent(receiver, event);
    }
}

bool _QTApplication_ctx::isIdle(){
    synchronized("isIdle"){
        auto it = events.begin();
        if(it == events.end()){
            return true;
        }
        return G_CUR_TIME() < it->first - mIdleThreshold;
    }
}

void _QTApplication_ctx::removeMessages(Handler* h, int what,
                    Object* object, bool allowEquals){
    synchronized("removeMessages_what"){
        for( auto it = events.begin(); it != events.end(); ){
            if(it->second->removeMessages(h, what, object, allowEquals) == 0){
                events.erase(it++);
            }else{
                it++;
            }
        }
    }
}

void _QTApplication_ctx::removeMessages(Handler* h, Message* target,
                    std::function<int(MsgPtr, MsgPtr)> comparator){
    synchronized("removeMessages_cmp"){
        for( auto it = events.begin(); it != events.end(); ){
            if(it->second->removeMessages(h, target, comparator) == 0){
                events.erase(it++);
            }else{
                it++;
            }
        }
    }
}

void _QTApplication_ctx::removeMessages(Handler* h, Func_Callback r,
                    Object* object){
    synchronized("removeMessages_func_CB")
    {
        for( auto it = events.begin(); it != events.end(); ){
            if(it->second->removeMessages(h, r, object) == 0){
                events.erase(it++);
            }else{
                it++;
            }
        }
    }
}

void _QTApplication_ctx::removeCallbacksAndMessages(Handler* h, Object* object){
    synchronized("removeCallbacksAndMessages"){
        for( auto it = events.begin(); it != events.end(); ){
            if(it->second->removeCallbacksAndMessages(h, object) == 0){
                events.erase(it++);
            }else{
                it++;
            }
        }
    }
}
bool _QTApplication_ctx::hasMessages(Handler* h, int what, Object* object){
    synchronized("hasMessages_what"){
        for( auto it = events.begin(); it != events.end(); it++){
            if (it->second->hasMessages(h, what, object)) {
                return true;
            }
        }
    }
    return false;
}
bool _QTApplication_ctx::hasMessages(Handler* h, Func_Callback r, Object* object){
    synchronized("hasMessages_func_CB"){
        for( auto it = events.begin(); it != events.end(); it++){
            if (it->second->hasMessages(h, r, object)) {
                return true;
            }
        }
    }
    return false;
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
        for( auto it = events.begin(); it != events.end(); ){
            if (it->second->removeSyncBarrier(token) == 0 ){
                events.erase(it++);
                return true;
            }else{
                it++;
            }
        }
    }
    _LOG_ERR("The specified message queue synchronization "
            " barrier token has not been posted or has already been removed.");
    return false;
}

void _QTApplication_ctx::quit(bool safe){
    if(mQuitting){
        return;
    }
    mQuitting = true;
    if(safe){
        //remove future messages.
        _removeFutureMessages(G_CUR_TIME());
    }else{
        //may have barrier
        synchronized("clear_event"){
            events.clear();
        }
    }
}

void _QTApplication_ctx::_removeFutureMessages(long long now){
    synchronized("_removeBarrierEvent"){
        bool reached = false;
        for(auto it = events.begin(); it != events.end(); ){
            if(reached){
                events.erase(it++);
            }else if (it->first > now){
                events.erase(it++);
                reached = true;
            }else{
                it++;
            }
        }
    }
}

void _QTApplication_ctx::dump(std::stringstream& ss, CString prefix){
    int n = 0 ;
    {
    synchronized ("dump") {
        for( auto it = events.begin(); it != events.end(); ){
            it->second->dump(ss, prefix, n);
        }
    }
    }
    ss << prefix << "(Total messages: " << n
       <<  ", polling=" << isPollingLocked()
        << ", quitting=" << mQuitting << ")" << _NEW_LINE;
}

}

#endif
