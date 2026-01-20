#pragma once

#include <vector>
#include <string>
#include "handler-os/rpc_api.h"

namespace h7 {
    typedef void* ClientPtr;
}

namespace h7_task {

using String = std::string;
using CString = const String&;
template<typename T>
using List = std::vector<T>;

class RemoteGRpc: public IRemoteGRpc
{
public:
    RemoteGRpc(CString addr, size_t timeOutMs = 500);
    ~RemoteGRpc();

    String& getServerAddr(){return m_serverAddr;}
    //------------
public:
    bool invoke(int code, CString inData, String* outData) override;

private:
    RemoteGRpc(const RemoteGRpc& src) = delete;
    RemoteGRpc(RemoteGRpc&& src) = delete;
    RemoteGRpc& operator=(const RemoteGRpc& src) = delete;
    RemoteGRpc& operator=(RemoteGRpc&& src) = delete;

private:
    h7::ClientPtr m_client {nullptr};
    String m_serverAddr;
};

}
