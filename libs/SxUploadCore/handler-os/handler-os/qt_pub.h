#pragma once

#include <functional>

#ifdef BUILD_WITH_QT

namespace h7_handler_os{
class Message;

void handler_qt_post_func(std::function<void()> func, int delayMs);

void handler_qt_post_msg(h7_handler_os::Message* ptr, int delayMs);

}

#endif
