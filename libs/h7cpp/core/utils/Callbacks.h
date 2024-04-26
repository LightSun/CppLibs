#pragma once

#include <memory>


namespace h7 {

//wara raw ptr and smart ptr.
template<typename Callback, typename Invoker,
         typename Ptr = std::shared_ptr<Callback>>
struct CallbackWrapper{
    Callback* ptr {nullptr};
    Ptr spPtr;

    template<typename Parameter>
    void sendPacket(std::shared_ptr<Parameter> pkg){
        Callback* impl = ptr != nullptr ? ptr : spPtr.get();
        Invoker()(impl, pkg);
    }
    template<typename Parameter, class... Args>
    void sendNewPacket(Args&& ... args){
        Callback* impl = ptr != nullptr ? ptr : spPtr.get();
        Invoker()(impl, std::make_shared<Parameter>(std::forward<Args>(args)...));
    }
};

template<typename Callback, typename Parameter,
         typename Ptr = std::shared_ptr<Callback>>
struct CallbackWrapper2{
    Callback* ptr {nullptr};
    Ptr spPtr;

    template<typename Handler>
    void sendPacket(Handler* inv,std::shared_ptr<Parameter> pkg){
        Callback* impl = ptr != nullptr ? ptr : spPtr.get();
        inv->handle(impl, pkg);
    }

    template<typename Handler, class... Args>
    void sendNewPacket(Handler* inv, Args&& ... args){
        Callback* impl = ptr != nullptr ? ptr : spPtr.get();
        auto pkg = std::make_shared<Parameter>(std::forward<Args>(args)...);
        inv->handle(impl, pkg);
    }
};


}
