#pragma once

#include <thread>

namespace h7_handler_os{

class Trace;
class MessageQueue;

class Looper
{
public:
    using CString = const std::string&;
    using String = std::string;

    ~Looper();

    void setTraceTag(long long traceTag){
        mTraceTag = traceTag;
    }
    bool isCurrentThread(){
        return std::this_thread::get_id() == mTid;
    }
    const MessageQueue* getQueue() {
        return mQueue;
    };

    void quit();

    void quitSafely();

    static void prepare(){
        prepare(true);
    }
    static void prepareMainLooper();
    static std::shared_ptr<Looper> getMainLooper();
    static std::shared_ptr<Looper> myLooper();
    static const MessageQueue* myQueue(){
        return myLooper()->mQueue;
    }

    static void loop();

    void setDebug(bool debug){
        mDebug = debug;
    }
    static void setTrace(std::shared_ptr<Trace> ptr){
        sTrace = ptr;
    }
private:
    Looper(bool quitAllowed);

    void dump(std::stringstream& ss, CString prefix);

    static void prepare(bool quitAllowed);

private:
    std::thread::id mTid;
    long long mTraceTag {0};
    bool mDebug {false};
    MessageQueue* mQueue{nullptr};
    static std::shared_ptr<Trace> sTrace;

    friend class Handler;
};

}
