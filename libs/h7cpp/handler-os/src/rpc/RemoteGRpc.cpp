#include "RemoteGRpc.h"
#ifdef CS_CORE_DIR
#include "cs_core_client_api.h"
#endif

using namespace h7_task;

RemoteGRpc::RemoteGRpc(CString addr, size_t timeOutMs):m_serverAddr(addr)
{
#ifdef CS_CORE_DIR
    m_client = h7::grpc_client_create(addr);
    h7::grpc_client_setTimeout(m_client, timeOutMs);
#else
    (void)timeOutMs;
#endif
}
RemoteGRpc::~RemoteGRpc(){
#ifdef CS_CORE_DIR
    if(m_client){
        h7::grpc_client_destroy(m_client);
        m_client = nullptr;
    }
#endif
}

bool RemoteGRpc::invoke(int code, CString inData, String* outData){
#ifdef CS_CORE_DIR
    h7::CsCore_Msg m_res;
    h7::CsCore_Msg msg;
    m_res.code = kCode_UNKNOWN;
    msg.flags = 0;
    msg.code = code;
    msg.data = inData;
    bool ret = h7::grpc_client_call(m_client, &msg, &m_res);
    auto state = ret && m_res.code == kCode_OK;
    if(state && outData){
        *outData = std::move(m_res.data);
    }
    return state;
#endif
    return false;
}
//--------------------------------

