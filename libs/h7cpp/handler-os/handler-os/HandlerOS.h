#pragma once

#include "handler-os/Handler.h"
#include "handler-os/Looper.h"
#include "handler-os/HandlerThread.h"

namespace h7_handler_os{

class HandlerOS{

public:
    static void loopMain(){
        Looper::prepareMainLooper();
        Looper::getMainLooper()->loop();
    }

    void start(std::function<bool(Message*)> func){
        start(std::make_shared<HandlerCallback>(func));
    }

    void start(std::shared_ptr<HandlerCallback> cb){
        m_ht = std::unique_ptr<HandlerThread>(new HandlerThread());
        m_ht->setAfterQuitCallback([this](){
            delete this;
        });
        m_ht->start();
        m_handler = std::shared_ptr<Handler>(new Handler(m_ht->getLooper(), cb));
    }

    std::shared_ptr<Handler> newHandler(std::shared_ptr<HandlerCallback> cb){
        if(m_handler){
            return std::shared_ptr<Handler>(new Handler(m_ht->getLooper(), cb));
        }
        return nullptr;
    }

    std::shared_ptr<Handler> getHandler(){
        return m_handler;
    }

    void quit(){
        if(m_ht){
            m_ht->quit();
        }
    }
    void quitSafely(){
        if(m_ht){
            m_ht->quitSafely();
        }
    }

private:
    std::unique_ptr<HandlerThread> m_ht;
    std::shared_ptr<Handler> m_handler;
};
}
