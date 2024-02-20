#pragma once

#include <QApplication>
#include <QObject>
#include <QTimer>
#include <QDebug>
#include <QEvent>
#include "common.h"
//#include "../events.h"

#define QTH_EVENT_TYPE(name, offset)\
static const QEvent::Type name = static_cast<QEvent::Type>(QEvent::User + offset);
QTH_EVENT_TYPE(TYPE_INTERNAL_USE, 1000)

namespace h7 {

    class __Event: public QEvent{
    public:
        inline __Event(FUNC_SHARED_PTR(void()) ptr):
            QEvent(TYPE_INTERNAL_USE), ptr(ptr) {}
        FUNC_SHARED_PTR(void()) ptr;
    };

    class Handler: public QObject{
        Q_OBJECT
    public:
        ~Handler(){}
       // Handler& operator=(const Handler&) = delete;

        static Handler& get(){
            static Handler handler;
            return handler;
        }

        //post a function to run right-now.
        inline void post(FUNC_SHARED_PTR(void()) ptr){
            QApplication::instance()->postEvent(this, new __Event(ptr));
        }
        inline void post(std::function<void()> func){
            post(FUNC_MAKE_SHARED_PTR_0(void(), func));
        }
        inline void postDelay(std::function<void()> func, int delayMs){
            FUNC_SHARED_PTR(void()) ptr = FUNC_MAKE_SHARED_PTR_0(void(), func);
            if(delayMs > 0){
                QTimer::singleShot(delayMs, this, [this, ptr]() {
                    post(ptr);
                });
            }else{
                post(ptr);
            }
        }
        inline void postAsync(std::function<void()> func, int delayMs){
            FUNC_SHARED_PTR(void()) ptr = FUNC_MAKE_SHARED_PTR_0(void(), func);
            if(delayMs <= 0){
                delayMs = 1;
            }
            QTimer::singleShot(delayMs, this, [ptr]() {
                (*ptr)();
            });
        }
    protected:
        inline void customEvent(QEvent * e)override{
            //qDebug() << "---Handler::customEvent";
            __Event* qe = (__Event*)e;
            (*qe->ptr)();
        }
    };
    //must init in main thread for ui.
    inline void handler_post(std::function<void()> func, int delayMs = 0){
        Handler::get().postDelay(func, delayMs);
    }
    inline void handler_postAsync(std::function<void()> func, int delayMs = 0){
        Handler::get().postAsync(func, delayMs);
    }
}
