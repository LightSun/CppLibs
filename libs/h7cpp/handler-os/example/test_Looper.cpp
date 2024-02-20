
#include "handler-os/Looper.h"
#include "../src/_common_pri.h"

using namespace h7_handler_os;

void testLooper(){
    Looper::prepareMainLooper();

    auto lp = Looper::myLooper();
    _HANDLER_ASSERT(lp);
}
