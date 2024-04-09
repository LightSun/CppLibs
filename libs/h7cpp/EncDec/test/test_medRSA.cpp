#include "../core/MedRSA.h"
#include "../core/AES.h"

#include <random>
#include <iostream>
#include "PerformanceHelper.h"

using namespace med;
using namespace h7;

enum KeyMode{
    kKey_16 = 2,
    kKey_24,
    kKey_32,
};

static void test_medRSA0(size_t len);
static String gen_sequence(size_t len);
static void test_AES(size_t len, KeyMode keyMode);

static void test_AES_ecb(size_t seqLen, KeyMode keyMode);
static void test_AES_cbc(size_t seqLen, KeyMode keyMode);
static void test_AES_cfb1(size_t seqLen, KeyMode keyMode);
static void test_AES_cfb8(size_t seqLen, KeyMode keyMode);
static void test_AES_cfb128(size_t seqLen, KeyMode keyMode);
static void test_AES_ofb128(size_t seqLen, KeyMode keyMode);

extern std::string PUBLIC_KEY;
extern std::string PRIVATE_KEY;


void test_medRSA(){
    //auto str = gen_sequence(100 << 20);
    //printf("gen_sequence(%lu): %s\n", str.length(), str.data());
    //test_medRSA0(10 << 20);

    test_AES(10 << 20, KeyMode::kKey_16);
    test_AES(10 << 20, KeyMode::kKey_24);
    test_AES(10 << 20, KeyMode::kKey_32);
}

void test_AES(size_t len, KeyMode keyMode){
    test_AES_ecb(len, keyMode);
    test_AES_cbc(len, keyMode);
    test_AES_cfb1(len, keyMode);
    test_AES_cfb8(len, keyMode);
    test_AES_cfb128(len, keyMode);
    test_AES_ofb128(len, keyMode);
}

void test_AES_ecb(size_t seqLen, KeyMode keyMode){
    h7::PerfHelper ph;
    auto str = gen_sequence(seqLen);
    auto key = gen_sequence(keyMode * 8);
    auto ivec = gen_sequence(16);
    AES aes(1 << 20);
    {
        ph.begin();
        auto encRet = aes.ecb(str, key, true);
        ph.print("ecb_enc");
        MED_ASSERT(!encRet.empty());
        ph.begin();
        auto decRet = aes.ecb(encRet, key, false);
        ph.print("ecb_dec");
        MED_ASSERT(!decRet.empty());
        MED_ASSERT(str == decRet);
    }
}

#define test_AES_name(name)\
void test_AES_##name(size_t seqLen, KeyMode keyMode){\
    h7::PerfHelper ph;\
    auto str = gen_sequence(seqLen);\
    auto key = gen_sequence(keyMode * 8);\
    auto ivec = gen_sequence(16);\
    AES aes(1 << 20);\
    {\
        ph.begin();\
        auto encRet = aes.name(str, key, ivec, true);\
        ph.print(String(#name) + "_enc");\
        MED_ASSERT(!encRet.empty());\
        ph.begin();\
        auto decRet = aes.name(encRet, key, ivec, false);\
        ph.print(String(#name) + "_dec");\
        MED_ASSERT(!decRet.empty());\
        MED_ASSERT(str == decRet);\
    }\
}
test_AES_name(cbc);
test_AES_name(cfb1);
test_AES_name(cfb8);
test_AES_name(cfb128);
test_AES_name(ofb128);

void test_medRSA0(size_t len){
    //gen the leng of sequence.
    //enc by public key, dec by private key
    //enc by private key, dec by public key
    h7::PerfHelper ph;
    auto str = gen_sequence(len);
    MedRSA mrsa(1 << 20); // 1M
    printf("max_tc = %d\n", mrsa.getMaxThreadCount());
    {
        ph.begin();
        auto encRet = mrsa.encByPubKey(PUBLIC_KEY, str);
        ph.print("encByPubKey");
        MED_ASSERT(!encRet.empty());
        ph.begin();
        auto decRet = mrsa.decByPriKey(PRIVATE_KEY, encRet);
        ph.print("decByPriKey");
        MED_ASSERT(!decRet.empty());
        MED_ASSERT(decRet == str);
    }
    {
        ph.begin();
        auto encRet = mrsa.encByPriKey(PRIVATE_KEY, str);
        ph.print("encByPriKey");
        MED_ASSERT(!encRet.empty());
        ph.begin();
        auto decRet = mrsa.decByPubKey(PUBLIC_KEY, encRet);
        ph.print("decByPubKey");
        MED_ASSERT(!decRet.empty());
        MED_ASSERT(decRet == str);
    }
}

String gen_sequence(size_t len){
    std::random_device rd;
    std::mt19937 gen(rd()); //gen是一个使用rd()作种子初始化的标准梅森旋转算法的随机数发生器
    std::uniform_int_distribution<> distrib(0, 255);//[0,255]
    String str;
    str.resize(len);
    for(size_t i = 0 ; i < len ; ++i){
        ((char*)str.data())[i] = distrib(gen);
    }
    return str;
}

std::string PUBLIC_KEY =
        "-----BEGIN PUBLIC KEY-----\n"
        "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCw/YJVyXIVh7Vh0DAFqASWB8x/\n"
        "XXrzF2yuto3iZkkqBFasojN9R94FFHhT7AFHWwvXGFdXsUPLxq3R6GXoqdynqAX6\n"
        "UxROZD/CWBXRSIQL6c9S4+qsDH8MnXYYQmZZRSsDt5zpzukSU0nEr9+qq+GMBPAQ\n"
        "scRim6wTV6STOXuEhQIDAQAB\n"
        "-----END PUBLIC KEY-----\n";

std::string PRIVATE_KEY =
        "-----BEGIN RSA PRIVATE KEY-----\n"
        "MIICXAIBAAKBgQCw/YJVyXIVh7Vh0DAFqASWB8x/XXrzF2yuto3iZkkqBFasojN9\n"
        "R94FFHhT7AFHWwvXGFdXsUPLxq3R6GXoqdynqAX6UxROZD/CWBXRSIQL6c9S4+qs\n"
        "DH8MnXYYQmZZRSsDt5zpzukSU0nEr9+qq+GMBPAQscRim6wTV6STOXuEhQIDAQAB\n"
        "AoGACPEV152wzNOpX0K0WmTNroLAWyLu5j8lt4Hzkx+VzLChbGFZdpfd6KXLGnpO\n"
        "6jr4UyqgpwaGpVHpUDSMiX+jbSNbqNNkwAL8cv8wYlQBvZhe4NoqTi+e99dwH1iD\n"
        "McwjIR8IEDYxCSdplGWPN5hKkSoP6OWXNYguDDK0y8V52OECQQDrHhrJxRI89sM5\n"
        "vrOk7FBCbMV2T2foF2dcdf7j4gLT7p5Be8plI11P4KD0ZHY4gzjLJU/NdmX96EOk\n"
        "KDaZjnfNAkEAwLXBw8t0JvCzX/Qvn7izht/x8C85yYtUqDkFmqDr31era/yIUGSR\n"
        "pPLblXh8oFrjTEPp/H86mGKX5YLR0VyXmQJAXyjvFKzzhcMmHtAFa4HNtiTKAul+\n"
        "l5wpVG3ZfSgzls1kNgLBVw/qK3MyEdg7VQIfUXFHjFQYUZzZC67O8nWMHQJARwn5\n"
        "jtLOU5iBl0qtz6RH0d12E4NlOw24vHagwTq3GNL5p0olefVI11SLa9NJpdc7WR7j\n"
        "/6drE0etFPcfn50RaQJBAKaYjlQMlY4mF7TPxjkaCqc2sezfYYAIKtY9ipXovMG/\n"
        "SjLcXRy1Q3wpLtSRBAoo0K6GdiJnjEwH1WC1csWO54I=\n"
        "-----END RSA PRIVATE KEY-----\n";
