#ifndef SXSERVERWORKER_H
#define SXSERVERWORKER_H

#include "common/common.h"
#include "common/c_common.h"
#include "common/SkRefCnt.h"
#include "hv/TcpServer.h"
#include "utils/ByteBuffer.hpp"

namespace h7 {

class SxServerWorker : public SkRefCnt{
public:

    typedef std::function<void(int type, void* ptr, int rawSize)> HandleMsgCallback;
    SxServerWorker(hv::SocketChannelPtr channel):m_channel(channel){

    }
   // void start();
    void putMsgData(const void* data, uint32 size);

    bool read(HandleMsgCallback cb);

public:
     std::mutex m_mutex_buf;
private:
    std::atomic_bool m_connected {false};
    int m_dsize {0};  //for every msg from client

    ByteBuffer m_buf {20 << 20};//20M
    hv::SocketChannelPtr m_channel;

    uint32 read0(const void* data, uint32 size, HandleMsgCallback cb);
};

}

#endif // SXSERVERWORKER_H
