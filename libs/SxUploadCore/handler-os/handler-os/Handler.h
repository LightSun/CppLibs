#pragma once

#include <future>
#include <functional>
#include <chrono>

namespace h7_handler_os{

class Message;
class Looper;
class MessageQueue;
class Object;

class HandlerCallbackI{
public:
    ~HandlerCallbackI(){}
    virtual bool handleMessage(Message* m) = 0;
};

class HandlerCallback{
private:
    HandlerCallbackI* handler_ptr {nullptr};
    std::shared_ptr<HandlerCallbackI> handler;
    std::shared_ptr<std::packaged_task<bool(Message*)>> task;
    std::mutex mutex;
public:
    HandlerCallback(HandlerCallbackI* ptr):handler_ptr(ptr){
    }
    HandlerCallback(std::shared_ptr<HandlerCallbackI> ptr):handler(ptr){
    }
    HandlerCallback(std::function<bool(Message*)> func){
        task = std::make_shared<std::packaged_task<bool(Message*)>>(func);
    }
    static std::shared_ptr<HandlerCallback> make(
            HandlerCallbackI* ptr){
        return std::shared_ptr<HandlerCallback>(new HandlerCallback(ptr));
    }
    static std::shared_ptr<HandlerCallback> make(
            std::shared_ptr<HandlerCallbackI> ptr){
        return std::shared_ptr<HandlerCallback>(new HandlerCallback(ptr));
    }
    static std::shared_ptr<HandlerCallback> make(
           std::function<bool(Message*)> func){
        return std::shared_ptr<HandlerCallback>(new HandlerCallback(func));
    }
    bool handleMessage(Message* m){
        if(handler){
            return handler->handleMessage(m);
        }
        if(handler_ptr){
            return handler_ptr->handleMessage(m);
        }
        std::unique_lock<std::mutex> lck(mutex);
        if(task){
            auto fu = task->get_future();
            (*task)(m);
            bool ret = fu.get();
            task->reset();
            return ret;
        }
        return false;
    }
};

class Handler
{
public:
    using boolean = bool;
    using MsgPtr = Message*;
    using CString = const std::string&;
    using Func_Callback = std::shared_ptr<std::packaged_task<void()>>;

    Handler(std::shared_ptr<HandlerCallback> cb, bool async = false);

    Handler(Looper* looper, std::shared_ptr<HandlerCallback> cb, bool async = false);
    Handler(std::shared_ptr<Looper> looper, std::shared_ptr<HandlerCallback> cb,
            bool async = false):Handler(looper.get(), cb, async){}

    Handler(Looper* looper):Handler(looper, nullptr, false){}
    Handler(std::shared_ptr<Looper> looper):Handler(looper.get(), nullptr, false){}

    Handler(boolean async):Handler(nullptr, async){}
    Handler():Handler(false){}

    ~Handler(){mLooper = nullptr;}

    //---------------------
    static Func_Callback makeCallback(std::function<void()> r){
        return std::make_shared<std::packaged_task<void()>>(r);
    }

    MsgPtr obtainMessage();
    MsgPtr obtainMessage(int what, Object* obj = nullptr);
    MsgPtr obtainMessage(int what, int arg1, long long arg2,
                         Object* obj = nullptr);

    //--------------------------
    void dispatchMessage(Message* m);

    //virtual void handleMessage(Message*){}

    boolean sendMessageAtFrontOfQueue(Message* msg);
    boolean sendMessageAtTime(Message* msg, long long uptimeMillis);
    boolean sendMessageDelayed(Message* msg, long long delayMillis);

    boolean sendEmptyMessageAtTime(int what, long long uptimeMillis);
    boolean sendEmptyMessageDelayed(int what, long long delayMillis);

    boolean sendEmptyMessage(int what){
        return sendEmptyMessageDelayed(what, 0);
    }
    boolean sendMessage(Message* msg){
        return sendMessageDelayed(msg, 0);
    }
    //unlike 'post(Func_Callback r, CString)'
    //after call this. the task can't be removed from call removeCallbacks.
    boolean post(std::function<void()> r, CString name=""){
        return post(makeCallback(r), name);
    }
    boolean post(Func_Callback r, CString name="");

    boolean postAtTime(Func_Callback r, long long timeMills, CString name="");
    boolean postAtTime(Func_Callback r, Object* token,
                       long long timeMills, CString name="");

    boolean postAtTime(std::function<void()> r, long long timeMills, CString name=""){
        return postAtTime(makeCallback(r), timeMills, name);
    }
    boolean postAtTime(std::function<void()> r, Object* token,
                       long long timeMills, CString name=""){
        return postAtTime(makeCallback(r), token, timeMills, name);
    }

    boolean postDelayed(Func_Callback r, long long delayMills,
                        CString name="");
    boolean postDelayed(std::function<void()> r, long long delayMills,
                        CString name=""){
        return postDelayed(makeCallback(r), delayMills, name);
    }

    boolean postAtFrontOfQueue(Func_Callback r, CString name="");
    boolean postAtFrontOfQueue(std::function<void()> r, CString name=""){
        return postAtFrontOfQueue(makeCallback(r), name);
    }

    //run sync
    boolean runWithScissors(std::function<void()> r, long long timeout = 0,
                            CString name=""){
        return runWithScissors(makeCallback(r), timeout, name);
    }
    boolean runWithScissors(Func_Callback r, long long timeout = 0,
                            CString name="");

    void removeCallbacks(Func_Callback r, Object* token);
    void removeCallbacks(Func_Callback r);

    void removeMessages(int what);
    void removeMessages(int what, Object* obj);
    void removeMessagesEquals(int what, Object* object);
    void removeMessages(Message* target, std::function<int(MsgPtr, MsgPtr)> comparator);
    void removeCallbacksAndMessages(Object* token);

    boolean hasMessages(int what);
    boolean hasMessages(int what, Object* object);
    boolean hasCallbacks(Func_Callback r);

    Looper* getLooper(){
        return mLooper;
    }
    void dump(std::stringstream& ss, CString prefix);

private:
    bool enqueueMessage(MessageQueue* queue, Message* msg,
                        long long uptimeMillis);

private:
    bool mAsynchronous {false};
    Looper* mLooper {nullptr};
    MessageQueue* mQueue {nullptr};
    std::shared_ptr<HandlerCallback> mCallback;
};

}
