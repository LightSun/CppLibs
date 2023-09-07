#include <map>
#include "handler-os/qt_pri.h"
#include "handler-os/MessageQueue.h"
#include "handler-os/Message.h"
#include "handler-os/Object.h"
#include "_time.h"

#include "_common_pri.h"

#ifdef BUILD_WITH_QT

#define G_CUR_TIME() h7_handler_os::getCurTime()
#define synchronized() std::unique_lock<std::mutex> lck(mtx);

namespace h7_handler_os {

bool _QTApplication_ctx::hasEventThenRemove(QEvent* event){
    synchronized(){
        __Event* le = (__Event*)event;
        Item item = {le->msg, 0};
        auto it = events.find(item);
        if(it != events.end()){
            events.erase(it);
            return true;
        }
    }
    return false;
}
void _QTApplication_ctx::addEvent(QEvent* event){
    synchronized(){
        __Event* le = (__Event*)event;
        Item item = {le->msg, G_CUR_TIME()};
        //events[item] = le;
        events.insert(std::make_pair<>(std::move(item), le));
    }
}

bool _QTApplication_ctx::isIdle(){
    synchronized(){
        auto it = events.begin();
        if(it == events.end()){
            return true;
        }
        __Event* le = (__Event*)(it->second);
        return G_CUR_TIME() < le->msg->when;
    }
}

void _QTApplication_ctx::removeMessages(Handler* h, int what,
                    Object* object, bool allowEquals){
    synchronized(){
        for( auto it = events.begin(); it != events.end(); ){
            __Event* le = (__Event*)(it->second);
            MsgPtr p = le->msg;
            if (p != nullptr && p->target == h && p->what == what
                   && (object == nullptr ||
                       p->obj.equals(object, allowEquals)
                                )) {
                events.erase(it++);
            }else{
                it++;
            }
        }
    }
}

void _QTApplication_ctx::removeMessages(Handler* h, Message* target,
                    std::function<int(MsgPtr, MsgPtr)> comparator){
    synchronized(){
        for( auto it = events.begin(); it != events.end(); ){
            __Event* le = (__Event*)(it->second);
            MsgPtr p = le->msg;
            if (p != nullptr && p->target == h
                    && comparator(p, target) == 0) {
                events.erase(it++);
            }else{
                it++;
            }
        }
    }
}

void _QTApplication_ctx::removeMessages(Handler* h, Func_Callback r,
                    Object* object){
    synchronized(){
        for( auto it = events.begin(); it != events.end(); ){
            __Event* le = (__Event*)(it->second);
            MsgPtr p = le->msg;
            if (p != nullptr && p->target == h
                    && p->callback.cb == r &&
                    (object == nullptr || p->obj == *object)
                    ){
                events.erase(it++);
            }else{
                it++;
            }
        }
    }
}

void _QTApplication_ctx::removeCallbacksAndMessages(Handler* h, Object* object){
    synchronized(){
        for( auto it = events.begin(); it != events.end(); ){
            __Event* le = (__Event*)(it->second);
            MsgPtr p = le->msg;
            if (p != nullptr && p->target == h
                    && (object == nullptr || p->obj == *object)
                    ){
                events.erase(it++);
            }else{
                it++;
            }
        }
    }
}
bool _QTApplication_ctx::hasMessages(Handler* h, int what, Object* object){
    synchronized(){
        for( auto it = events.begin(); it != events.end(); it++){
            __Event* le = (__Event*)(it->second);
            MsgPtr p = le->msg;
            if (p != nullptr && p->target == h && p->what == what
                    && (object == nullptr || p->obj == *object)) {
                return true;
            }
        }
    }
    return false;
}
bool _QTApplication_ctx::hasMessages(Handler* h, Func_Callback r, Object* object){
    synchronized(){
        for( auto it = events.begin(); it != events.end(); it++){
            __Event* le = (__Event*)(it->second);
            MsgPtr p = le->msg;
            if (p != nullptr && p->target == h && p->callback.cb == r
                    && (object == nullptr || p->obj == *object)) {
                return true;
            }
        }
    }
    return false;
}
bool _QTApplication_ctx::enqueueMessage(Message* msg, long long when){
    synchronized () {
        if (mQuitting) {
            char buf[128];
            snprintf(buf, 128, "%p sending message to a Handler on a dead thread",
                     msg->target);
            String str(buf);
            _LOG_INFO(str);
            msg->recycle();
            return false;
        }
        msg->markInUse();
        msg->when = when;
        //
        auto delay = when - G_CUR_TIME();
        handler_qt_post_msg(msg, delay);
        return true;
    }
}

int _QTApplication_ctx::postSyncBarrier(long long when){
    synchronized(){
        int token = mNextBarrierToken++;
        MsgPtr msg = Message::obtain();
        msg->markInUse();
        msg->when = when;
        msg->arg1 = token;
        auto le = new __Event(msg);
        Item item = {le->msg, G_CUR_TIME()};
        //events[item] = le;
        events.insert(std::make_pair<>(std::move(item), le));

        return token;
    }
}

bool _QTApplication_ctx::removeSyncBarrier(int token){
    synchronized(){
        for( auto it = events.begin(); it != events.end(); ){
            __Event* le = (__Event*)(it->second);
            MsgPtr p = le->msg;
            if (p != nullptr && p->target == nullptr
                    && p->arg1 == token
                    ){
                events.erase(it++);
                //for barrier is post by internal, we need manual delete it.
                delete le;
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
        auto now = G_CUR_TIME();
        //remove future messages.
        synchronized(){
            for( auto it = events.begin(); it != events.end(); ){
                __Event* le = (__Event*)(it->second);
                MsgPtr p = le->msg;
                if (p != nullptr && p->getWhen() > now
                        ){
                    if(p->target == nullptr){
                        //barrier
                        delete le;
                    }
                    events.erase(it++);
                }else{
                    it++;
                }
            }
        }
    }else{
        //may have barrier
        synchronized(){
            for( auto it = events.begin(); it != events.end(); ){
                __Event* le = (__Event*)(it->second);
                MsgPtr p = le->msg;
                if (p != nullptr){
                    if(p->target == nullptr){
                        //barrier
                        delete le;
                    }
                    events.erase(it++);
                }else{
                    it++;
                }
            }
        }
    }
}

void _QTApplication_ctx::dump(std::stringstream& ss, CString prefix){
    synchronized () {
        int n= 0 ;
        for( auto it = events.begin(); it != events.end(); ){
            __Event* le = (__Event*)(it->second);
            MsgPtr msg = le->msg;
            ss << prefix << "Message " << n << ": " << msg->toString()
               << _NEW_LINE;
            n ++;
        }
        ss << prefix << "(Total messages: " << n
           <<  ", polling=" << isPollingLocked()
            << ", quitting=" << mQuitting << ")" << _NEW_LINE;
    }
}

}

#endif
