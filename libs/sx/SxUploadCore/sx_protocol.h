#ifndef SX_PROTOCOL_H
#define SX_PROTOCOL_H

#include "common/common.h"
#include "common/c_common.h"
#include "common/SkRefCnt.h"

#define HASH_SEED 11

namespace h7_sx_pro {

    enum{
        kType_SEND_FILE_START = 1,
        kType_SEND_FILE_RESTART,
        kType_SEND_FILE_PART,
        kType_SEND_FILE_END,
        kType_LOGIN,
        kType_GET_UPLOAD_INFO,
        kType_PAUSE,
        kType_UPLOAD_OK, //8

        //main from server
        kType_STOP_UPLOAD  = 9,
        kType_START_UPLOAD = 10,
        kType_PRE_FILE_INFO = 11,

        kType_HEART = 10086,
        kType_REC_SIZE,
        kType_GET_RECOMMEND_INFO,
    };
    enum{
      kVersion_1 = 1
    };

    enum{
        kCode_OK = 0,
        kCode_FILE_NOT_EXIST    = 2,
        kCode_MEMORY_NOT_ENOUGH = 3,
        kCode_ALREADY_START     = 4,
        kCode_PARAM_ERROR       = 5,

        kCode_WAIT              = 100 //means wait hash done
    };

    typedef struct{
        String token;
    }Login;

    typedef struct{
        String filepath; //UTF8
        String hash;
        String userdata;
    }FileInfo;

    typedef struct{
        String name;
        String hash;
        String userData;
        sint64 totalLen;
        sint64 blockSize;
        int blockCount {0};
        std::vector<int> blockIds;
    }FileStart;

    typedef struct{
        String hash;
        String userData;
        int blockId;
        String data;
    }FilePart;

    typedef struct{
        String hash;
        String userData;
    }FileEnd, GetUploadInfo, Pause;

    typedef struct {
        String filepath;
        String hash;
        String userdata;
        int code;
    }StartUploadRet;
    //----------------------------
    //--------- virtual ----------
    struct VirtualFilePart : public SkRefCnt{
        String file; //can read
        String hash;
        String userData;
        sint64 offset;
        sint64 size;
        int blockId;
    };
}

#endif // SX_PROTOCOL_H
