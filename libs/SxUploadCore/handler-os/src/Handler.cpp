#include <condition_variable>
#include "handler-os/Handler.h"
#include "handler-os/Message.h"
#include "handler-os/MessageQueue.h"
#include "handler-os/Looper.h"
#include "_common_pri.h"
#include "_time.h"

#define synchronized(a) std::unique_lock<std::mutex> lck(mMutex);

namespace h7_handler_os{

struct BlockingRunnable{
    Message::Callback mTask;
    std::atomic_bool mDone{false};
    std::condition_variable mConv;
    std::mutex mMutex;

    Message::Func_Callback mAcb;

    BlockingRunnable(Message::Func_Callback cb, Message::CString name){
        mTask.cb = cb;
        mTask.name = name;
        mAcb = std::make_shared<Message::PkgTask>([this](){
            run();
        });
    }

    void run(){
        (*mTask.cb)();
        mTask.cb->reset();
        synchronized(this){
            mDone = true;
            mConv.notify_all();
        }
    }

    bool postAndWait(Handler* h, long long timeout){
        if(!h->post(mAcb, mTask.name)){
            return false;
        }
        synchronized (this) {
            if (timeout > 0) {
                auto expirationTime = getCurTime() + timeout;
                while (!mDone) {
                    long delay = expirationTime - getCurTime();
                    if (delay <= 0) {
                        return false; // timeout
                    }
                    mConv.wait_for(lck, std::chrono::milliseconds(delay));
                }
            } else {
                while (!mDone) {
                    mConv.wait(lck);
                }
            }
        }
        return true;
    }
};

struct _RAII{
    BlockingRunnable* br;
    _RAII(BlockingRunnable* br):br(br){}
    ~_RAII(){
        if(br){
            delete br;
            br = nullptr;
        }
    }
};
//----------------------------------------

Handler::Handler(std::shared_ptr<HandlerCallback> cb, bool async){
    mLooper = Looper::myLooper();
    _HANDLER_ASSERT(mLooper != nullptr);
    mQueue = mLooper->mQueue;
    mCallback = cb;
    mAsynchronous = async;
}

Handler::Handler(std::shared_ptr<Looper> looper,
                 std::shared_ptr<HandlerCallback> cb,
                 bool async){
    mLooper = looper;
    _HANDLER_ASSERT(mLooper != nullptr);
    mQueue = mLooper->mQueue;
    mCallback = cb;
    mAsynchronous = async;
}

Handler::MsgPtr Handler::obtainMessage(){
    return Message::obtain(this);
}
Handler::MsgPtr Handler::obtainMessage(int what, Object* obj){
    if(obj == nullptr){
        return Message::obtain(this, what);
    }
    return Message::obtain(this, what, obj);
}
Handler::MsgPtr Handler::obtainMessage(int what, int arg1, long long arg2,
                                       Object* obj){
    if(obj == nullptr){
        return Message::obtain(this, what, arg1, arg2);
    }
    return Message::obtain(this, what, arg1, arg2, obj);
}

void Handler::dispatchMessage(Message* m){
    if(m->callback.cb){
        (*m->callback.cb)();
        m->callback.cb->reset();
    }else{
        if(mCallback && mCallback->handleMessage(m)){
            return;
        }
        //handleMessage(m);
    }
}

bool Handler::enqueueMessage(MessageQueue* queue, Message* msg,
                    long long uptimeMillis)
{
    msg->target = this;
    if(mAsynchronous){
        msg->setAsynchronous(true);
    }
    return queue->enqueueMessage(msg, uptimeMillis);
}

Handler::boolean Handler::sendMessageAtFrontOfQueue(Message* msg){
    MessageQueue* queue = mQueue;
    if(queue == nullptr){
        _LOG_ERR("sendMessageAtFrontOfQueue >> MessageQueue is null.");
        return false;
    }
    return enqueueMessage(queue, msg, 0);
}
Handler::boolean Handler::sendMessageAtTime(Message* msg,
                                            long long uptimeMillis){
    MessageQueue* queue = mQueue;
    if(queue == nullptr){
        _LOG_ERR("sendMessageAtTime >> MessageQueue is null.");
        return false;
    }
    return enqueueMessage(queue, msg, uptimeMillis);
}

Handler::boolean Handler::sendMessageDelayed(Message* msg,
                                             long long delayMillis){
    if (delayMillis < 0) {
        delayMillis = 0;
    }
    //nan -> mills
    auto time = getCurTime() + delayMillis;
    return sendMessageAtTime(msg, time);
}
Handler::boolean Handler::sendEmptyMessageAtTime(int what,
                                                 long long uptimeMillis){
    auto msg = Message::obtain();
    msg->what = what;
    return sendMessageAtTime(msg, uptimeMillis);
}

Handler::boolean Handler::sendEmptyMessageDelayed(int what,
                                                 long long delayMillis){
    auto msg = Message::obtain();
    msg->what = what;
    auto time = getCurTime() + delayMillis;
    return sendMessageAtTime(msg, time);
}

Handler::boolean Handler::post(Func_Callback r, CString name){
    Message* m = Message::obtain();
    m->callback.cb = r;
    m->callback.name = name;
    return sendMessageDelayed(m, 0);
}
Handler::boolean Handler::postAtTime(Func_Callback r,
                                     long long time, CString name){
    Message* m = Message::obtain();
    m->callback.cb = r;
    m->callback.name = name;
    return sendMessageAtTime(m, time);
}
Handler::boolean Handler::postAtTime(Func_Callback r,Object* token,
                                     long long time, CString name){
    Message* m = Message::obtain();
    m->callback.cb = r;
    m->callback.name = name;
    m->obj.copyFrom(token);
    return sendMessageAtTime(m, time);
}
Handler::boolean Handler::postDelayed(Func_Callback r, long long delayMills,
                    CString name){
    Message* m = Message::obtain();
    m->callback.cb = r;
    m->callback.name = name;
    return sendMessageDelayed(m, delayMills);
}

Handler::boolean Handler::postAtFrontOfQueue(Func_Callback r, CString name){
    Message* m = Message::obtain();
    m->callback.cb = r;
    m->callback.name = name;
    return sendMessageAtFrontOfQueue(m);
}

Handler::boolean Handler::runWithScissors(Func_Callback r, long long timeout,
                        CString name){
    if(!r){
        return false;
    }
    if(timeout < 0){
        timeout = 0;
    }
    if(Looper::myLooper() == mLooper){
        (*r)();
        return true;
    }
    _RAII raii(new BlockingRunnable(r, name));
    return raii.br->postAndWait(this, timeout);
}

void Handler::removeCallbacks(Func_Callback r, Object* token){
    mQueue->removeMessages(this, r, token);
}
void Handler::removeCallbacks(Func_Callback r){
    mQueue->removeMessages(this, r, nullptr);
}
void Handler::removeMessages(int what){
    mQueue->removeMessages(this, what, nullptr);
}
void Handler::removeMessages(int what, Object* obj){
    mQueue->removeMessages(this, what, obj);
}
void Handler::removeMessagesEquals(int what, Object* obj){
    mQueue->removeMessages(this, what, obj, true);
}
void Handler::removeMessages(Message* target,
                             std::function<int(MsgPtr, MsgPtr)> comparator){
    mQueue->removeMessages(this, target, comparator);
}
void Handler::removeCallbacksAndMessages(Object* token){
    mQueue->removeCallbacksAndMessages(this, token);
}
Handler::boolean Handler::hasMessages(int what){
    return mQueue->hasMessages(this, what, nullptr);
}
Handler::boolean Handler::hasMessages(int what, Object* object){
    return mQueue->hasMessages(this, what, object);
}
Handler::boolean Handler::hasCallbacks(Func_Callback r){
    return mQueue->hasMessages(this, r, nullptr);
}

void Handler::dump(std::stringstream& ss, CString prefix){
    auto c = getCurTime();
    char buf[256];
    snprintf(buf, 256, "%s_%p @ time=", prefix.data(), this);

    ss << std::string(buf) << formatTime(c) << _NEW_LINE;
    if(!mLooper){
        ss << prefix << "looper not inited." << _NEW_LINE;
    }else{
        mLooper->dump(ss, prefix + " ");
    }
}

}
