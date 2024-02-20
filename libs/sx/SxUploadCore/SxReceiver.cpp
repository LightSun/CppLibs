#include "SxReceiver.h"
#include "sx_protocol.h"
#include "utils/ReadHelper.h"

using namespace h7;

uint32 SxReceiver::read0(const void* _data, uint32 size){
    char* data = (char*)_data;
    uint32 offset = 0;
    //
    offset += readInt(data, m_reqType);
    switch (m_reqType) {
    case h7_sx_pro::kType_LOGIN:{
        PRINTLN("=========== kType_LOGIN > \n");
        offset += readInt(data + offset, m_state.code);
        offset += readString(data + offset, m_state.msg);
    }break;

    case h7_sx_pro::kType_GET_UPLOAD_INFO:{
        PRINTLN("=========== kType_GET_UPLOAD_INFO > \n");
        offset += readGetUploadInfo(data + offset, m_uploadedInfo);
    }break;

    case h7_sx_pro::kType_HEART:{
        //nothing.
        //PRINTLN("=========== kType_HEART > \n");
    }break;

    case h7_sx_pro::kType_REC_SIZE:{
        //PRINTLN("=========== kType_REC_SIZE > \n");
        int real_type, size;
        offset += readInt(data + offset, real_type);
        offset += readInt(data + offset, size);
        m_recInfo.type = real_type;
        m_recInfo.size = size;
    }break;

    case h7_sx_pro::kType_GET_RECOMMEND_INFO:{
        PRINTLN("=========== kType_GET_RECOMMEND_INFO > \n");
        offset += readInt(data + offset, m_recommendInfo.maxBlockSize);
    }break;

    case h7_sx_pro::kType_UPLOAD_OK:{
        offset += readString(data + offset, m_fileHash);
        PRINTLN("=========== kType_UPLOAD_OK > hash = %s\n", m_fileHash.data());
        //
    }break;

    case h7_sx_pro::kType_STOP_UPLOAD:{
        offset += readString(data + offset, m_fileHash);
        offset += readString(data + offset, m_ud);
        PRINTLN("=========== kType_STOP_UPLOAD > hash = %s, ud = %s\n",
                m_fileHash.data(), m_ud.data());
    }break;

    case h7_sx_pro::kType_START_UPLOAD:{
        offset += readString(data + offset, m_filePath);
        offset += readString(data + offset, m_ud);
        offset += readGetUploadInfo(data + offset, m_uploadedInfo);
        PRINTLN("=========== kType_START_UPLOAD > hash = %s.\n",
                m_uploadedInfo.hash.data());
    }break;

    case h7_sx_pro::kType_PRE_FILE_INFO:{
        PRINTLN("=========== kType_PRE_FILE_INFO > \n");
    }break;

    default:
        PRINTERR("wrong req: %d\n", m_reqType);
        break;
    }
    return offset;
}

bool SxReceiver::read(){
    //
    int& dsize = m_dsize;
    if(m_buf.dataSize() >= (int)sizeof(int)){
        if(dsize == 0){
            dsize = m_buf.getInt();
            MED_ASSERT(dsize > 0);
        }
        if(m_buf.dataSize() >= dsize){
           // PRINTLN("read: data size = %d\n", dsize);
            int old_pos = m_buf.position();
            int old_dataSize = m_buf.dataSize();
            //
            int reqType = m_buf.getInt();
            m_buf.position(old_pos);
            //
            char* ptr = (char*)m_buf.data_ptr() + old_pos;
            int ret = read0(ptr, dsize);
            {
                if(ret != dsize){
                    //for test write data to file:

                    PRINTLN("--> Error: read ret = %d, dsize = %d, reqType = %d\n",
                            ret, dsize, reqType);
                }
                char _buf[128];
                snprintf(_buf, 128, "read data_size(%d) must = dsize(%d).",
                         ret, dsize);
                String msg = _buf;
                MED_ASSERT_X(ret == dsize, msg);
            }
            m_buf.position(old_pos + dsize);
            m_buf.dataSize(old_dataSize - dsize);
            dsize = 0;
            return true;
        }
    }
    return false;
}

uint32 SxReceiver::readGetUploadInfo(char* data, UploadedInfo& info){
    info.reset();
    uint32 offset = 0;
    offset += readString(data + offset, info.hash);
    bool hasRecord = *(char*)(data + offset) == 1;
    offset += sizeof(char);
    if(hasRecord){
        offset += readLong(data + offset, info.blockSize);
        offset += readLong(data + offset, info.totalLen);
        offset += readListInt((char*)(data + offset), info.blockIds);
        offset += readListInt((char*)(data + offset), info.finishBlockIds);
    }else{
        info.blockIds.clear();
        info.finishBlockIds.clear();
        info.blockSize = 0;
        info.totalLen = 0;
    }
    return offset;
}
