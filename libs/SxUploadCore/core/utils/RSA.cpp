#include "RSA.h"
#include <memory.h>
#include <iostream>

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

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

/*
 * 公钥加密
 */
std::string RSAUtil::enc_by_public_key(const std::vector<char>& publicKey, const std::vector<char>& data)
{
    std::string strRet;
    strRet.reserve(data.size());
    ///创建RSA指针
    RSA* rsa = create_RSA(publicKey.data(), true);
    if(rsa == NULL){
        return "";
    }

    int len = RSA_size(rsa);

    char* decryptedText = (char*)malloc(len + 1);
    memset(decryptedText, 0, len + 1);

    int nClearDataLen = data.size();

    int pdBlock = len - 11;
    int nCount = (nClearDataLen / pdBlock) + 1;//分段次数
    unsigned char* pClearData = (unsigned char*)data.data();
    //分段加密
    for (int i = 0; i < nCount; i++)
    {
        int nSize = 0;
        pdBlock = (nClearDataLen > pdBlock) ? pdBlock : nClearDataLen;
        nSize = RSA_public_encrypt(pdBlock, (const unsigned char*)pClearData,
                                   (unsigned char*)decryptedText, rsa, RSA_PKCS1_PADDING);
        pClearData += pdBlock;
        nClearDataLen -= pdBlock;

        if (nSize >= 0)
        {
            strRet += std::string(decryptedText, nSize);
        }
    }

    // 释放内存
    free(decryptedText);
    RSA_free(rsa);
    return strRet;
}
/*
 * 公钥解密
 */
std::string RSAUtil::dec_by_public_key(const std::vector<char>& publicKey, const std::vector<char>& data)
{
    std::string strRet;
    strRet.reserve(data.size());
    ///创建RSA指针
    RSA* rsa = create_RSA(publicKey.data(), true);
    if(rsa == NULL){
        return "";
    }

    int len = RSA_size(rsa);
    char* decryptedText = (char*)malloc(len + 1);
    memset(decryptedText, 0, len + 1);

    int nClearDataLen = data.size();

    int pdBlock = len;
    int nCount = (nClearDataLen / pdBlock) + 1;//分段次数
    unsigned char* pClearData = (unsigned char*)data.data();
    //分段解密
    for (int i = 0; i < nCount; i++)
    {
        int nSize = 0;

        pdBlock = (nClearDataLen > pdBlock) ? pdBlock : nClearDataLen;
        nSize = RSA_public_decrypt(pdBlock, (const unsigned char*)pClearData,
                                   (unsigned char*)decryptedText, rsa, RSA_PKCS1_PADDING);
        pClearData += pdBlock;
        nClearDataLen -= pdBlock;

        if (nSize >= 0)
        {
            strRet += std::string(decryptedText, nSize);
        }
    }
    free(decryptedText);
    // 释放内存
    RSA_free(rsa);
    return strRet;
}

/*
 * 私钥加密
 */
std::string RSAUtil::enc_by_private_key(const std::vector<char>& privateKey, const std::vector<char>& data)
{
    std::string strRet;
    strRet.reserve(data.size());
    ///创建RSA指针
    RSA* rsa = create_RSA(privateKey.data(), false);
    if(rsa == NULL){
        return "";
    }

    int len = RSA_size(rsa);

    char* decryptedText = (char*)malloc(len + 1);
    memset(decryptedText, 0, len + 1);

    int nClearDataLen = data.size();

    int pdBlock = len - 11;
    int nCount = (nClearDataLen / pdBlock) + 1;//分段次数
    unsigned char* pClearData = (unsigned char*)data.data();
    //分段加密
    for (int i = 0; i < nCount; i++)
    {
        int nSize = 0;
        pdBlock = (nClearDataLen > pdBlock) ? pdBlock : nClearDataLen;
        nSize = RSA_private_encrypt(pdBlock, (const unsigned char*)pClearData,
                                    (unsigned char*)decryptedText, rsa, RSA_PKCS1_PADDING);
        pClearData += pdBlock;
        nClearDataLen -= pdBlock;

        if (nSize >= 0)
        {
            strRet += std::string(decryptedText, nSize);
        }
    }

    // 释放内存
     free(decryptedText);
    RSA_free(rsa);
    return strRet;
}
/*
 * 私钥解密
 */
std::string RSAUtil::dec_by_private_key(const std::vector<char>& privateKey, const std::vector<char>& data)
{
    std::string strRet;
    strRet.reserve(data.size());
    ///创建RSA指针
    RSA* rsa = create_RSA(privateKey.data(), false);
    if(rsa == NULL){
        return "";
    }

    int len = RSA_size(rsa);
    char* decryptedText = (char*)malloc(len + 1);
    memset(decryptedText, 0, len + 1);

    int nClearDataLen = data.size();
    int pdBlock = len;
    int nCount = (nClearDataLen / pdBlock) + 1;//分段次数
    unsigned char* pClearData = (unsigned char*)data.data();
    //分段解密
    for (int i = 0; i < nCount; i++)
    {
        int nSize = 0;

        pdBlock = (nClearDataLen > pdBlock) ? pdBlock : nClearDataLen;
        nSize = RSA_private_decrypt(pdBlock, (const unsigned char*)pClearData,
                                    (unsigned char*)decryptedText, rsa, RSA_PKCS1_PADDING);
        pClearData += pdBlock;
        nClearDataLen -= pdBlock;

        if (nSize >= 0)
        {
            strRet += std::string(decryptedText, nSize);
        }
    }
    free(decryptedText);
    // 释放内存
    RSA_free(rsa);
    return strRet;
}
//----------------------------------
static inline std::vector<char> str2Vec(const std::string& str){
    std::vector<char> vec;
    vec.resize(str.length());
    memcpy(vec.data(), str.data(), str.length());
    return vec;
}
std::string RSAUtil::enc_by_public_key(const std::string&  publicKey,
                                     const std::vector<char>& data){
    return enc_by_public_key(str2Vec(publicKey), data);
}
 std::string RSAUtil::dec_by_public_key(const std::string&  publicKey,
                                     const std::vector<char>& data){
     return dec_by_public_key(str2Vec(publicKey), data);
}
//私钥加解密
 std::string RSAUtil::enc_by_private_key(const std::string&  privateKey,
                                     const std::vector<char>& data){
    return enc_by_private_key(str2Vec(privateKey), data);
 }
 std::string RSAUtil::dec_by_private_key(const std::string&  privateKey,
                                     const std::vector<char>& data){
    return dec_by_private_key(str2Vec(privateKey), data);
 }


