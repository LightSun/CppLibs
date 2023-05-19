#ifndef _XBACKTRACE_H
#define _XBACKTRACE_H
//support win/linux/mac

namespace h7 {

class Backtrace{
public:
static void enableBacktrace(const char* exe_name,int ret_code = -1);

//only for windows
static void attachConsole();

};

}



#endif // XBACKTRACE_H
