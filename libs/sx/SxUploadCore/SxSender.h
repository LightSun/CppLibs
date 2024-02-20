#ifndef SXSENDER_H
#define SXSENDER_H

#include <condition_variable>
#include <mutex>
#include <list>
#include "sx_protocol.h"
#include "hv/TcpClient.h"
#include "utils/h_atomic.h"
#include "utils/Executor.h"
#include "utils/hash.h"
#include "utils/Queue.hpp"

namespace h7 {

class SxSender{
public:
    enum{
        kTYPE_cb_OK = 1,
        kTYPE_cb_FAILED,
    };

    struct Callback{
        std::function<void(CString hash, int code)> onSendResult;
    };

    ~SxSender(){
        if (h_atomic_get(&m_stopped) == 0){
            stopRec();
        }
    }
    void setCallback(Callback* cb){
        m_cb.onSendResult = std::move(cb->onSendResult);
    }
    //front
    void sendLogin(const h7_sx_pro::Login& lg);
    //front
    void sendFileStart(const h7_sx_pro::FileStart& lg, bool restart = false);

    void sendFilePart(const h7_sx_pro::FilePart& lg);
    void sendVirtaulFilePart(CString tag, sk_sp<h7_sx_pro::VirtualFilePart> p);

    void sendFileEnd(const h7_sx_pro::FileEnd& lg){
        sendBaseCmd(lg.hash, lg.userData, "sendFileEnd",
                    h7_sx_pro::kType_SEND_FILE_END, true, true);
    }
    //front
    void sendGetUploadInfo(const h7_sx_pro::GetUploadInfo& lg){
        sendBaseCmd(lg.hash, lg.userData, "sendGetUploadInfo",
                    h7_sx_pro::kType_GET_UPLOAD_INFO, false, false);
    }
    //front
    void sendPause(const h7_sx_pro::Pause& lg){
        sendBaseCmd(lg.hash, lg.userData, "sendPause",
                    h7_sx_pro::kType_PAUSE, true, false);
    }
    //front
    void sendStartUploadRet(const h7_sx_pro::StartUploadRet& ret);

    void sendHeartBeat();

    void sendGetRecommendInfo();

    void sendFileInfo(const h7_sx_pro::FileInfo& info);

    void stopRec(){
        clearMsg();
        h_atomic_set(&m_stopped, 1);
        m_case.notify_all();
        m_cbMsgs.notify_all();
    }
    void startRec(){
        h_atomic_set(&m_left_size, 0);
        h_atomic_set(&m_stopped, 0);
        startLoopMsg();
    }

    void reportRecSize(int size){
        //PRINTLN("reportRecSize: %d\n", size);
        //int old =
        h_atomic_add(&m_left_size, -size);
        m_case.notify_one();
        PRINTLN("left msg_count = %d\n", getPoolMsgSize());
    }

    void setMaxWriteBufSize(int size){
        MED_ASSERT(size > 0);
        m_maxWriteBufSize = size;
    }

    int getPoolMsgSize(){
         std::unique_lock<std::mutex> lock(m_mutex_msg);
         return m_pendingMsgs.size();
    }
    void setChannel(hv::SocketChannelPtr ptr){
        m_channel = ptr;
    }
    void removeMsgs(CString hash);

private:
    struct Msg
    {
        String tag;
        String data;
        String hash;
        volatile int paused {0};
        sk_sp<h7_sx_pro::VirtualFilePart> vfp;
    };
    struct Msg_cb{
        String hash;
        int code;
    };
    using QMsgCb = h7::Queue<Msg_cb>::Msg<Msg_cb>;
    using Share_Msg = std::shared_ptr<Msg>;

    Callback m_cb;

    hv::SocketChannelPtr m_channel;
    //the left size of server need rec
    volatile int m_left_size {0};
    std::mutex m_mutex;
    std::condition_variable m_case;
    int m_maxWriteBufSize {8 << 20};
    volatile int m_stopped {0};
    //
    std::list<Share_Msg> m_pendingMsgs;
    std::mutex m_mutex_msg;

    h7::Queue<Msg_cb> m_cbMsgs;

private:

    void startLoopMsg();
    String encodeData(sk_sp<h7_sx_pro::VirtualFilePart> p);
    void encodeFilePart(const h7_sx_pro::FilePart& lg, std::vector<char>& buf);

    void clearMsg(){
        std::unique_lock<std::mutex> lock(m_mutex_msg);
        PRINTLN("clearMsg >> message count = %d\n", m_pendingMsgs.size());
        m_pendingMsgs.clear();
    }

    void pushFront(Share_Msg msg){
        std::unique_lock<std::mutex> lock(m_mutex_msg);
        m_pendingMsgs.push_front(msg);
    }

    void doSendAsync(CString tag, const void* data, uint32 size,
                     CString hash, bool toBack){
        {
        std::unique_lock<std::mutex> lock(m_mutex_msg);
        Share_Msg msg = Share_Msg(new Msg());
        msg->tag = tag;
        msg->hash = hash;
        msg->data = String((char*)data, size);
        if(toBack){
            m_pendingMsgs.push_back(msg);
        }else{
            m_pendingMsgs.push_front(msg);
        }
        }
    }
    Share_Msg pollNextMsg(){
        std::unique_lock<std::mutex> lock(m_mutex_msg);
        if(m_pendingMsgs.size() == 0){
            return nullptr;
        }
        auto msg = m_pendingMsgs.front();
        m_pendingMsgs.pop_front();
        return msg;
    }

    void doSend(CString tag, const void* data, uint32 size,
                CString hash = "",  bool back = true){
        //doSend0(tag, data, size);
        doSendAsync(tag, data, size, hash, back);
        //PRINTLN("%s >> add data to pool. size = %u\n", tag.data(), size);
    }
    bool doSend0(Share_Msg msg);

    //toBack: true to back, false to front
    void sendBaseCmd(CString hash, CString ud, CString tag,
                     int type, bool canRemove, bool toBack = true);
};

}

#endif // SXSENDER_H
