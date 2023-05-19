
#include "common/common.h"
#include "common/logger.h"
#include "utils/Backtrace.h"

void crash2() {
    int *s = 0;
    *s = 100;
}

void crash1() {
    crash2();
}

void test_XBacktrace(){
    h7::initLogger(h7::LoggerParam());
    h7::Backtrace::enableBacktrace("unittest", 0);
    LOGI("test_XBacktrace >> enable backtrace.\n");

    LOGI("test_XBacktrace >> Calling crash function.\n");
    crash1();
}
