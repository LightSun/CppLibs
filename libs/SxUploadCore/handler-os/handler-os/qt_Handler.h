#pragma once

#ifdef BUILD_WITH_QT
#include <QApplication>
#include <QObject>
#include <QTimer>
#include <QDebug>
#include <QEvent>
#include "common/common.h"
//#include "../events.h"
#include "handler-os/qt_pri.h"
#include "handler-os/QTApplication.h"


namespace h7_handler_os {
    class QtHandler: public QObject{
        Q_OBJECT
    public:
        QtHandler& operator=(const QtHandler&) = delete;
        using APP = QTApplication;

        static QtHandler* get(){
            static QtHandler handler;
            return &handler;
        }
        //post a function to run right-now.
        inline void post(HHMsg* ptr){
            APP::get()->postEvent2(this, new __Event(ptr));
        }
        inline void post(std::function<void()> func){
            auto ptr = FUNC_MAKE_SHARED_PTR_0(void(), func);
            APP::get()->postEvent2(this, new __Event(ptr));
        }
        inline void post(FUNC_SHARED_PTR(void()) ptr){
            APP::get()->postEvent2(this, new __Event(ptr));
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
//    inline void handler_post(std::function<void()> func, int delayMs = 0){
//        QtHandler::get()->postDelay(func, delayMs);
//    }
//    inline void handler_post(HHMsg* ptr, int delayMs = 0){
//        QtHandler::get()->postDelay(ptr, delayMs);
//    }
//    inline void qt_post_event(Message* ptr, int delayMs){
//        QtHandler::get()->postDelay(ptr, delayMs);
//    }
}
#endif
