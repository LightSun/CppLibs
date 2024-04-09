#include "MedRSA.h"

#include <memory.h>
#include <iostream>

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

#include "ThreadPool.h"
#include "hash.h"
#include "ByteBufferIO.h"

using namespace med;

struct RSAHolder{
    RSA* rsa;
    RSAHolder(RSA* rsa):rsa(rsa){}
    ~RSAHolder(){
        if(rsa){
            RSA_free(rsa);
            rsa = nullptr;
        }
    }
};

static inline RSA* create_RSA(const void* key, bool pub){
    BIO* in = BIO_new_mem_buf(key, -1);

    if (in == NULL) {
        std::cout << "create BIO failed !" << std::endl;
        return NULL;
    }
    RSA* rsa;
    if(pub){
        rsa = PEM_read_bio_RSA_PUBKEY(in, NULL, NULL, NULL);
    }else{
        rsa = PEM_read_bio_RSAPrivateKey(in, NULL, NULL, NULL);
    }
    BIO_free(in);
    if (rsa == NULL) {
        std::cout << "create RSA failed !" << std::endl;
        return NULL;
    }
    return rsa;
}

std::string MedRSA::encByPublicKey(CString publicKey,CString data){
    RSA* rsa = create_RSA(publicKey.data(), true);
    if(rsa == nullptr) return "";
    RSAHolder rh(rsa);
    return encode0(rh.rsa, true, data);
}
std::string MedRSA::decByPublicKey(CString publicKey,
                                    CString data){
    RSA* rsa = create_RSA(publicKey.data(), true);
    if(rsa == nullptr) return "";
    RSAHolder rh(rsa);
    return decode0(rh.rsa, true, data);
}
std::string MedRSA::encByPrivateKey(CString  privateKey,
                                     CString data){
    RSA* rsa = create_RSA(privateKey.data(), false);
    if(rsa == nullptr) return "";
    RSAHolder rh(rsa);
    return encode0(rh.rsa, false, data);
}
std::string MedRSA::decByPrivateKey(CString  privateKey,
                                     CString data){
    RSA* rsa = create_RSA(privateKey.data(), false);
    if(rsa == nullptr) return "";
    RSAHolder rh(rsa);
    return decode0(rh.rsa, false, data);
}
std::string MedRSA::encode0(void* _rsa, bool pub, CString data){
    std::string strRet;
    strRet.reserve(data.size());

    RSA* rsa = (RSA*)_rsa;
    int len = RSA_size(rsa);

    String decText;
    decText.resize(len);

    int dataLen = data.size();

    int blockLen = len - 11;
    //分段次数
    int nCount = (dataLen / blockLen) + 1;
    unsigned char* dataPtr = (unsigned char*)data.data();
    //分段加密
    for (int i = 0; i < nCount; i++)
    {
        int nSize = 0;
        blockLen = (dataLen > blockLen) ? blockLen : dataLen;
        if(pub){
            nSize = RSA_public_encrypt(blockLen, (const unsigned char*)dataPtr,
                               (unsigned char*)decText.data(), rsa, RSA_PKCS1_PADDING);
        }else{
            nSize = RSA_private_encrypt(blockLen, (const unsigned char*)dataPtr,
                               (unsigned char*)decText.data(), rsa, RSA_PKCS1_PADDING);
        }
        dataPtr += blockLen;
        dataLen -= blockLen;

        if (nSize >= 0){
            strRet += std::string(decText.data(), nSize);
        }
    }
    return strRet;
}
std::string MedRSA::decode0(void* _rsa, bool pub, CString data){
    std::string strRet;
    strRet.reserve(data.size());
    ///
    RSA* rsa = (RSA*)_rsa;

    int len = RSA_size(rsa);
    String decText;
    decText.resize(len);

    int dataLen = data.size();

    int pdBlock = len;
    //分段次数
    int nCount = (dataLen / pdBlock) + 1;
    unsigned char* dataPtr = (unsigned char*)data.data();
    //分段解密
    for (int i = 0; i < nCount; i++)
    {
        int nSize = 0;

        pdBlock = (dataLen > pdBlock) ? pdBlock : dataLen;
        if(pub){
            nSize = RSA_public_decrypt(pdBlock, (const unsigned char*)dataPtr,
                           (unsigned char*)decText.data(), rsa, RSA_PKCS1_PADDING);
        }else{
            nSize = RSA_private_decrypt(pdBlock, (const unsigned char*)dataPtr,
                           (unsigned char*)decText.data(), rsa, RSA_PKCS1_PADDING);
        }
        dataPtr += pdBlock;
        dataLen -= pdBlock;
        if (nSize >= 0){
            strRet += std::string(decText.data(), nSize);
        }
    }
    return strRet;
}
//-----------------------------
void MedRSA::doEncode(void* rsa, bool pub, CString data, DataBlock* out){
    MED_ASSERT(!data.empty());
    out->version = DB_VERSION;
    if(data.length() > m_maxLen){
        List<String> splitDatas;
        auto len = data.length();
        for(size_t i = 0 ; i < len; i += m_maxLen){
            size_t subLen = len - i < m_maxLen ? len - i : m_maxLen;
            splitDatas.push_back(data.substr(i, subLen));
        }
        out->setBlockCount(splitDatas.size());
        h7::ThreadPool::batchRawRun(m_tc, 0, out->blockCount,
                                    [rsa, pub, &splitDatas, out](int di){
            out->blockHashes[di] = fasthash64(splitDatas[di].data(),
                            splitDatas[di].length(), DEF_HASH_SEED);
            auto ret = encode0(rsa, pub, splitDatas[di]);
            out->setEncodeData(di, ret);
            return true;
        });
    }else{
        out->setBlockCount(1);
        out->blockHashes[0] = fasthash64(data.data(), data.length(), DEF_HASH_SEED);
        auto ret = encode0(rsa, pub, data);
        out->setEncodeData(0, ret);
    }
}
void MedRSA::doDecode(void* rsa, bool pub, DataBlock* out, bool verify){
    MED_ASSERT(out->blockCount > 0);
    if(out->blockCount > 1){
        h7::ThreadPool::batchRawRun(m_tc, 0, out->blockCount,
                    [out, rsa, pub, verify](int di){
            auto ret = decode0(rsa, pub, out->blockDatas[di]);
            out->setDecodeData(di, ret, verify);
            return true;
        });
    }else{
        auto ret = decode0(rsa, pub, out->blockDatas[0]);
        out->setDecodeData(0, ret, verify);
    }
}
//
String MedRSA::encByPubKey(CString key, CString data){
    RSA* rsa = create_RSA(key.data(), true);
    if(rsa == nullptr) return "";
    RSAHolder rh(rsa);
    DataBlock block;
    doEncode(rh.rsa, true, data, &block);
    return block.toString();
}
String MedRSA::decByPriKey(CString key, CString data, bool verify){
    RSA* rsa = create_RSA(key.data(), false);
    if(rsa == nullptr) return "";
    RSAHolder rh(rsa);
    DataBlock block;
    block.fromString(data, verify);
    doDecode(rh.rsa, false, &block, verify);
    if(block.isVerifyOk()){
        return block.getDecodeResult();
    }
    fprintf(stderr, "decByPriKey >> hash-verify failed.\n");
    return "";
}

String MedRSA::decByPubKey(CString key, CString data, bool verify){
    RSA* rsa = create_RSA(key.data(), true);
    if(rsa == nullptr) return "";
    RSAHolder rh(rsa);
    DataBlock block;
    block.fromString(data, verify);
    doDecode(rh.rsa, true, &block, verify);
    if(block.isVerifyOk()){
        return block.getDecodeResult();
    }
    fprintf(stderr, "decByPubKey >> hash-verify failed.\n");
    return "";
}
String MedRSA::encByPriKey(CString key, CString data){
    RSA* rsa = create_RSA(key.data(), false);
    if(rsa == nullptr) return "";
    RSAHolder rh(rsa);
    DataBlock block;
    doEncode(rh.rsa, false, data, &block);
    return block.toString();
}
