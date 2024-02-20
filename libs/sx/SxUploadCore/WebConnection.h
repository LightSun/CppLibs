#pragma once

#include "hv/WebSocketServer.h"
#include "SxClient.h"
#include "sx_web_protocol.h"

#include "ipc/ShareMemory.h"

namespace h7 {

class WebConnection
{

public:
    WebConnection(SxClient* client, WebSocketChannelPtr channel);

    void processMsg(const std::string& str);
    void disconnect();
    bool isConnected();

    void sendDirectMsg(CString msg);

    void setId(int id){
        m_id = id;
    }
    int getId(){
        return m_id;
    }

private:
    void doHeartBeat();
    void sendErrorMsg(const std::string& cmd,const std::string& msg);
    void sendRetMsg(const std::string& cmd,const std::string& msg);
    void processMsg0(const std::string& str);

    void doSelectFileFromCmd();
    void doSelectFileFromAnotherProcess(const std::vector<String>& vec);
    void doSelectFileFromAnotherProcess0(CString appPath, CString ud, CString params);

    void doSelectFile(const std::vector<String>& vec);

    void onPostSelectFile(CString ud, CString encs);

private:
    SxClient* m_uploadClient {nullptr};
    WebSocketChannelPtr m_channel;
    volatile int m_stopped {0};
    int m_id {0};
};


}
