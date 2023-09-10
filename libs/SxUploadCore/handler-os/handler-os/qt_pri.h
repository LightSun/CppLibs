#pragma once

#ifdef BUILD_WITH_QT

#include <QEvent>
#include <functional>
#include <future>
#include <map>

#include "handler-os/qt_pub.h"

//max 65535
#define QTH_EVENT_TYPE(name, offset)\
static const QEvent::Type name = static_cast<QEvent::Type>(QEvent::User + offset);
QTH_EVENT_TYPE(TYPE_HANDLER_OS_MSG, 30567)
QTH_EVENT_TYPE(TYPE_HANDLER_OS_INTERNAL, 30568)

namespace h7_handler_os{
class Message;

extern void handler_os_delete_msg(h7_handler_os::Message*);
extern void handler_os_dispatch_msg(Message* m);

extern void handler_os_prepare_qtLooper();
}

namespace h7_handler_os {
    using HHMsg = h7_handler_os::Message;
    using ShareFunc0 = std::shared_ptr<std::packaged_task<void()>>;

    class Handler;
    class Object;

    class __Event: public QEvent{
    public:
        void* rec {nullptr};
        HHMsg* msg {nullptr};
        ShareFunc0 ptr;
        inline __Event(HHMsg* msg):
            QEvent(TYPE_HANDLER_OS_MSG), msg(msg) {}

        inline __Event(ShareFunc0 ptr):
            QEvent(TYPE_HANDLER_OS_INTERNAL), ptr(ptr) {}
        ~__Event(){
            if(msg){
                handler_os_delete_msg(msg);
                msg = nullptr;
            }
        }
    };
    struct HEvent{
        QEvent* event {nullptr};
        HEvent* next {nullptr};

        using Func_Callback = std::shared_ptr<std::packaged_task<void()>>;
        using MsgPtr = Message*;
        using CString = const std::string&;

        ~HEvent(){
            recyleMsg();
        }
        void recyleMsg();
        Message* msgPtr();
        void recycleUnchecked();
    };

    struct _QTApplication_ctx{
        using LL = long long;
        //using ShareItem = std::shared_ptr<Item>;
//        struct ItemCmp{
//            bool operator()(const ShareItem& it1, const ShareItem& it2)const{
//                return it1->time < it2->time;
//            }
//        };
        using MsgPtr = Message*;
        using Func_Callback = std::shared_ptr<std::packaged_task<void()>>;
        using String = std::string;
        using CString = const String&;
        using HEPtr = HEvent*;

        _IdleExecutor* mIdleExe {nullptr};
        std::mutex mtx;
        HEvent* mMessages {nullptr};
        std::atomic_bool mQuitting {false};
        std::atomic_bool mIdleChecking {false};
        int mNextBarrierToken {0};
        int mIdleThreshold {10};
        std::thread::id mTid;

        //
        _QTApplication_ctx(){
            mTid = std::this_thread::get_id();
        }
        bool isCurrentThread(){
            return std::this_thread::get_id()== mTid;
        }
        void setIdleTimeThreshold(int msec){
            mIdleThreshold = msec;
        }
        bool isPollingLocked(){
            return !mQuitting;
        }

        void checkIdle();
        bool hasEventThenRemove(QEvent* event);
        void addEvent(QObject* receiver, QEvent* event);
        //void removeEvent(void* msgPtr);
        bool isIdle(int deltaMs);
        void removeMessages(Handler* h, int what,
                                          Object* object, bool allowEquals);
        void removeMessages(Handler* h, Message* target,
                            std::function<int(MsgPtr, MsgPtr)> comparator);
        void removeMessages(Handler* h, Func_Callback r,
                                          Object* object);
        void removeCallbacksAndMessages(Handler* h, Object* object);

        bool hasMessages(Handler* h, int what, Object* object);
        bool hasMessages(Handler* h, Func_Callback cb, Object* object);

        bool enqueueMessage(Message* msg, long long when);

        int postSyncBarrier(long long when);
        bool removeSyncBarrier(int token);

        void quit(bool safe);
        void dump(std::stringstream& ss, CString prefix);

    private:
        void removeAllMessagesLocked();
        void removeAllFutureMessagesLocked();
        __Event* findEvent(MsgPtr ptr);
    };
}
#endif
