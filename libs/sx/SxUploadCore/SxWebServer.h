#ifndef SXWEBSOCKET_H
#define SXWEBSOCKET_H

#include "hv/WebSocketServer.h"
#include "SxClient.h"
#include "sx_web_protocol.h"
#include "WebConnection.h"

//#define USE_UI_UPLOAD_INTERNAL 1

namespace med_api{
    class ShowUploadHelper;
};

namespace h7 {

#define MED_WSC_LIMIT_CHAR "::"

//as websocket server
class SxWebServer : public SkRefCnt
{
public:
    using SharedConn = std::shared_ptr<WebConnection>;
    SxWebServer(int web_port = 21353)
        :m_port(web_port){
        setupCallback();
    }
    ~SxWebServer(){
        PRINTLN(">> ~SxWebServer .\n");
    }

    void setUploadClient(sk_sp<SxClient> client){
        m_uploadClient = client;
        m_uploadClient->setCallback(&m_callback);
    }
    void setShowUploadHelper(med_api::ShowUploadHelper* h){
        m_uiHelper = h;
    }

    void startService();
    void stopService();
    void disconnect(bool checkStop = true);

    void doDestroy(){
        SxWebServer* ins = this;
        ins->ref();
        std::thread thd([ins](){
             ins->stopService();
             ins->unref();
        });
        thd.detach();
    }

private:
     med_api::ShowUploadHelper* m_uiHelper{nullptr};
     hv::WebSocketServer* m_server{nullptr};
     hv::WebSocketService m_ws;
     sk_sp<SxClient> m_uploadClient;
     SxClient::Callback m_callback;
     volatile int m_stopped {0};
     volatile int m_conn_c {0}; //the connect count
     volatile int m_heartBeatStarted {0}; //
     int m_conn_id {0};
     int m_port;

     std::map<WebSocketChannelPtr, SharedConn> m_connMap;
     std::mutex m_mutex_conn;

     std::mutex m_mutex_cb;

     std::mutex m_mutex_hb;//heart beat
     std::condition_variable m_con_hb;
     //
     void setupCallback();
     void processMsg(const std::string& msg);

     SharedConn getConn(WebSocketChannelPtr ptr, bool require);
     SharedConn removeConn(WebSocketChannelPtr ptr);
    //send msg to all conn
     void sendErrorMsg(const std::string& cmd,const std::string& msg);
     int sendRetMsg(const std::string& cmd, const std::string& msg);

     void doHeartBeat();
     void clearCallback();
};

}

#endif // SXWEBSOCKET_H
