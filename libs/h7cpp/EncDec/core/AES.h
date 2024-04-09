#pragma once

#include <thread>
#include "DataBlock.h"

namespace med {

class AESEncDec;

/**
 * @brief The AES class
 * AES加密算法，包括AES下多种加密模式的实现，底层调用openssl。
 */
class AES
{
public:
    AES(int tc, size_t maxLen):m_tc(tc), m_maxLen(maxLen){}
    AES(size_t maxLen):m_maxLen(maxLen){
        m_tc = std::thread::hardware_concurrency();
    }
    int getMaxThreadCount(){return m_tc;}

    String ecb(CString in, CString key, bool enc);
    String cbc(CString in, CString key, CString ivec, bool enc);
    String cfb1(CString in, CString key, CString ivec, bool enc);
    String cfb8(CString in, CString key, CString ivec, bool enc);
    String cfb128(CString in, CString key, CString ivec, bool enc);
    String ofb128(CString in, CString key, CString ivec, bool enc);

private:
    void doEncode(CString key, CString ivec, CString data, DataBlock* out,
                  AESEncDec* impl);
    void doDecode(CString key, CString ivec, DataBlock* out,
                  bool verify, AESEncDec* impl);
    String doExec(CString tag, CString in, CString key, CString ivec,
                  bool enc, AESEncDec* impl);

private:
    int m_tc;
    size_t m_maxLen {1 << 20};
};

}
