#include "DataBlock.h"

#include "hash.h"
#include "ByteBufferIO.h"

using namespace med;
#define DEF_HASH_SEED 11

String DataBlock::getDecodeResult(){
    String str;
    str.reserve(4096);
    for(int i = 0 ; i < blockCount ; ++i){
        str += blockDatas[i];
    }
    return str;
}
void DataBlock::setEncodeData(int i, CString data){
    blockSizes[i] = data.length();
    blockDatas[i] = data;
}
void DataBlock::setDecodeData(int i, CString data, bool verify){
    blockSizes[i] = data.length();
    blockDatas[i] = data;
    if(verify){
        auto hash = fasthash64(data.data(), data.length(), DEF_HASH_SEED);
        verifyStates[i] = (hash == blockHashes[i]);
    }else{
        //reguard as ok.
        verifyStates[i] = 1;
    }
}
String DataBlock::toString(){
    using namespace h7;
    ByteBufferOut bio(40960);
    bio.putInt(version);
    bio.putInt(blockCount);
    for(int i = 0 ; i < blockCount ; ++i){
        bio.putUInt(blockSizes[i]);
        bio.putULong(blockHashes[i]);
        bio.putRawString(blockDatas[i]);
    }
    return bio.bufferToString();
}
String DataBlock::fromString(CString str, bool strict){
    using namespace h7;
    ByteBufferIO bio((String*)&str);
    if(strict){
        if(str.length() < sizeof(int)){
            return "data error!";
        }
        version = bio.getInt();
        //current only one version. latter may more
        if(version != DB_VERSION){
            return "version code error!";
        }
        if(bio.getLeftLength() < sizeof(int)){
            return "data error: need bc!";
        }
        setBlockCount(bio.getInt());
        for(int i = 0 ; i < blockCount ; ++i){
            if(bio.getLeftLength() < sizeof(uint32)){
                return "data error: need bsize!";
            }
            auto bsize = bio.getUInt();
            if(bio.getLeftLength() < sizeof(size_t) + bsize){
                return "data error: need hash and bsize data!";
            }
            blockSizes[i] = bsize;
            blockHashes[i] = bio.getULong();
            blockDatas[i] = bio.getRawString(bsize);
        }
    }else{
        version = bio.getInt();
        setBlockCount(bio.getInt());
        for(int i = 0 ; i < blockCount ; ++i){
            auto bsize = bio.getUInt();
            blockSizes[i] = bsize;
            blockHashes[i] = bio.getULong();
            blockDatas[i] = bio.getRawString(bsize);
        }
    }
    return "";
}
