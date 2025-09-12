#pragma once

#include <string>
#include <functional>
#include <future>
#include <chrono>
#include "handler-os/Object.h"

namespace h7_handler_os{

class Handler;

class Message
{
public:
    using PkgTask = std::packaged_task<void()>;
    using Func_Callback = std::shared_ptr<PkgTask>;
    using Arg2Type = long long;
    using String = std::string;
    using CString = const std::string&;

    struct Callback{
        Func_Callback cb;
        String name;
    };
    enum{
        kFlag_IN_USE = 1 << 0,
        kFlag_ASYNCHRONOUS = 1 << 1 /*If set message is asynchronous*/
    };
    ~Message(){}

    static Message* obtain();
    static Message* obtain(Message* src);
    static Message* obtain(Handler* h);
    static Message* obtain(Handler* h, Func_Callback cb, CString cb_name= "");
    static Message* obtain(Handler* h, std::function<void()> cb, CString cb_name="");
    static Message* obtain(Handler* h, int what);
    static Message* obtain(Handler* h, int what, Object* obj);
    static Message* obtain(Handler* h, int what, int arg1, Arg2Type arg2);
    static Message* obtain(Handler* h, int what, int arg1, Arg2Type arg2,
                           Object* obj);

    bool recycle(){
        if(isInUse()){
            return false;
        }
        recycleUnchecked();
        return true;
    }
    void setAsynchronous(bool async) {
        if (async) {
            flags |= kFlag_ASYNCHRONOUS;
        } else {
            flags &= ~kFlag_ASYNCHRONOUS;
        }
    }
    bool isAsynchronous() {
        return (flags & kFlag_ASYNCHRONOUS) != 0;
    }
    void sendToTarget();

    void copyFrom(Message* src);

    String toString();

    //--------------------------
    long long getWhen(){
        return when;
    }
    void setTarget(Handler* h){
        target = h;
    }
    Handler* getTarget(){
        return target;
    }
    Callback& getCallback(){
        return callback;
    }
    Object& getData(){
        return data;
    }
    void setData(const Object& _d){
        data = _d;
    }
    static void logDebugInfo();

private:
    void recycleUnchecked();

    bool isInUse() {
        return ((flags & kFlag_IN_USE) == kFlag_IN_USE);
    }
    void markInUse() {
        flags |= kFlag_IN_USE;
    }
    String getTraceName();

public:
    int what {0};
    int arg1 {0};
    Arg2Type arg2 {0};
    Object obj;


private:
    int flags {0};
    long long when;

    Object data;
    Handler* target {nullptr};
    Callback callback;

    Message* next {nullptr};
    friend class _QTApplication_ctx;
    friend class __Events;
    friend class HEvent;
    friend class StaticObj;
    friend class MessageQueue;
    friend class Looper;
    friend class RAIIMsg;
    friend class Handler;
    friend class Handler;
};

}

