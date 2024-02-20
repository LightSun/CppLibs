#include "SxWebServer.h"
#include "hv/WebSocketServer.h"
#include "utils/string_utils.hpp"
#include "utils/FileUtils.h"
#include "utils/url_encode.h"
#include "utils/system.h"
#include "utils/_time.h"
#include "utils/CmdHelper.h"

#include "utils/ByteBuffer.hpp"
#include "ipc/ShareMemory.h"
#include "service/service_base.h"

#include <chrono>

#include "FilesSync.hpp"

using namespace hv;
using namespace h7;
using namespace h7_sx_pro::web;

void SxWebServer::startService(){
    PRINTLN("start new service.\n");
    if(m_server != nullptr){
        PRINTLN("m_server != nullptr.\n");
        return;
    }
    m_server = new WebSocketServer();
    //
    WebSocketService& ws = m_ws;
    //WebSocketService ws;
    {
        std::unique_lock<std::mutex> lock(m_mutex_cb);
        ws.onopen = [this](const WebSocketChannelPtr& channel, const HttpRequestPtr& req) {
           int old_c = h_atomic_add(&m_conn_c, 1);
           auto conn = getConn(channel, true);
           PRINTLN("ws >>(%p) onopen: GET. now_conn = %d, path = %s\n",
                   this, (old_c + 1), req->path.data());
           //mark not stopped
           h_atomic_set(&m_stopped, 0);
           doHeartBeat();
        };
        ws.onmessage = [this](const WebSocketChannelPtr& channel, const std::string& msg) {
            PRINTLN("onmessage: %.*s\n", (int)msg.size(), msg.data());
            auto conn = getConn(channel, false);
            if(conn){
                conn->processMsg(msg);
            }else{
                PRINTLN("onmessage >> no conn.\n");
            }
        };
        ws.onclose = [this](const WebSocketChannelPtr& channel) {
           int c = h_atomic_add(&m_conn_c, -1);
           auto conn = removeConn(channel);
           if(conn){
               PRINTLN("ws >> onclose. conn_id = %d, left_conn = %d\n", conn->getId(), c - 1);
               conn->disconnect();
           }else{
               PRINTLN("ws >> onclose. left_conn = %d\n", c - 1);
           }
        };
    }
    m_server->registerWebSocketService(&ws);
    m_server->setPort(m_port);
    m_server->setThreadNum(3);
    m_server->run();
}

void SxWebServer::disconnect(bool checkStop){
    PRINTLN("disconnect>> exe task count = %d，cs.connected = %d\n",
            m_uploadClient->getExeTaskCount(), m_uploadClient->isConnected());
    if(checkStop && !h_atomic_cas(&m_stopped, 0, 1)){
        return;
    }
    m_con_hb.notify_all();
    {
        std::unique_lock<std::mutex> lock(m_mutex_conn);
        auto it = m_connMap.begin();
        while (it != m_connMap.end()) {
            it->second->disconnect();
        }
        m_connMap.clear();
    }
    h_atomic_set(&m_conn_c, 0);
}

void SxWebServer::stopService(){
    if(!h_atomic_cas(&m_stopped, 0, 1)){
        return;
    }
    println("stopService --> real!\n");
    disconnect(false);
    clearCallback();
    //m_server->stop(); //~m_server will call stop
    if(m_server){
        delete m_server;
        m_server = nullptr;
    }
    m_uploadClient->setCallback(NULL);
}

void SxWebServer::setupCallback(){
    m_callback.func_connn = [this](bool connected){
        if(!connected){
            PRINTLN("cs-server not connected.\n");
            //doDestroy();
        }
    };
    m_callback.func_login=[this](int code, CString msg){
        if(code != h7_sx_pro::kCode_OK){
            sendErrorMsg("login", "msg="+msg);
        }else{
            sendRetMsg("login","msg=" + msg);
        }
    };
    m_callback.func_upload = [this](CString hash, CString str){
        sendErrorMsg("upload","hash=" + hash + ",msg=" + str);
    };
    m_callback.func_progress = [this](CString hash, int progress, int total){
        std::string proc = std::to_string(progress) + "/" + std::to_string(total);
        sendRetMsg("progress","hash=" + hash + ",progress=" + proc);
    };
}

void SxWebServer::doHeartBeat(){
    if(!h_atomic_cas(&m_heartBeatStarted, 0, 1)){
        return;
    }
    //
    std::thread thd([this](){       
        while (h_atomic_get(&m_stopped) == 0) {
             std::unique_lock<std::mutex> lock(m_mutex_hb);
             auto time = h7_handler_os::getCurTime();
             auto time_str = h7_handler_os::formatTime(time);
             int c = sendRetMsg("heart_beat","");
             if(c > 0){
                 PRINTLN("local >> send heart_beat. t = %s\n", time_str.data());
             }
             auto costTime = h7_handler_os::getCurTime() - time;
             m_con_hb.wait_for(lock, std::chrono::milliseconds(15000 - costTime));
             //hv_sleep(30);
        }       
        PRINTLN("heart_beat: done!\n");
        h_atomic_set(&m_heartBeatStarted, 0);
    });
    thd.detach();
}
void SxWebServer::clearCallback(){
    std::unique_lock<std::mutex> lock(m_mutex_cb);
    m_ws.onclose = nullptr;
    m_ws.onmessage = nullptr;
    m_ws.onopen = nullptr;
}
SxWebServer::SharedConn SxWebServer::getConn(WebSocketChannelPtr ptr, bool require){
    std::unique_lock<std::mutex> lock(m_mutex_conn);
    auto it = m_connMap.find(ptr);
    if(it == m_connMap.end()){
        if(require){
           SharedConn conn = std::make_shared<WebConnection>(m_uploadClient.get(), ptr);
           m_conn_id ++;
           conn->setId(m_conn_id);
           m_connMap[ptr] = conn;
           return conn;
        }else{
            return nullptr;
        }
    }
    return it->second;
}
SxWebServer::SharedConn SxWebServer::removeConn(WebSocketChannelPtr ptr){
    std::unique_lock<std::mutex> lock(m_mutex_conn);
    auto it = m_connMap.find(ptr);
    if(it == m_connMap.end()){
        return nullptr;
    }
    SharedConn conn = it->second;
    m_connMap.erase(it);
    return conn;
}
void SxWebServer::sendErrorMsg(const std::string& cmd,const std::string& msg){
    std::string real_msg = "error:" + cmd + "#" + msg;
    PRINTLN("SxWebServer >> (all) sendErrorMsg >> %s\n", real_msg.data());
    std::unique_lock<std::mutex> lock(m_mutex_conn);
    auto it = m_connMap.begin();
    while (it != m_connMap.end()) {
        it->second->sendDirectMsg(real_msg);
        it ++;
    }
}
int SxWebServer::sendRetMsg(const std::string& cmd, const std::string& msg){
    std::string real_msg = "ret:"+ cmd + "#" + msg;
    //PRINTLN("sendRetMsg >> %s\n", real_msg.data());
    std::unique_lock<std::mutex> lock(m_mutex_conn);
    int size = m_connMap.size();
    auto it = m_connMap.begin();
    while (it != m_connMap.end()) {
        it->second->sendDirectMsg(real_msg);
        it ++;
    }
    return size;
}



