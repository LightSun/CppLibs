#pragma once

#include <string>
#include <memory>
#include <future>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace h7_handler_os{

class Looper;

class HandlerThread
{
public:
    using String = std::string;
    using CString = const std::string&;
    using PkgTask = std::packaged_task<void()>;

    HandlerThread(CString name = "", int priority = 0):
        mName(name), mPriority(priority){
    }
    void setFriendCpuId(int cpuId){
        mFriendCpuId = cpuId;
    }
    void setOnPreparedCallback(std::function<void()> func){
        mOnPrepared = std::make_shared<PkgTask>(func);
    }
    void setOnPreparedCallback(std::shared_ptr<PkgTask> ptr){
        mOnPrepared = ptr;
    }
    void setAfterQuitCallback(std::function<void()> func){
        setAfterQuitCallback(std::make_shared<PkgTask>(func));
    }
    void setAfterQuitCallback(std::shared_ptr<PkgTask> ptr){
        mAfterQuit = ptr;
    }
    void deleteAfterQuit(){mDeleteAfterQuit = true;}

    //sync, it will block until quit.
    void run();
    //async
    void start();
    bool quit();
    bool quitSafely();
    std::shared_ptr<Looper> getLooper();

private:
    void run_main();

private:
    std::shared_ptr<Looper> mLooper;
    std::shared_ptr<PkgTask> mOnPrepared;
    std::shared_ptr<PkgTask> mAfterQuit;
    std::mutex mMutex;
    std::condition_variable mConv;
    String mName;
    std::atomic_bool mDestroied {false};
    bool mDeleteAfterQuit {false};
    int mPriority {0};
    int mFriendCpuId {-1};
};

}
