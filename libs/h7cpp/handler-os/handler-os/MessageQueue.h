#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include <future>
#include <sstream>
#include <atomic>

#include "handler-os/qt_pub.h"

namespace h7_handler_os{

#ifdef BUILD_WITH_QT
class _QTApplication_ctx;
#endif

class Message;
class Handler;
class Object;

class IdleHandlerI{
public:
    virtual ~IdleHandlerI(){};
    virtual bool queueIdle() = 0;
};

class IdleHandler{
private:
    IdleHandlerI* handler_ptr {nullptr};
    std::shared_ptr<IdleHandlerI> handler;
    std::shared_ptr<std::packaged_task<bool()>> task;
    std::mutex mutex;
public:
    IdleHandler(IdleHandlerI* ptr):handler_ptr(ptr){}
    IdleHandler(std::shared_ptr<IdleHandlerI> ptr):handler(ptr){}
    IdleHandler(std::function<bool()> func){
        task = std::make_shared<std::packaged_task<bool()>>(func);
    }
    /**
     * Called when the message queue has run out of messages and will now
     * wait for more. Return true to keep your idle handler active, false to
     * make it removed. This may be called if there are still messages
     * pending in the queue, but they are all scheduled to be dispatched
     * after the current time.
     */
    bool queueIdle(){
        if(handler){
            return handler->queueIdle();
        }
        if(handler_ptr){
            return handler_ptr->queueIdle();
        }
        std::unique_lock<std::mutex> lck(mutex);
        if(task){
            auto fu = task->get_future();
            (*task)();
            bool ret = fu.get();
            task->reset();
            return ret;
        }
        return false;
    }
};

class MessageQueue
    #ifdef BUILD_WITH_QT
        :public _IdleExecutor
    #endif
{
public:
    using MsgPtr = Message*;
    using CString = const std::string&;
    using Func_Callback = std::shared_ptr<std::packaged_task<void()>>;

    MessageQueue(bool quitAllowed):mQuitAllowed(quitAllowed){
    }
#ifdef BUILD_WITH_QT
    MessageQueue(_QTApplication_ctx* ctx);
#endif
    ~MessageQueue(){}

    bool isIdle();

    void addIdleHandler(std::shared_ptr<IdleHandler> p){
        if(p){
            std::unique_lock<std::mutex> lck(mMutex);
            mIdleHandlers.push_back(p);
        }
    }
    void removeIdleHandler(std::shared_ptr<IdleHandler> p){
        std::unique_lock<std::mutex> lck(mMutex);
        for(auto it = mIdleHandlers.begin() ; it != mIdleHandlers.end() ; ++it){
            if(*it == p){
                mIdleHandlers.erase(it);
                break;
            }
        }
    }
    bool isPolling() {
        std::unique_lock<std::mutex> lck(mMutex);
        return isPollingLocked();
    }

    Message* next();

    /**
     * Posts a synchronization barrier to the Looper's message queue.
     *
     * Message processing occurs as usual until the message queue encounters the
     * synchronization barrier that has been posted. When the barrier is
     * encountered, later synchronous messages in the queue are stalled
     * (prevented from being executed) until the barrier is released by calling
     * {@link #removeSyncBarrier} and specifying the token that identifies the
     * synchronization barrier.
     *
     * This method is used to immediately postpone execution of all subsequently
     * posted synchronous messages until a condition is met that releases the
     * barrier. Asynchronous messages (see {@link Message#isAsynchronous} are
     * exempt from the barrier and continue to be processed as usual.
     *
     * This call must be always matched by a call to {@link #removeSyncBarrier}
     * with the same token to ensure that the message queue resumes normal
     * operation. Otherwise the application will probably hang!
     *
     * @return A token that uniquely identifies the barrier. This token must be
     *         passed to {@link #removeSyncBarrier} to release the barrier.
     * //when have a barrier in the head. only the async msg will be dispatch.
     * @hide
     */
    int postSyncBarrier();
    /**
     * Removes a synchronization barrier.
     *
     * @param token
     *            The synchronization barrier token that was returned by
     *            {@link #postSyncBarrier}.
     *
     * @hide
     */
    bool removeSyncBarrier(int token);

#ifdef BUILD_WITH_QT
    ///often called by QTApp
    void runIdleTasks() override;
#endif

private:
    bool isPollingLocked();

    void removeMessages(Handler* h, int what, Object* obj){
        removeMessages(h, what, obj, false);
    }
    void removeMessages(Handler* h, int what, Object* obj,
                        bool allowEquals);

    void removeMessages(Handler* h, Message* target,
                        std::function<int(MsgPtr, MsgPtr)> comparator);

    void removeMessages(Handler* h, Func_Callback cb,Object* object);

    void removeCallbacksAndMessages(Handler* h, Object* object);

    void removeAllMessagesLocked();
    void removeAllFutureMessagesLocked();
    int postSyncBarrier(long long when);

    void quit(bool safe);

    bool enqueueMessage(Message* msg, long long when);
    bool hasMessages(Handler* h, int what, Object* object);
    bool hasMessages(Handler* h, Func_Callback cb, Object* object);

    void dump(std::stringstream& ss, CString prefix);

private:
    std::vector<std::shared_ptr<IdleHandler>> mIdleHandlers;
    std::vector<std::shared_ptr<IdleHandler>> mPendingIdleHandlers;
    bool mQuitAllowed {false};
    std::atomic_bool mQuitting {false};
    std::atomic_bool mBlocked {false};
    int mNextBarrierToken {0};
    Message* mMessages {nullptr};
    std::mutex mMutex;
    //
#ifdef BUILD_WITH_QT
    _QTApplication_ctx* m_qt_ctx {nullptr};
#endif

    friend class Looper;
    friend class Handler;
    friend class _QTApplication_ctx;
};

}
