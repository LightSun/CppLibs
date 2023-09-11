#pragma once

#include "handler-os/QTApplication.h"
#include "handler-os/Looper.h"
#include "handler-os/HandlerOS.h"
#include "handler-os/Message.h"
#include "handler-os/MessageQueue.h"

#include "src/_common_pri.h"
#include "src/Locker.h"

using namespace h7_handler_os;

#ifdef BUILD_WITH_QT
#include <QWidget>
#include <QPushButton>

#define CONN_0(src, src_name, dst, dst_name) \
QObject::connect(src, SIGNAL(src_name()), dst, SLOT(dst_name()))


class TestReceiver: public QObject, public HandlerCallbackI{
    Q_OBJECT
public:
    TestReceiver(){
        //for qt main thread. it will not blick by looper.loop.
        m_qtMain = std::make_shared<Handler>(HandlerCallback::make(this));
        _HANDLER_ASSERT(m_qtMain->getLooper()->isCurrentThread());
        m_os.start([](Message* m){
            printf("rec msg >> m.when = %lld\n", m->getWhen());
            return true;
        });
        m_cb = Handler::makeCallback([](){
            printf("TestReceiver >> m_cb callback is called.\n");
        });
        m_idHandler = std::make_shared<IdleHandler>([](){
            printf("TestReceiver >> IdleHandler callback is run.\n");
            return true;
        });
    }
    void testSendDelayOnMain(){
        m_qtMain->sendEmptyMessageDelayed(1, 1000);
    }

    void testRemoveMsgOnMain(){
        m_qtMain->postDelayed(m_cb, 2000, "testRemoveMsgOnMain");
        m_qtMain->postDelayed([this](){
            m_qtMain->removeCallbacks(m_cb);
            printf("TestReceiver >> qt main callback msg is removed.\n");
        }, 1000);
    }

    void testIdleTask(){
        m_qtMain->getLooper()->getQueue()->addIdleHandler(m_idHandler);
        m_qtMain->postDelayed([](){
            printf("TestReceiver >> testIdleTask.\n");
        }, 1000);
    }

//---------------------------

    bool handleMessage(Message* m) override{
        printf("main thread >> rec msg >> m.what = %d, m.when = %lld\n",
               m->what, m->getWhen());
        return true;
    }
public slots:
    void onClicked(){
        printf("TestReceiver >> onClicked...\n");
        //m_os.getHandler()->sendEmptyMessage(0);
        //testSendDelayOnMain();
        //testRemoveMsgOnMain();
        testIdleTask();
    }

private:
    HandlerOS m_os;
    std::shared_ptr<Handler> m_qtMain;
    Handler::Func_Callback m_cb;
    std::shared_ptr<IdleHandler> m_idHandler;
};
#endif
