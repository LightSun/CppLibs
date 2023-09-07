#include "handler-os/QTApplication.h"
#include "handler-os/qt_pri.h"

#include "handler-os/MessageQueue.h"

#ifdef BUILD_WITH_QT

using namespace h7_handler_os;

//static QTApplication* _App = nullptr;

QTApplication::QTApplication(int& argc, char** argv):QApplication(argc, argv){
   // _App = this;
    m_ctx = new _QTApplication_ctx();
    handler_os_prepare_qtLooper();
    handler_qt_post_func([](){
        printf("QTApplication >> init ok !\n");
    }, 0);
}
QTApplication::~QTApplication(){
    if(m_ctx){
        delete m_ctx;
        m_ctx = nullptr;
    }
}

QTApplication* QTApplication::get(){
    return (QTApplication*)QApplication::instance();
}

void QTApplication::postEvent2(QObject *receiver, QEvent *event){
    if(event->type() == TYPE_HANDLER_OS_MSG){
        m_ctx->addEvent(event);
    }
    postEvent(receiver, event);
}
bool QTApplication::notify(QObject *obj, QEvent *event){
    //barrier,
    if(event->type() == TYPE_HANDLER_OS_MSG){
        if(m_ctx->hasEventThenRemove(event)){
            return QApplication::notify(obj, event);
        }else{
            event->ignore();
        }
        return true;
    }
    return QApplication::notify(obj, event);
}

#endif
