#include "SxServer.h"
#include "sx_protocol.h"
#include "hv/TcpServer.h"

using namespace hv;
using namespace h7;
using namespace h7_sx_pro;

bool SxServer::start(){
    TcpServer srv;
    int listenfd = srv.createsocket(m_port);
    if (listenfd < 0) {
        return false;
    }
    PRINTLN("server listen on port %d, listenfd=%d ...\n", m_port, listenfd);

    srv.onConnection = [this](const SocketChannelPtr& channel) {
        std::string peeraddr = channel->peeraddr();
        if (channel->isConnected()) {
            PRINTLN("%s connected! connfd=%d\n", peeraddr.c_str(), channel->fd());
        } else {
            PRINTLN("%s disconnected! connfd=%d\n", peeraddr.c_str(), channel->fd());
        }
        onConnectStateChanged(channel);
    };
    srv.onMessage = [this](const SocketChannelPtr& channel, Buffer* buf) {
        // echo
        onMessage(channel, buf);
    };
    srv.setThreadNum(m_thread_count);
    srv.start();
    return true;
}

void SxServer::startVerify(){
    std::thread thd([this](){
        //todo
    });
    thd.detach();
}

//---------------------------------------
void SxServer::onConnectStateChanged(const SocketChannelPtr& channel){
    String addr = channel->peeraddr();
    if(channel->isConnected()){
        m_lock_worker.lock([this, addr, channel](){
            sk_sp<SxServerWorker> worker = sk_make_sp<SxServerWorker>(channel);
            //TODO need ? worker->start();
            m_workerMap[addr] = worker;
        });
    }else{
        m_lock_worker.lock([this, addr](){
            auto it = m_workerMap.find(addr);
            if(it != m_workerMap.end()){
                m_workerMap.erase(it);
            }
        });
    }
}

void SxServer::onMessage(const hv::SocketChannelPtr& channel,hv::Buffer* buf){
    String addr = channel->peeraddr();
    sk_sp<SxServerWorker> worker;
    m_lock_worker.lock([this, &worker, addr](){
        worker = m_workerMap[addr];
    });
    if(worker){
//        std::unique_lock<std::mutex> lock(worker->m_mutex_buf);
//        worker->putMsgData(buf->data(), buf->size());
//        auto func = [channel](int type, void* ptr, int size){
//            switch (type) {
//            case kType_LOGIN:

//                break;
//            default:
//                break;
//            }
//        };
//        while (true) {
//            if(!worker->read(func)){
//                break;
//            }
//        }
    }else{
        PRINTLN("can't find worker for: %s\n", addr.data());

    }
}
