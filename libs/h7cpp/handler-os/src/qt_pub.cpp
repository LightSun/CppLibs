#include "handler-os/qt_pub.h"
#include "handler-os/qt_Handler.h"

#ifdef BUILD_WITH_QT

namespace h7_handler_os{

void handler_qt_post_func(std::function<void()> func, int delayMs){
    QtHandler::get()->postDelay(func, delayMs);
}

void handler_qt_post_msg(h7_handler_os::Message* ptr){
    QtHandler::get()->post(ptr);
}

void handler_qt_post_func_async(std::function<void()> func, int delayMs){
    QtHandler::get()->postAsync(func, delayMs);
}

}

#endif
