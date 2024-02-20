#include "SxServerWorker.h"
#include "sx_protocol.h"
#include "utils/ReadHelper.h"

using namespace h7;
using namespace h7_sx_pro;

void SxServerWorker::putMsgData(const void* data, uint32 size){
    int _pos = m_buf.position();
    m_buf.put(data, size);
    m_buf.position(_pos);
}
bool SxServerWorker::read(HandleMsgCallback cb){
    int& dsize = m_dsize;
    if(m_buf.dataSize() >= sizeof(int)){
        if(dsize == 0){
            dsize = m_buf.getInt();
            MED_ASSERT(dsize > 0);
        }
        if(m_buf.dataSize() >= dsize){
            int old_pos = m_buf.position();
            int old_dataSize = m_buf.dataSize();
            //
            char* ptr = (char*)m_buf.data_ptr() + old_pos;
            int ret = read0(ptr, dsize, cb);
            {
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
uint32 SxServerWorker::read0(const void* _data, uint32 size,
                             HandleMsgCallback cb){
    char* data = (char*)(_data);
    //
    uint32 offset = 0;
    int version, type;
    version = *((int*)data);
    offset += sizeof(int);
    type = *(int*)(data + offset);
    offset += sizeof(int);

    switch (type) {
    case kType_LOGIN:{
        Login login;
        offset += readString(data + offset, login.token);
        //TODO cb(type, &login);
    }break;

    case kType_SEND_FILE_RESTART:
    case kType_SEND_FILE_START:{
        FileStart fs;
        offset += readString(data + offset, fs.name);
        offset += readString(data + offset, fs.hash);
        offset += readString(data + offset, fs.userData);
        offset += readLong(data + offset, fs.totalLen);
        offset += readLong(data + offset, fs.blockSize);
        offset += readListInt(data + offset, fs.blockIds);
        fs.blockCount = fs.blockIds.size();
        //TODO cb(type, &fs);
    }break;

    case kType_SEND_FILE_PART:{
        FilePart fs;
        offset += readString(data + offset, fs.hash);
        offset += readString(data + offset, fs.userData);
        offset += readInt(data + offset, fs.blockId);
        offset += readBytes(data + offset, fs.data);
        //cb(type, &fs);
    }break;

    case kType_SEND_FILE_END:{
        FileEnd fs;
        offset += readString(data + offset, fs.hash);
        offset += readString(data + offset, fs.userData);
        //cb(type, &fs);
    }break;

    case kType_GET_UPLOAD_INFO:{
        GetUploadInfo fs;
        offset += readString(data + offset, fs.hash);
        offset += readString(data + offset, fs.userData);
        //cb(type, &fs);
    }break;

    case kType_PAUSE:{
        Pause fs;
        offset += readString(data + offset, fs.hash);
        offset += readString(data + offset, fs.userData);
        //cb(type, &fs);
    }break;

    case kType_GET_RECOMMEND_INFO:
    case kType_HEART:{
        //do nothing
    }break;

    default:
        PRINTLN("wrong type = %d\n", type);

        MED_ASSERT(false);
    }
    return offset;
}
