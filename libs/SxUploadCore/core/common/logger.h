#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <string.h>
#include "common/c_common.h"

enum{
    H7_LOG_LEVEL_VERBOSE = 0,
    H7_LOG_LEVEL_DEBUG,
    H7_LOG_LEVEL_INFO,
    H7_LOG_LEVEL_WARN,
    H7_LOG_LEVEL_ERROR,
    H7_LOG_LEVEL_FATAL,
    H7_LOG_LEVEL_SILENT,
};
#ifndef __FILENAME__
// #define __FILENAME__  (strrchr(__FILE__, DIR_SEPARATOR) ? strrchr(__FILE__, DIR_SEPARATOR) + 1 : __FILE__)
#define __FILENAME__  (strrchr(DIR_SEP_STR __FILE__, DIR_SEP) + 1)
#endif

typedef struct logger_s logger_t;

extern "C" int logger_print(logger_t* logger, int level, const char* fmt, ...);

namespace h7 {

typedef struct LoggerParam_{
    std::string name {"core"};
    std::string max_fsize {"20M"};
    std::string level {"DEBUG"};
}LoggerParam;

class Logger{
public:
    Logger(){};
    ~Logger();
    void init(const LoggerParam& p);
    void setLogLevel(int level);
   // void println(int level, const char* fmt, va_list va);
public:
    logger_t* m_log {nullptr};
};

h7::Logger* getLogger();
static inline void initLogger(const LoggerParam& lp){
    getLogger()->init(lp);
}

}
//-----------------------------------
#define med_logger_print(level, fmt, ...)\
do{\
    printf(fmt, ## __VA_ARGS__);\
    logger_print(h7::getLogger()->m_log, level, fmt " [%s:%d:%s]\n", \
        ##__VA_ARGS__, __FILENAME__, __LINE__, __FUNCTION__);\
}while(0);

#define h7_logd(fmt, ...) med_logger_print(H7_LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define h7_logi(fmt, ...) med_logger_print(H7_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define h7_logw(fmt, ...) med_logger_print(H7_LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define h7_loge(fmt, ...) med_logger_print(H7_LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define h7_logf(fmt, ...) med_logger_print(H7_LOG_LEVEL_FATAL, fmt, ##__VA_ARGS__)


#endif // LOGGER_HPP
