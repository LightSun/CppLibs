#include "SxSender.h"
#include "hv/TcpClient.h"
#include <memory.h>

#include "utils/FileUtils.h"
#include "utils/hash.h"
#include "utils/h_atomic.h"
#include "utils/system.h"
#include "utils/Numbers.hpp"
#include "utils/RSA.h"
#include "utils/vectors.h"

using namespace h7_sx_pro;
using namespace hv;
using namespace h7;

//#define HV_MAX_WRITE_BUFSIZE           (28U << 20)  // 16M

#define SIZE_OF_BYTE sizeof(char)
#define SIZE_OF_INT sizeof(int)
#define SIZE_OF_U64 sizeof(uint64)

static std::string PUB_KEY = "-----BEGIN PUBLIC KEY-----" NEW_LINE
          "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDTQJ8u9kodlzmFwKsf1qmB/r9I" NEW_LINE
          "T8v5852pVvFvQBYf3K5i2ptdQ1RMadG/5puMX69mkrnluBQn/As9xMpeQ3h0eIz+" NEW_LINE
          "LJfWVgCrYwITksHUww/Z4XaDf/mEP6dAeyNgN55/DKajtGX+nuMbvGa836JTdkWk" NEW_LINE
          "z+TV75d8Ac5ul1iVVwIDAQAB" NEW_LINE
          "-----END PUBLIC KEY-----" NEW_LINE;

static inline uint32 _header_size(){
    //return SIZE_OF_BYTE + SIZE_OF_INT * 2;
    //size + version + type
    return SIZE_OF_INT * 3;
}

static inline uint32 _writeHeader(std::vector<char>& buf, int type){
    uint32 offset = 0;
    //*(char*)(buf.data() + offset) = endian_is_big();
    //offset += SIZE_OF_BYTE;
    //
    //all size except size_header
    *(int*)(buf.data() + offset) = buf.size() - SIZE_OF_INT;
    offset += SIZE_OF_INT;
    //version + type.
    *(int*)(buf.data() + offset) = kVersion_1;
    offset += SIZE_OF_INT;
    *(int*)(buf.data() + offset) = type;
    offset += SIZE_OF_INT;
    return offset;
}

void SxSender::sendLogin(const h7_sx_pro::Login& lg){
    std::string acc = lg.token.empty() ? "@def/med/heaven7" : lg.token;
    //auto data = RSAUtil::enc_by_public_key(PUB_KEY, h7::string2vec(acc));
    auto& data = acc;
    uint32 totalLen = _header_size() + SIZE_OF_INT + data.length();
    std::vector<char> buf(totalLen, 0);
    auto offset = _writeHeader(buf, kType_LOGIN);
    *(int*)(buf.data() + offset) = data.length();
    offset += SIZE_OF_INT;
    memcpy(buf.data() + offset, data.data(), data.length());
    //
    doSend("sendLogin", buf.data(), buf.size(), "", false);
}
void SxSender::sendFileStart(const h7_sx_pro::FileStart& lg, bool restart){
    const int len_ud = lg.userData.length();
    const int len_name = lg.name.length();
    const int len_hash = lg.hash.length();
    //byte_order + version + type.
    uint32 totalLen = _header_size()
            + SIZE_OF_INT + len_name
            + SIZE_OF_INT + len_hash
            + SIZE_OF_INT + len_ud
            + SIZE_OF_U64 * 2 + SIZE_OF_INT + SIZE_OF_INT * lg.blockCount;
    std::vector<char> buf(totalLen, 0);
    //
    uint32 offset = _writeHeader(buf, restart ? kType_SEND_FILE_RESTART
                                              : kType_SEND_FILE_START);
    //name
    memcpy(buf.data() + offset, &len_name, SIZE_OF_INT);
    offset += SIZE_OF_INT;
    memcpy(buf.data() + offset, lg.name.data(), len_name);
    offset += len_name;
    //hash
    memcpy(buf.data() + offset, &len_hash, SIZE_OF_INT);
    offset += SIZE_OF_INT;
    memcpy(buf.data() + offset, lg.hash.data(), len_hash);
    offset += len_hash;
    //ud
    memcpy(buf.data() + offset, &len_ud, SIZE_OF_INT);
    offset += SIZE_OF_INT;
    memcpy(buf.data() + offset, lg.userData.data(), len_ud);
    offset += len_ud;
    //total len + block size + block count
    memcpy(buf.data() + offset, &lg.totalLen, SIZE_OF_U64);
    offset += SIZE_OF_U64;
    memcpy(buf.data() + offset, &lg.blockSize, SIZE_OF_U64);
    offset += SIZE_OF_U64;
    memcpy(buf.data() + offset, &lg.blockCount, SIZE_OF_INT);
    offset += SIZE_OF_INT;
    //block ids
    memcpy(buf.data() + offset, lg.blockIds.data(), SIZE_OF_INT* lg.blockCount);
    doSend("sendFileStart",buf.data(), buf.size(), lg.hash, false);
}
void SxSender::sendFilePart(const h7_sx_pro::FilePart& lg){

    std::vector<char> buf;
    encodeFilePart(lg, buf);

    doSend("sendFilePart",buf.data(), buf.size(), lg.hash);
}

void SxSender::sendVirtaulFilePart(CString tag, sk_sp<h7_sx_pro::VirtualFilePart> p){
    MED_ASSERT(p);
    std::unique_lock<std::mutex> lock(m_mutex_msg);
    Share_Msg msg = Share_Msg(new Msg());
    msg->vfp = p;
    msg->hash = p->hash;
    msg->tag = tag;
    m_pendingMsgs.push_back(msg);
}

void SxSender::sendHeartBeat(){
    uint32 totalLen = _header_size();
    std::vector<char> buf(totalLen, 0);
    {
        _writeHeader(buf, kType_HEART);
    }
    doSend("=== sendHeartBeat", buf.data(), buf.size());
}
void SxSender::sendGetRecommendInfo(){
    uint32 totalLen = _header_size();
    std::vector<char> buf(totalLen, 0);
    {
        _writeHeader(buf, kType_GET_RECOMMEND_INFO);
    }
    doSend("sendGetRecommendInfo", buf.data(), buf.size());
}

void SxSender::sendFileInfo(const h7_sx_pro::FileInfo& ret){
    uint32 len_fp = ret.filepath.length();
    uint32 len_ud = ret.userdata.length();
    uint32 len_hash = ret.hash.length();

    uint32 totalLen = _header_size()
            + SIZE_OF_INT + len_fp
            + SIZE_OF_INT + len_ud
            + SIZE_OF_INT + len_hash
            ;
    std::vector<char> buf(totalLen, 0);
    {
        uint32 offset = _writeHeader(buf, kType_PRE_FILE_INFO);
        //file path
        memcpy(buf.data() + offset, &len_fp, SIZE_OF_INT);
        offset += SIZE_OF_INT;
        memcpy(buf.data() + offset, ret.filepath.data(), len_fp);
        offset += len_fp;
        //hash
        memcpy(buf.data() + offset, &len_hash, SIZE_OF_INT);
        offset += SIZE_OF_INT;
        memcpy(buf.data() + offset, ret.hash.data(), len_hash);
        offset += len_hash;
        //ud
        memcpy(buf.data() + offset, &len_ud, SIZE_OF_INT);
        offset += SIZE_OF_INT;
        memcpy(buf.data() + offset, ret.userdata.data(), len_ud);
        offset += len_ud;
    }
    doSend("sendFileInfo", buf.data(), buf.size());
}

void SxSender::sendStartUploadRet(const h7_sx_pro::StartUploadRet& ret){
    uint32 len_fp = ret.filepath.length();
    uint32 len_hash = ret.hash.length();
    uint32 len_ud = ret.userdata.length();
    uint32 totalLen = _header_size()
            + SIZE_OF_INT + len_fp
            + SIZE_OF_INT + len_hash
            + SIZE_OF_INT + len_ud
            + SIZE_OF_INT;
    std::vector<char> buf(totalLen, 0);
    {
        uint32 offset = _writeHeader(buf, kType_START_UPLOAD);
        //file path
        memcpy(buf.data() + offset, &len_fp, SIZE_OF_INT);
        offset += SIZE_OF_INT;
        memcpy(buf.data() + offset, ret.filepath.data(), len_fp);
        offset += len_fp;
        //hash
        memcpy(buf.data() + offset, &len_hash, SIZE_OF_INT);
        offset += SIZE_OF_INT;
        memcpy(buf.data() + offset, ret.hash.data(), len_hash);
        offset += len_hash;
        //ud
        memcpy(buf.data() + offset, &len_ud, SIZE_OF_INT);
        offset += SIZE_OF_INT;
        memcpy(buf.data() + offset, ret.userdata.data(), len_ud);
        offset += len_ud;
        //state
        memcpy(buf.data() + offset, &ret.code, SIZE_OF_INT);
    }
    doSend("sendStartUploadRet", buf.data(), buf.size(), "", false);
}
//----------------------------------------------------------
void SxSender::sendBaseCmd(CString hash, CString userData, CString tag,
                           int type, bool canRemove, bool toBack){
    const int len_ud = userData.length();
    const int len_hash = hash.length();
    uint32 totalLen = _header_size()
            + SIZE_OF_INT + len_hash
            + SIZE_OF_INT + len_ud;
    std::vector<char> buf(totalLen, 0);
    {
        uint32 offset = _writeHeader(buf, type);
        //hash
        memcpy(buf.data() + offset, &len_hash, SIZE_OF_INT);
        offset += SIZE_OF_INT;
        memcpy(buf.data() + offset, hash.data(), len_hash);
        offset += len_hash;
        //ud
        memcpy(buf.data() + offset, &len_ud, SIZE_OF_INT);
        offset += SIZE_OF_INT;
        memcpy(buf.data() + offset, userData.data(), len_ud);
        offset += len_ud;
    }
    doSend(tag, buf.data(), buf.size(), canRemove ? hash : "", toBack);
}

bool SxSender::doSend0(Share_Msg msg){
    //MAX_WRITE_BUFSIZE
    if(m_channel && m_channel->isConnected()){
        //check if need wait
        //PRINTLN("%s >> doSend: size = %u\n", tag.data(),size);

        String& tag = msg->tag;
        auto data = msg->data.data();
        uint32 size = msg->data.size();
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            int left_size;
            while (h_atomic_get(&m_stopped) == 0) {
                left_size = h_atomic_get(&m_left_size);
                if(int(left_size + size) > m_maxWriteBufSize){
                   // PRINTLN("... wait now...left_size = %d\n", left_size);
                    m_case.wait(lock);
                   // PRINTLN("... notify now...left_size = %d\n", h_atomic_get(&m_left_size));
                }else{
                    break;
                }
            }
        }
        //check if stopped , no need send
        if(h_atomic_get(&m_stopped) == 1){
            return true;
        }
        if(!m_channel){
            PRINTLN(">> channel not exist.\n");
            return false;
        }
        if(!m_channel->isConnected()){
            PRINTLN(">> channel not connected.\n");
            return false;
        }
        //paused
        if(h_atomic_get(&msg->paused) != 0){
            return true;
        }
        if(m_channel->isWriteComplete()){
            h_atomic_add(&m_left_size, size);
            m_channel->write(data, size);
            return true;
        }
        PRINT_W("channel can't write.\n");
    }else{
        PRINTERR("%s >> send data failed.\n", msg->tag.data());
    }
    return false;
}

//--------------------------------
void SxSender::encodeFilePart(const h7_sx_pro::FilePart& lg, std::vector<char>& buf){
    const int len_ud = lg.userData.length();
    const int len_data = lg.data.length();
    const int len_hash = lg.hash.length();
    //
    uint64 totalLen = _header_size()
            + SIZE_OF_INT + len_hash
            + SIZE_OF_INT + len_ud
            + SIZE_OF_INT
            + SIZE_OF_INT + len_data;
    buf.resize(totalLen, 0);
    //std::vector<char> buf(totalLen, 0);
    {
        uint32 offset = _writeHeader(buf, kType_SEND_FILE_PART);
        //hash
        memcpy(buf.data() + offset, &len_hash, SIZE_OF_INT);
        offset += SIZE_OF_INT;
        memcpy(buf.data() + offset, lg.hash.data(), len_hash);
        offset += len_hash;
        //ud
        memcpy(buf.data() + offset, &len_ud, SIZE_OF_INT);
        offset += SIZE_OF_INT;
        memcpy(buf.data() + offset, lg.userData.data(), len_ud);
        offset += len_ud;
        //bid
        memcpy(buf.data() + offset, &lg.blockId, SIZE_OF_INT);
        offset += SIZE_OF_INT;
        //file len + data
        memcpy(buf.data() + offset, &len_data, SIZE_OF_INT);
        offset += SIZE_OF_INT;
        memcpy(buf.data() + offset, lg.data.data(), len_data);
    }
    //doSend("sendFilePart",buf.data(), buf.size(), lg.hash);
}
String SxSender::encodeData(sk_sp<h7_sx_pro::VirtualFilePart> p){
    if(!FileUtils::isFileExists(p->file)){
        PRINTLN("file not exist. %s\n", p->file.data());
        return "";
    }
    std::vector<char> buf;
    {
     FilePart fs;
     fs.data = FileUtils::getFileContent(p->file, p->offset, p->size);
     if(fs.data.length() != p->size){
         LOGW("encodeData >> p->size == fs.data.length(). %ll != %lu\n",
              p->size, fs.data.length());
     }
     fs.blockId = p->blockId;
     fs.hash = p->hash;
     fs.userData = p->userData;
     //
     encodeFilePart(fs, buf);
    }
    return String(buf.data(), buf.size());
}


void SxSender::startLoopMsg(){
    PRINTLN("startLoopMsg >> start\n");
    std::thread thd([this](){
        Share_Msg msg;
        uint64 last_failed_hash = 0;
        uint32 failed_c = 0;
        while (h_atomic_get(&m_stopped) == 0) {
            msg = pollNextMsg();
            if(!msg){
                continue;
            }
            //for empty, need read data.
            if(msg->data.empty()){
                MED_ASSERT(msg->vfp);
                msg->data = encodeData(msg->vfp);
            }
            //may file not exist.
            if(msg->data.empty()){
                if(msg->vfp){
                    Msg_cb mc = {
                        .hash = msg->hash,
                        .code = kTYPE_cb_FAILED
                    };
                    m_cbMsgs.push(msg->tag, mc);
                    m_cbMsgs.notify();
                }
                continue;
            }
            //ignore paused
            if(h_atomic_get(&msg->paused) != 0){
                continue;
            }
            if(!doSend0(msg)){
                pushFront(msg);
                auto hash = fasthash64(msg->data.data(), msg->data.length(), 11);
                if(hash == last_failed_hash){
                    failed_c ++;
                    PRINTLN("resend msg(%s) failed. count = %u\n", msg->tag.data(), failed_c);
                }else{
                    last_failed_hash = hash;
                    failed_c = 0;
                }
                hv_msleep(30);
            }else{
                if(msg->vfp){
                    Msg_cb mc = {
                        .hash = msg->hash,
                        .code = kTYPE_cb_OK
                    };
                    m_cbMsgs.push(msg->tag, mc);
                    m_cbMsgs.notify();
                }
            }
        }
        PRINTLN("startLoopMsg >> main msgs end\n");
    });
    thd.detach();

    //loop cb msgs
    std::thread thd2([this](){
        QMsgCb mcb;
        while (h_atomic_get(&m_stopped) == 0) {
            if(m_cbMsgs.pop(&mcb)){
                m_cb.onSendResult(mcb.data.hash, mcb.data.code);
            }else{
                m_cbMsgs.wait();
            }
        }
        PRINTLN("startLoopMsg >> cb msgs end\n");
    });
    thd2.detach();
}

void SxSender::removeMsgs(CString hash){
    std::unique_lock<std::mutex> lock(m_mutex_msg);
    //copy to vector and travel then copy return to link-list
    std::vector<Share_Msg> vec{std::begin(m_pendingMsgs), std::end(m_pendingMsgs)};
    int size = vec.size();
    for(int i = size - 1 ; i >= 0 ; i--){
        if(vec[i]->hash == hash){
            //pause and remove
            h_atomic_set(&vec[i]->paused, 1);
            vec.erase(vec.begin() + i);
        }
    }
    m_pendingMsgs.clear();
    m_pendingMsgs.assign(std::begin(vec), std::end(vec));
}


