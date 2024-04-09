#pragma once

#include <functional>
#include <thread>

#include "DataBlock.h"

namespace med {

class MedRSA
{
public:
    MedRSA(int tc, size_t maxLen):m_tc(tc), m_maxLen(maxLen){}
    MedRSA(size_t maxLen):m_maxLen(maxLen){
        m_tc = std::thread::hardware_concurrency();
    }
    int getMaxThreadCount(){return m_tc;}

    String encByPubKey(CString key, CString data);
    String decByPriKey(CString key, CString data, bool verify = true);

    String decByPubKey(CString key, CString data, bool verify = true);
    String encByPriKey(CString key, CString data);

public:
    static std::string encByPublicKey(CString publicKey, CString data);
    static std::string decByPublicKey(CString publicKey, CString data);
    static std::string encByPrivateKey(CString privateKey, CString data);
    static std::string decByPrivateKey(CString privateKey, CString data);

private:
    static std::string encode0(void* rsa, bool pub, CString data);
    static std::string decode0(void* rsa, bool pub, CString data);

private:
    void doEncode(void* rsa, bool pub, CString data, DataBlock* out);
    void doDecode(void* rsa, bool pub, DataBlock* out, bool verify);

private:
    int m_tc;
    size_t m_maxLen {1 << 20};
};

}
