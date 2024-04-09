#include "AES.h"
#include <openssl/modes.h>
#include <openssl/aes.h>
#include "Padding.h"

#include "hash.h"
#include "ThreadPool.h"

using namespace med;

namespace med {
class AESEncDec{
public:
    ~AESEncDec(){}
    virtual String encode(CString key, CString ivec, CString data) = 0;
    virtual String decode(CString key, CString ivec, CString data) = 0;
    void verifyKey(CString key, CString ivec){
        // 检查密钥合法性(只能是16、24、32字节)
        MED_ASSERT(key.size() == 16 || key.size() == 24 || key.size() == 32);
        if(!ivec.empty()){
            MED_ASSERT(ivec.size() == 16);
        }
    }
};
}
//------------------------------------
class ImplEcb: public AESEncDec{
public:
    String encode(CString key, CString ivec, CString in) override{
        verifyKey(key, ivec);
        // 生成加密key
        AES_KEY aes_key;
        if (AES_set_encrypt_key((const unsigned char*)key.data(), key.size() * 8, &aes_key) != 0){
            fprintf(stderr, "ImplEcb >> AES_set_encrypt_key failed. key = %s\n",
                    key.data());
            return "";
        }
        // 进行PKCS7Padding填充
        auto inTemp = Padding::doPKCS7Padding(in, AES_BLOCK_SIZE);
        int count = inTemp.size() / AES_BLOCK_SIZE;
        // 执行ECB模式加密
        String out;
        out.resize(inTemp.size());
        for (int i = 0; i < count; i++){
            AES_ecb_encrypt((const unsigned char*)inTemp.data() + AES_BLOCK_SIZE * i,
                            (unsigned char*)out.data() + AES_BLOCK_SIZE * i,
                            &aes_key, AES_ENCRYPT);
        }
        return out;
    }
    String decode(CString key, CString ivec, CString in) override{
        verifyKey(key, ivec);
        // 生成解密key
        AES_KEY aes_key;
        if (AES_set_decrypt_key((const unsigned char*)key.data(),
                                key.size() * 8, &aes_key) != 0){
            fprintf(stderr, "ImplEcb >> AES_set_decrypt_key failed. key = %s\n",
                    key.data());
           return "";
        }
        int count = in.size() / AES_BLOCK_SIZE;

        //执行ECB模式解密
        String out;
        out.resize(in.size()); // 调整输出buf大小
        for (int i = 0; i < count; i++){
           AES_ecb_encrypt((const unsigned char*)in.data() + AES_BLOCK_SIZE * i,
                           (unsigned char*)out.data() + AES_BLOCK_SIZE * i,
                           &aes_key, AES_DECRYPT);
        }
        //解除PKCS7Padding填充
        return Padding::doPKCS7UnPadding(out);
    }
};

class ImplCbc: public AESEncDec{
public:
    String encode(CString key, CString ivec, CString in) override{
        // 生成加密key
        AES_KEY aes_key;
        if (AES_set_encrypt_key((const unsigned char*)key.data(), key.size() * 8,
                                &aes_key) != 0){
            fprintf(stderr, "ImplCbc >> AES_set_encrypt_key failed. key = %s\n",
                    key.data());
            return "";
        }

        // 进行PKCS7Padding填充
        auto inTemp = Padding::doPKCS7Padding(in, AES_BLOCK_SIZE);

        // 执行CBC模式加密
        String out;
        String ivecTemp = ivec;    // ivec会被修改，故需要临时变量来暂存
        out.resize(inTemp.size()); // 调整输出buf大小
        AES_cbc_encrypt((const unsigned char*)inTemp.data(),
                        (unsigned char*)out.data(),
                        inTemp.size(),
                        &aes_key,
                        (unsigned char*)ivecTemp.data(),
                        AES_ENCRYPT);
        return out;
    }
    String decode(CString key, CString ivec, CString in) override{
        // 生成解密key
        AES_KEY aes_key;
        if (AES_set_decrypt_key((const unsigned char*)key.data(),
                                key.size() * 8, &aes_key) != 0){
            fprintf(stderr, "ImplCbc >> AES_set_decrypt_key failed. key = %s\n",
                    key.data());
            return "";
        }

        // 执行CBC模式解密
        String out;
        String ivecTemp = ivec; // ivec会被修改，故需要临时变量来暂存
        out.resize(in.size()); // 调整输出buf大小
        AES_cbc_encrypt((const unsigned char*)in.data(),
                        (unsigned char*)out.data(),
                        in.size(),
                        &aes_key,
                        (unsigned char*)ivecTemp.data(),
                        AES_DECRYPT);

        // 解除PKCS7Padding填充
        return Padding::doPKCS7UnPadding(out);
    }
};
class _Impl: public AESEncDec{
public:
    ~_Impl(){}
    String encode(CString key, CString ivec, CString in) override{
        return exec(key, ivec, in, true);
    }
    String decode(CString key, CString ivec, CString in) override{
        return exec(key, ivec, in, false);
    }
protected:
    virtual String exec(CString key, CString ivec, CString in, bool enc) = 0;
};

class ImplCfb1: public _Impl{
protected:
    String exec(CString key, CString ivec, CString in, bool enc){
        // 特别注意：CFB模式加密和解密均使用加密key。
        // 生成加密key
        AES_KEY aes_key;
        if (AES_set_encrypt_key((const unsigned char*)key.data(), key.size() * 8,
                                &aes_key) != 0){
            fprintf(stderr, "ImplCfb1 >> AES_set_encrypt_key failed. key = %s\n",
                    key.data());
            return "";
        }
        // 执行CFB1模式加密或解密
        int num = 0;
        String ivecTemp = ivec; // ivec会被修改，故需要临时变量来暂存
        int encVal = enc ? AES_ENCRYPT : AES_DECRYPT;
        String out;
        out.resize(in.size()); // 调整输出buf大小
        AES_cfb1_encrypt((const unsigned char*)in.data(),
                       (unsigned char*)out.data(),
                       in.size() * 8,
                       &aes_key,
                       (unsigned char*)ivecTemp.data(),
                       &num,
                       encVal);
        return out;
    }
};

class ImplCfb8: public _Impl{
protected:
    String exec(CString key, CString ivec, CString in, bool enc){
        // 特别注意：CFB模式加密和解密均使用加密key。
        // 生成加密key
        AES_KEY aes_key;
        if (AES_set_encrypt_key((const unsigned char*)key.data(),
                                key.size() * 8, &aes_key) != 0){
            fprintf(stderr, "ImplCfb8 >> AES_set_encrypt_key failed. key = %s\n",
                    key.data());
            return "";
        }

        // 执行CFB8模式加密或解密
        int num = 0;
        String ivecTemp = ivec; // ivec会被修改，故需要临时变量来暂存
        int encVal = enc ? AES_ENCRYPT : AES_DECRYPT;
        String out;
        out.resize(in.size()); // 调整输出buf大小
        AES_cfb8_encrypt((const unsigned char*)in.data(),
                        (unsigned char*)out.data(),
                        in.size(),
                        &aes_key,
                        (unsigned char*)ivecTemp.data(),
                        &num,
                        encVal);
        return out;
    }
};

class ImplCfb128: public _Impl{
protected:
    String exec(CString key, CString ivec, CString in, bool enc){
        // 特别注意：CFB模式加密和解密均使用加密key。
        // 生成加密key
        AES_KEY aes_key;
        if (AES_set_encrypt_key((const unsigned char*)key.data(),
                                key.size() * 8, &aes_key) != 0){
            fprintf(stderr, "ImplCfb128 >> AES_set_encrypt_key failed. key = %s\n",
                    key.data());
            return "";
        }

        // 执行CFB128模式加密或解密
        int num = 0;
        String ivecTemp = ivec; // ivec会被修改，故需要临时变量来暂存
        int encVal = enc ? AES_ENCRYPT : AES_DECRYPT;
        String out;
        out.resize(in.size()); // 调整输出buf大小
        AES_cfb128_encrypt((const unsigned char*)in.data(),
                        (unsigned char*)out.data(),
                        in.size(),
                        &aes_key,
                        (unsigned char*)ivecTemp.data(),
                        &num,
                        encVal);
        return out;
    }
};

class ImplOfb128: public _Impl{
protected:
    String exec(CString key, CString ivec, CString in, bool){
        // 特别注意：OFB模式加密和解密均使用加密key。
        // 生成加密key
        AES_KEY aes_key;
        if (AES_set_encrypt_key((const unsigned char*)key.data(),
                                key.size() * 8, &aes_key) != 0){
            fprintf(stderr, "ImplOfb128 >> AES_set_encrypt_key failed. key = %s\n",
                    key.data());
            return "";
        }
        // 特别注意：加密与解密执行相同的操作
        // 执行OFB128模式加密或解密
        int num = 0;
        String ivecTemp = ivec; // ivec会被修改，故需要临时变量来暂存
        String out;
        out.resize(in.size()); // 调整输出buf大小
        AES_ofb128_encrypt((const unsigned char*)in.data(),
                       (unsigned char*)out.data(),
                       in.size(),
                       &aes_key,
                       (unsigned char*)ivecTemp.data(),
                       &num);
        return out;
    }
};

//------------------------------------------------------------------------
void AES::doEncode(CString key, CString ivec, CString data, DataBlock* out,
                   AESEncDec* impl){
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
                                    [impl, key, ivec, &splitDatas, out](int di){
            out->blockHashes[di] = fasthash64(splitDatas[di].data(),
                            splitDatas[di].length(), DEF_HASH_SEED);
            auto ret = impl->encode(key, ivec, splitDatas[di]);
            out->setEncodeData(di, ret);
            return true;
        });
    }else{
        out->setBlockCount(1);
        out->blockHashes[0] = fasthash64(data.data(), data.length(), DEF_HASH_SEED);
        auto ret = impl->encode(key, ivec, data);
        out->setEncodeData(0, ret);
    }
}
void AES::doDecode(CString key, CString ivec, DataBlock* out, bool verify,
                   AESEncDec* impl){
    MED_ASSERT(out->blockCount > 0);
    if(out->blockCount > 1){
        h7::ThreadPool::batchRawRun(m_tc, 0, out->blockCount,
                    [impl, out, key, ivec, verify](int di){
            auto ret = impl->decode(key, ivec, out->blockDatas[di]);
            out->setDecodeData(di, ret, verify);
            return true;
        });
    }else{
        auto ret = impl->decode(key, ivec, out->blockDatas[0]);
        out->setDecodeData(0, ret, verify);
    }
}

String AES::doExec(CString tag, CString in, CString key, CString ivec,
                   bool enc, AESEncDec* impl){
    if(key.size() != 16 && key.size() != 24 && key.size() != 32){
        fprintf(stderr, "AES::doExec >> key(%s) is wrong. req = 16/24/32\n", key.data());
        return "";
    }
    if(!ivec.empty() && ivec.size() != 16){
        fprintf(stderr, "AES::doExec >> ivec(%s) is wrong. req = 16\n", ivec.data());
        return "";
    }
    DataBlock block;
    if(enc){
        doEncode(key, ivec, in, &block, impl);
        return block.toString();
    }else{
        block.fromString(in);
        doDecode(key, ivec, &block, true, impl);
        if(block.isVerifyOk()){
            return block.getDecodeResult();
        }
        fprintf(stderr, "%s: decode verify failed.\n", tag.data());
        return "";
    }
}
String AES::ecb(CString in, CString key, bool enc){
    ImplEcb impl;
    return doExec("ecb", in, key, "", enc, &impl);
}
String AES::cbc(CString in, CString key, CString ivec, bool enc){
    ImplCbc impl;
    return doExec("cbc", in, key, ivec, enc, &impl);
}
String AES::cfb1(CString in, CString key, CString ivec, bool enc){
    ImplCfb1 impl;
    return doExec("cfb1", in, key, ivec, enc, &impl);
}
String AES::cfb8(CString in, CString key, CString ivec, bool enc){
    ImplCfb8 impl;
    return doExec("cfb8", in, key, ivec, enc, &impl);
}
String AES::cfb128(CString in, CString key, CString ivec, bool enc){
    ImplCfb128 impl;
    return doExec("cfb128", in, key, ivec, enc, &impl);
}
String AES::ofb128(CString in, CString key, CString ivec, bool enc){
    ImplOfb128 impl;
    return doExec("ofb128", in, key, ivec, enc, &impl);
}
