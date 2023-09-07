#pragma once

#ifdef BUILD_WITH_QT

#include <QEvent>
#include <functional>
#include <future>
#include <map>

#define QTH_EVENT_TYPE(name, offset)\
static const QEvent::Type name = static_cast<QEvent::Type>(QEvent::User + offset);
QTH_EVENT_TYPE(TYPE_HANDLER_OS_MSG, 70567)
QTH_EVENT_TYPE(TYPE_HANDLER_OS_INTERNAL, 70568)

namespace h7_handler_os{
class Message;

extern void handler_os_delete_msg(h7_handler_os::Message*);
extern void handler_os_dispatch_msg(Message* m);

extern void handler_os_prepare_qtLooper();
extern void handler_qt_post_func(std::function<void()> func, int delayMs);
extern void handler_qt_post_msg(h7_handler_os::Message* ptr, int delayMs);
}

namespace h7_handler_os {
    using HHMsg = h7_handler_os::Message;
    using ShareFunc0 = std::shared_ptr<std::packaged_task<void()>>;

    class Handler;
    class Object;

    class __Event: public QEvent{
    public:
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
    struct _QTApplication_ctx{
        struct Item{
            void* msgPtr;
            long long time;

            friend bool operator==(const Item& i1, const Item& i2){
                return i1.msgPtr == i2.msgPtr;
            }
        };
        struct ItemCmp{
            bool operator()(const Item& it1, const Item& it2)const{
                return it1.time < it2.time;
            }
        };
        using MsgPtr = Message*;
        using Func_Callback = std::shared_ptr<std::packaged_task<void()>>;
        using String = std::string;
        using CString = const String&;

        std::mutex mtx;
        std::map<Item, QEvent*, ItemCmp> events;
        std::atomic_bool mQuitting {false};
        int mNextBarrierToken {0};
        std::thread::id mTid;

        //
        _QTApplication_ctx(){
            mTid = std::this_thread::get_id();
        }

        bool isCurrentThread(){
            return std::this_thread::get_id()== mTid;
        }

        bool hasEventThenRemove(QEvent* event);
        void addEvent(QEvent* event);
        //void removeEvent(void* msgPtr);
        bool isIdle();
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

        bool isPollingLocked(){
            return !mQuitting;
        }
    };
}
#endif
