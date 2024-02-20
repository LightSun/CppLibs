#pragma once

#include <iostream>
#include <chrono>
#include <thread>
#include <sstream>
#include <functional>

#define FUNC_PLACEHOLDERS_1 std::placeholders::_1
#define FUNC_PLACEHOLDERS_2 FUNC_PLACEHOLDERS_1,std::placeholders::_2
#define FUNC_PLACEHOLDERS_3 FUNC_PLACEHOLDERS_2,std::placeholders::_3
#define FUNC_PLACEHOLDERS_4 FUNC_PLACEHOLDERS_3,std::placeholders::_4
#define FUNC_PLACEHOLDERS_5 FUNC_PLACEHOLDERS_4,std::placeholders::_5
#define FUNC_PLACEHOLDERS_6 FUNC_PLACEHOLDERS_5,std::placeholders::_6
#define FUNC_PLACEHOLDERS_7 FUNC_PLACEHOLDERS_6,std::placeholders::_7
#define FUNC_PLACEHOLDERS_8 FUNC_PLACEHOLDERS_7,std::placeholders::_8
#define FUNC_PLACEHOLDERS_9 FUNC_PLACEHOLDERS_8,std::placeholders::_9

#define FUNC_SHARED_PTR(func_expre) std::shared_ptr<std::packaged_task<func_expre>>
#define FUNC_MAKE_SHARED_PTR(func, func_expre, place_holders) \
std::make_shared<std::packaged_task<func_expre>>(std::bind(func, place_holders))

#define FUNC_MAKE_SHARED_PTR_0(func_expre, func) std::make_shared<std::packaged_task<func_expre>>(std::bind(func))
#define FUNC_MAKE_SHARED_PTR_1(func_expre, func) FUNC_MAKE_SHARED_PTR(func, func_expre, FUNC_PLACEHOLDERS_1)
#define FUNC_MAKE_SHARED_PTR_2(func_expre, func) FUNC_MAKE_SHARED_PTR(func, func_expre, FUNC_PLACEHOLDERS_2)
#define FUNC_MAKE_SHARED_PTR_3(func_expre, func) FUNC_MAKE_SHARED_PTR(func, func_expre, FUNC_PLACEHOLDERS_3)
#define FUNC_MAKE_SHARED_PTR_4(func_expre, func) FUNC_MAKE_SHARED_PTR(func, func_expre, FUNC_PLACEHOLDERS_4)
#define FUNC_MAKE_SHARED_PTR_5(func_expre, func) FUNC_MAKE_SHARED_PTR(func, func_expre, FUNC_PLACEHOLDERS_5)
#define FUNC_MAKE_SHARED_PTR_6(func_expre, func) FUNC_MAKE_SHARED_PTR(func, func_expre, FUNC_PLACEHOLDERS_6)
#define FUNC_MAKE_SHARED_PTR_7(func_expre, func) FUNC_MAKE_SHARED_PTR(func, func_expre, FUNC_PLACEHOLDERS_7)
#define FUNC_MAKE_SHARED_PTR_8(func_expre, func) FUNC_MAKE_SHARED_PTR(func, func_expre, FUNC_PLACEHOLDERS_8)
#define FUNC_MAKE_SHARED_PTR_9(func_expre, func) FUNC_MAKE_SHARED_PTR(func, func_expre, FUNC_PLACEHOLDERS_9)

#define _HANDLER_ASSERT(condition)                                                   \
    do                                                                      \
    {                                                                       \
        if (!(condition))                                                   \
        {                                                                   \
            std::cout << "Assertion failure: " << __FILE__ << "::" << __FUNCTION__  \
                                     << __LINE__ \
                                     << " >> " << #condition << std::endl;  \
            abort();                                                        \
        }                                                                   \
    } while (0)

#define _LOG_DEBUG(msg)\
    std::cout << msg << std::endl;
#define _LOG_DEBUG2(m1, m2)\
    std::cout << m1 << m2 << std::endl;

#define _LOG_INFO(msg)\
    std::cout << msg << std::endl;

#define _LOG_ERR(msg)\
    std::cerr << msg << std::endl;

#ifdef _WIN32
#include <windows.h>
#define sleep_ms(t) Sleep(t);
#define _NEW_LINE "\r\n"
#else
#include <unistd.h>
#define sleep_ms(t) usleep(t*1000);
#define _NEW_LINE "\n"
#endif

// using DelayClock = std::chrono::steady_clock;
// using TimeT = std::chrono::time_point<DelayClock, std::chrono::milliseconds>;
//#define _CUR_TIME_POINT()\
//    std::chrono::duration_cast<std::chrono::milliseconds>(\
//                    DelayClock::now().time_since_epoch())

//#define _T_TIME_POINT(t)\
//    std::chrono::duration_cast<std::chrono::milliseconds>(\
//                    t.time_since_epoch())

//#define _TIME_POINT_MILLS(m)\
//    std::chrono::milliseconds>(m)

//printClockData<std::chrono::system_clock>();
template <typename C>
void _printClockData ()
{
    using namespace std;

    cout << "- precision: ";
    // if time unit is less or equal one millisecond
    typedef typename C::period P;// type of time unit
    if (ratio_less_equal<P,milli>::value) {
       // convert to and print as milliseconds
       typedef typename ratio_multiply<P,kilo>::type TT;
       cout << fixed << double(TT::num)/TT::den
            << " milliseconds" << endl;
    }
    else {
        // print as seconds
        cout << fixed << double(P::num)/P::den << " seconds" << endl;
    }
    cout << "- is_steady: " << boolalpha << C::is_steady << endl;
}

static inline std::string _tid_tostring(const std::thread::id& tid){
    std::stringstream ss;
    ss << tid;
    return ss.str();
}

static inline std::string _cur_tid_tostring(){
    return _tid_tostring(std::this_thread::get_id());
}

//mill seconds
static inline std::string _formatTime(long long time){
    char buf[32];
    double val = time;
    if (val < 1000) {
        snprintf(buf, 32, "%.2f msec", val);
        return std::string(buf);
    }
    val = val / 1000;
    if (val < 60) {
        snprintf(buf, 32, "%.2f sec", val);
        return std::string(buf);
    }
    float format = 60;
    val = val / format;
    if (val < format) {
        snprintf(buf, 32, "%.2f min", val);
        return std::string(buf);
    }
    val = val / format;
    if (val < format) {
        snprintf(buf, 32, "%.2f hour", val);
        return std::string(buf);
    }
    format = 24;
    val = val / format;
    snprintf(buf, 32, "%.2f day", val);
    return std::string(buf);
}

//CALLBACK_COMPAT_FUNC_CLASS(HandlerCallback, handleMessage,
//              Message*, bool, false)
#define CALLBACK_COMPAT_FUNC_CLASS(name, func_name, p_t, r_t, r_t_default)\
    class name##I{\
    public:\
        ~name##I(){}\
        virtual r_t func_name(p_t m) = 0;\
    };\
    class name{\
    private:\
        std::shared_ptr<name##I> handler;\
        std::shared_ptr<std::packaged_task<r_t(p_t)>> task;\
        std::mutex mutex;\
    public:\
        HandlerCallback(std::shared_ptr<name##I> ptr):handler(ptr){\
        }\
        HandlerCallback(std::function<r_t(p_t)> func){\
            task = std::make_shared<std::packaged_task<r_t(p_t)>>(func);\
        }\
        static std::shared_ptr<name> make(\
                std::shared_ptr<name##I> ptr){\
            return std::shared_ptr<name>(new name(ptr));\
        }\
        static std::shared_ptr<name> make(\
               std::function<r_t(p_t)> func){\
            return std::shared_ptr<name>(new name(func));\
        }\
        r_t func_name(p_t m){\
            if(handler){\
                return handler->func_name(m);\
            }\
            std::unique_lock<std::mutex> lck(mutex);\
            if(task){\
                auto fu = task->get_future();\
                (*task)(m);\
                r_t ret = fu.get();\
                task->reset();\
                return ret;\
            }\
            return r_t_default;\
        }\
    };


