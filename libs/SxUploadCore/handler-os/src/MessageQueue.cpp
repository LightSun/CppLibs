#include "handler-os/MessageQueue.h"
#include "handler-os/Message.h"
#include "_common_pri.h"
#include "_time.h"
#include "handler-os/Object.h"

#define synchronized(a) std::unique_lock<std::mutex> lck(mMutex);

namespace h7_handler_os{

using String = Message::String;

bool MessageQueue::isIdle(){
    std::unique_lock<std::mutex> lck(mMutex);
    auto now = getCurTime();
    return mMessages == nullptr || now < mMessages->when;
}

Message* MessageQueue::next(){
    int pendingIdleHandlerCount = -1; // -1 only during first iteration
    long long nextPollTimeoutMillis = 0; //TODO current not used.
    for(;;){
//        if (nextPollTimeoutMillis != 0) {
//            Binder.flushPendingCommands();
//        }
//        nativePollOnce(ptr, nextPollTimeoutMillis);
        synchronized(this) {
            auto now = getCurTime();
            MsgPtr prevMsg = nullptr;
            MsgPtr msg = mMessages;
            if(msg != nullptr && msg->target == nullptr){
                // Stalled by a barrier. Find the next asynchronous message
                // in the queue.
                do {
                    prevMsg = msg;
                    msg = msg->next;
                } while (msg != nullptr && !msg->isAsynchronous());
            }
            if(msg != nullptr){
                if(now < msg->getWhen()){
                    // Next message is not ready. Set a timeout to wake up
                                            // when it is ready.
                    nextPollTimeoutMillis = msg->getWhen() - now;
                }else{
                    mBlocked = false;
                    if(prevMsg != nullptr){
                        prevMsg->next = msg->next;
                    }else{
                        mMessages = msg->next;
                    }
                    msg->next = nullptr;
                    _LOG_DEBUG2("MessageQueue::next >> ", msg->toString());
                    msg->markInUse();
                    return msg;
                }
            }else{
                nextPollTimeoutMillis = -1;
            }
            // Process the quit message now that all pending messages have
            // been handled.
            if (mQuitting) {
                //dispose();
                return nullptr;
            }
            // If first time idle, then get the number of idlers to run.
            // Idle handles only run if the queue is empty or if the first
            // message
            // in the queue (possibly a barrier) is due to be handled in the
            // future.
            if (pendingIdleHandlerCount < 0 &&
                    (mMessages == nullptr || now < mMessages->getWhen())) {
                pendingIdleHandlerCount = mIdleHandlers.size();
            }
            if (pendingIdleHandlerCount <= 0) {
                // No idle handlers to run. Loop and wait some more.
                mBlocked = true;
                continue;
            }
            mPendingIdleHandlers.clear();
            mPendingIdleHandlers.insert(mPendingIdleHandlers.end(),
                                        mIdleHandlers.begin(),mIdleHandlers.end());
        }

        for(int i = 0 ; i < pendingIdleHandlerCount ; i ++){
            auto& idler = mPendingIdleHandlers[i];
            if(idler){
                if(!idler->queueIdle()){
                    removeIdleHandler(idler);
                }
            }
        }
        // Reset the idle handler count to 0 so we do not run them again.
        pendingIdleHandlerCount = 0;

        // While calling an idle handler, a new message could have been
        // delivered
        // so go back and look again for a pending message without waiting.
        nextPollTimeoutMillis = 0;
    }
}

void MessageQueue::removeMessages(Handler* h, int what,
                                  Object* object, bool allowEquals){
    if(h == nullptr){
        return;
    }
    synchronized(this){
        MsgPtr p = mMessages;
        while (p != nullptr && p->target == h && p->what == what
               && (object == nullptr ||
                   p->obj.equals(object, allowEquals)
                            )) {
                MsgPtr n = p->next;
                mMessages = n;
                p->recycleUnchecked();
                p = n;
        }
        // Remove all messages after front.
        while (p != nullptr) {
            MsgPtr n = p->next;
            if (n != nullptr) {
                if (n->target == h && n->what == what
                        && (object == nullptr ||
                            n->obj.equals(object, allowEquals
                                          ))) {
                    MsgPtr nn = n->next;
                    n->recycleUnchecked();
                    p->next = nn;
                    continue;
                }
            }
            p = n;
        }
    }
}

void MessageQueue::removeMessages(Handler* h, Message* target,
                    std::function<int(MsgPtr, MsgPtr)> comparator){
    if (h == nullptr) {
        return;
    }
    synchronized(this){
        MsgPtr p = mMessages;

        // Remove all messages at front.
        while (p != nullptr && p->target == h
               && comparator(p, target) == 0) {
            MsgPtr n = p->next;
            mMessages = n;
            p->recycleUnchecked();
            p = n;
        }

        // Remove all messages after front.
        while (p != nullptr) {
            MsgPtr n = p->next;
            if (n != nullptr) {
                if (n->target == h && comparator(p, target) == 0) {
                    MsgPtr nn = n->next;
                    n->recycleUnchecked();
                    p->next = nn;
                    continue;
                }
            }
            p = n;
        }
    }
}

void MessageQueue::removeMessages(Handler* h, Func_Callback r,
                                  Object* object){
    if (h == nullptr || !r) {
        return;
    }

    synchronized(this){
        MsgPtr p = mMessages;

        // Remove all messages at front.
        while (p != nullptr && p->target == h
               && p->callback.cb == r &&
               (object == nullptr || p->obj == *object)) {
            MsgPtr n = p->next;
            mMessages = n;
            p->recycleUnchecked();
            p = n;
        }

        // Remove all messages after front.
        while (p != nullptr) {
            MsgPtr n = p->next;
            if (n != nullptr) {
                if (n->target == h && p->callback.cb == r
                        && (object == nullptr || n->obj == *object)) {
                    MsgPtr nn = n->next;
                    n->recycleUnchecked();
                    p->next = nn;
                    continue;
                }
            }
            p = n;
        }
    }
}

void MessageQueue::removeCallbacksAndMessages(Handler* h, Object* object){
    if (h == nullptr) {
        return;
    }

    synchronized(this){
        MsgPtr p = mMessages;

        // Remove all messages at front.
        while (p != nullptr && p->target == h
               && (object == nullptr || p->obj == *object)) {
            MsgPtr n = p->next;
            mMessages = n;
            p->recycleUnchecked();
            p = n;
        }

        // Remove all messages after front.
        while (p != nullptr) {
            MsgPtr n = p->next;
            if (n != nullptr) {
                if (n->target == h && (object == nullptr || n->obj == *object)) {
                    MsgPtr nn = n->next;
                    n->recycleUnchecked();
                    p->next = nn;
                    continue;
                }
            }
            p = n;
        }
    }
}

int MessageQueue::postSyncBarrier(long long when){
    // Enqueue a new sync barrier token.
    // We don't need to wake the queue because the purpose of a barrier is
    // to stall it.
    synchronized (this) {
        int token = mNextBarrierToken++;
        MsgPtr msg = Message::obtain();
        msg->markInUse();
        msg->when = when;
        msg->arg1 = token;

        MsgPtr prev = nullptr;
        MsgPtr p = mMessages;
        if (when != 0) { //!= 0
            while (p != nullptr && p->when <= when) {
                prev = p;
                p = p->next;
            }
        }
        if (prev != nullptr) { // invariant: p == prev.next
            msg->next = p;
            prev->next = msg;
        } else {
            msg->next = p;
            mMessages = msg;
        }
        return token;
    }
}
int MessageQueue::postSyncBarrier(){
    return postSyncBarrier(0);
}
bool MessageQueue::removeSyncBarrier(int token){
    // Remove a sync barrier token from the queue.
    // If the queue is no longer stalled by a barrier then wake it.
    synchronized (this) {
        MsgPtr prev = nullptr; //the parent of p
        MsgPtr p = mMessages;
        while (p != nullptr && (p->target != nullptr || p->arg1 != token)) {
            prev = p;
            p = p->next;
        }
        if (p == nullptr) {
            _LOG_ERR("The specified message queue synchronization "
                    " barrier token has not been posted or has already been removed.");
            return false;
        }
        bool needWake;
        if (prev != nullptr) {
            prev->next = p->next;
            needWake = false;
        } else {
            mMessages = p->next;
            needWake = mMessages == nullptr || mMessages->target != nullptr;
        }
        p->recycleUnchecked();

        // If the loop is quitting then it is already awake.
        // We can assume mPtr != 0 when mQuitting is false.
        /*
         * if (needWake && !mQuitting) { nativeWake(mPtr); }
         */
    }
    return true;
}

void MessageQueue::quit(bool safe){
    if (!mQuitAllowed) {
        _LOG_ERR("Main thread not allowed to quit.");
        return;
    }

    synchronized (this) {
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
void MessageQueue::removeAllMessagesLocked(){
    MsgPtr p = mMessages;
    for (; p != nullptr;) {
        MsgPtr n = p->next;
        p->recycleUnchecked();
        p = n;
    }
    mMessages = nullptr;
}
void MessageQueue::removeAllFutureMessagesLocked(){
    auto now = getCurTime();
    MsgPtr p = mMessages;
    if (p != nullptr) {
        if (p->getWhen() > now) {
            removeAllMessagesLocked();
        } else {
            MsgPtr n;
            for (;;) {
                n = p->next;
                if (n == nullptr) {
                    return;
                }
                if (n->getWhen() > now) {
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

bool MessageQueue::enqueueMessage(Message* msg, long long when){
    if (msg->target == nullptr) {
        _LOG_ERR("Message must have a target.");
        msg->recycleUnchecked();
        return false;
    }
    if (msg->isInUse()) {
        auto _m = msg->toString() + " This message is already in use.";
        _LOG_ERR(_m);
        return false;
    }
    synchronized (this) {
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
        MsgPtr p = mMessages;
        bool needWake;
        if (p == nullptr || when == 0 || when < p->when) {
            // New head, wake up the event queue if blocked.
            msg->next = p;
            mMessages = msg;
            needWake = mBlocked;
        } else {
            // Inserted within the middle of the queue. Usually we don't
            // have to wake
            // up the event queue unless there is a barrier at the head of
            // the queue
            // and the message is the earliest asynchronous message in the
            // queue.
            needWake = mBlocked && p->target == nullptr
                    && msg->isAsynchronous();
            MsgPtr prev;
            for (;;) {
                prev = p;
                p = p->next;
                if (p == nullptr || when < p->when) {
                    break;
                }
                if (needWake && p->isAsynchronous()) {
                    needWake = false;
                }
            }
            msg->next = p; // invariant: p == prev.next
            prev->next = msg;
        }
        // We can assume mPtr != 0 because mQuitting is false.
        /*if (needWake) {
            nativeWake(mPtr);
        }*/
    }
    return true;
}

bool MessageQueue::hasMessages(Handler* h, int what, Object* object){
    if (h == nullptr) {
        return false;
    }

    synchronized (this) {
        MsgPtr p = mMessages;
        while (p != nullptr) {
            if (p->target == h && p->what == what
                    && (object == nullptr || p->obj == *object)) {
                return true;
            }
            p = p->next;
        }
        return false;
    }
}
bool MessageQueue::hasMessages(Handler* h, Func_Callback r, Object* object){
    if (h == nullptr) {
        return false;
    }

    synchronized (this) {
        MsgPtr p = mMessages;
        while (p != nullptr) {
            if (p->target == h && p->callback.cb == r
                    && (object == nullptr || p->obj == *object)) {
                return true;
            }
            p = p->next;
        }
        return false;
    }
}
void MessageQueue::dump(std::stringstream& ss, CString prefix){
    synchronized (this) {
        int n= 0 ;
        for(MsgPtr msg = mMessages ; msg != nullptr ; msg = msg->next){
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
