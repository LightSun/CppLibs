#include "../src/_common_pri.h"
#include "../src/_time.h"

using DelayClock = std::chrono::steady_clock;
using TimeT = std::chrono::time_point<DelayClock, std::chrono::milliseconds>;
using namespace h7_handler_os;

void testTime(){
    auto t = getCurTime();
    _LOG_INFO(t);
}
