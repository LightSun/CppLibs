
#include <stdarg.h>
#include "logger.h"

#ifdef BUILD_WITH_LIBHV
    #include "hv/hlog.h"
#else
    #define HV_EXPORT
    #include "_hlog.h"
    //#include "_hlog.c"
#endif

namespace h7 {

void Logger::init(const LoggerParam& p){
    //__FILENAME__
    m_log = logger_create();
    logger_set_file(m_log, p.name.data());
    logger_set_max_filesize_by_str(m_log, p.max_fsize.data());
    logger_set_level_by_str(m_log, p.level.data());
    logger_enable_fsync(m_log, 1);
}
Logger::~Logger(){
    if(m_log){
        logger_fsync(m_log);
        logger_destroy(m_log);
        m_log = nullptr;
    }
}
void Logger::setLogLevel(int level){
   logger_set_level(m_log, level);
}

h7::Logger* getLogger(){
    static h7::Logger logger;
    return &logger;
}
}
