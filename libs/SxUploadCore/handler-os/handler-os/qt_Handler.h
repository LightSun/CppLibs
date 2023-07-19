#pragma once

#ifdef BUILD_WITH_QT
#include <QApplication>
#include <QObject>
#include <QTimer>
#include <QDebug>
#include <QEvent>
#include "common/common.h"
//#include "../events.h"

#define QTH_EVENT_TYPE(name, offset)\
static const QEvent::Type name = static_cast<QEvent::Type>(QEvent::User + offset);
QTH_EVENT_TYPE(TYPE_INTERNAL_USE, 70567)

namespace h7_handler_os{
class Message;

extern void handler_os_delete_msg(h7_handler_os::Message*);
extern void handler_os_dispatch_msg(Message* m);
}

namespace h7 {

    using HHMsg = h7_handler_os::Message;

    class __Event: public QEvent{
    public:
        HHMsg* msg {nullptr};
        FUNC_SHARED_PTR(void()) ptr;
        inline __Event(HHMsg* msg):
            QEvent(TYPE_INTERNAL_USE), msg(msg) {}
        inline __Event(FUNC_SHARED_PTR(void()) ptr):
            QEvent(TYPE_INTERNAL_USE), ptr(ptr) {}
        ~__Event(){
            if(msg){
                handler_os_delete_msg(msg);
                msg = nullptr;
            }
        }
    };

    class Handler: public QObject{
        Q_OBJECT
    public:
        Handler& operator=(const Handler&) = delete;

        static Handler& get(){
            static Handler handler;
            return handler;
        }
        //post a function to run right-now.
        inline void post(HHMsg* ptr){
            QApplication::instance()->postEvent(this, new __Event(ptr));
        }
        inline void post(std::function<void()> func){
            auto ptr = FUNC_MAKE_SHARED_PTR_0(void(), func);
            QApplication::instance()->postEvent(this, new __Event(ptr));
        }
        inline void post(FUNC_SHARED_PTR(void()) ptr){
            QApplication::instance()->postEvent(this, new __Event(ptr));
        }
        inline void postDelay(HHMsg* ptr, int delayMs){
            if(delayMs > 0){
                QTimer::singleShot(delayMs, this, [this, ptr]() {
                    post(ptr);
                });
            }else{
                post(ptr);
            }
        }
        inline void postDelay(std::function<void()> func, int delayMs){
            auto ptr = FUNC_MAKE_SHARED_PTR_0(void(), func);
            if(delayMs > 0){
                QTimer::singleShot(delayMs, this, [this, ptr]() {
                    post(ptr);
                });
            }else{
                post(ptr);
            }
        }
    protected:
        inline void customEvent(QEvent * e)override{
            //qDebug() << "---Handler::customEvent";
            __Event* qe = (__Event*)e;
            if(qe->msg){
                handler_os_dispatch_msg(qe->msg);
            }else{
                (*qe->ptr)();
            }
        }
    };
    //must init in main thread for ui.
    inline void handler_post(std::function<void()> func, int delayMs = 0){
        Handler::get().postDelay(func, delayMs);
    }
    inline void handler_post(HHMsg* ptr, int delayMs = 0){
        Handler::get().postDelay(ptr, delayMs);
    }
}
#endif
