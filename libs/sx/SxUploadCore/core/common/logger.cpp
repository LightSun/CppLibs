
#include "logger.h"

h7::Logger* med_getLogger(){
    static h7::Logger logger;
    return &logger;
}
