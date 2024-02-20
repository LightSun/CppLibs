#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "hv/hlog.h"

namespace h7 {

class Logger{
public:
    Logger(){
        m_log = logger_create();
        logger_set_file(m_log, "sx_core");
        logger_set_max_filesize_by_str(m_log, "20M");
        logger_set_level_by_str(m_log, "DEBUG");
        logger_enable_fsync(m_log, 1);
    }
    ~Logger(){
        logger_fsync(m_log);
        logger_destroy(m_log);
    }

    void setLogLevel(int level){
        logger_set_level(m_log, level);
    }
public:
    logger_t* m_log {nullptr};
};

}
h7::Logger* med_getLogger();
#define H7_LOG med_getLogger()->m_log
#define med_logger_print(level, fmt, ...)\
do{\
    printf(fmt, ## __VA_ARGS__);\
    logger_print(med_getLogger()->m_log, level, fmt " [%s:%d:%s]\n", \
        ##__VA_ARGS__, __FILENAME__, __LINE__, __FUNCTION__);\
    logger_fsync(med_getLogger()->m_log);\
}while(0);

#define h7_logd(fmt, ...) med_logger_print(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define h7_logi(fmt, ...) med_logger_print(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define h7_logw(fmt, ...) med_logger_print(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define h7_loge(fmt, ...) med_logger_print(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define h7_logf(fmt, ...) med_logger_print(LOG_LEVEL_FATAL, fmt, ##__VA_ARGS__)


#endif // LOGGER_HPP
