#pragma once

#include <string>

namespace h7_task {

using String = std::string;
using CString = const std::string&;

class IRemoteGRpc{
public:
    virtual ~IRemoteGRpc(){}

    virtual bool invoke(int code, CString inData, String* outData) = 0;
};

class IRpcCacheDelegate{
public:
    virtual ~IRpcCacheDelegate(){}
    virtual IRemoteGRpc* get(CString addr, size_t timeoutMs) = 0;
};

}
