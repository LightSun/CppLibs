#ifndef SX_WEB_PROTOCOL_H
#define SX_WEB_PROTOCOL_H

#include "common/c_common.h"
#include <string>

namespace h7_sx_pro {
    namespace web {
        struct UploadFileRes{
            std::string file;
            std::string hash;
            std::string msg;
        };
        enum {
            kType_WEB_ERR_MSG = 1
        };
    }

}

#endif // SX_WEB_PROTOCOL_H
