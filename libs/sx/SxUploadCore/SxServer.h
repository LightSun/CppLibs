#ifndef SXSERVER_H
#define SXSERVER_H

#include "common/common.h"
#include "common/c_common.h"
#include "common/SkRefCnt.h"
#include "utils/Lock.hpp"
#include "utils/ByteBuffer.hpp"
#include "hv/TcpServer.h"

#include "SxServerWorker.h"

namespace h7 {

class SxServer: public SkRefCnt
{
public:
    SxServer(int port, int tc):m_port(port), m_thread_count(tc){
    }

    bool start();
    void startVerify();

private:
    int m_port;
    int m_thread_count;
    std::map<String, sk_sp<SxServerWorker>> m_workerMap;
    Lock m_lock_worker;

    void onConnectStateChanged(const hv::SocketChannelPtr& channel);
    void onMessage(const hv::SocketChannelPtr& channel, hv::Buffer* buf);
};

}

#endif // SXSERVER_H
