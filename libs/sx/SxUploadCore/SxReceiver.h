#ifndef SXRECEIVER_H
#define SXRECEIVER_H

#include "common/c_common.h"
#include <string>
#include <vector>
#include "utils/ByteBuffer.hpp"

namespace h7 {

class SxReceiver
{
public:
    struct State{
        int code {-1};
        std::string msg;
    };
    struct RecInfo
    {
        int type;
        int size;
    };
    struct RecommendInfo
    {
        int maxBlockSize {0};
    };
    struct UploadedInfo{
        std::string hash;
        sint64 blockSize;
        sint64 totalLen;
        std::vector<int> blockIds;       //all blocks
        std::vector<int> finishBlockIds; //finished blocks

        void reset(){
            blockIds.clear();
            finishBlockIds.clear();
        }
        std::vector<int> getUndoBlockIds()const{
            std::vector<int> vec;
            for(int id : blockIds){
                bool contains = false;
                for(int _fid : finishBlockIds){
                    if(_fid == id){
                        contains = true;
                        break;
                    }
                }
                if(!contains){
                    vec.push_back(id);
                }
            }
            return vec;
        }
    };

    void putData(const void* data, uint32 size){
        int _pos = m_buf.position();
        m_buf.put(data, size);
        m_buf.position(_pos);
    }
    bool read();
    uint32 read0(const void* data, uint32 size);

    //void read(SafeByteBuffer* buffer);

    State& getState(){
        return m_state;
    }
    UploadedInfo& getUploadedInfo(){
        return m_uploadedInfo;
    }
    RecInfo& getRecInfo(){
        return m_recInfo;
    }
    RecommendInfo& getRecommendInfo(){
        return m_recommendInfo;
    }
    String& getFileHash(){
        return m_fileHash;
    }
    String& getFilePath(){
        return m_filePath;
    }
    int getReqType(){
        return m_reqType;
    }
    String& getUserdata(){
        return m_ud;
    }
private:
    ByteBuffer m_buf {4 << 20}; // 4M
    int m_reqType;
    State m_state;
    UploadedInfo m_uploadedInfo;
    RecInfo m_recInfo;
    RecommendInfo m_recommendInfo;
    String m_fileHash;
    String m_filePath; //the local file path
    String m_ud;

    int m_dsize {0};

    static uint32 readGetUploadInfo(char* data, UploadedInfo& info);
};

}

#endif // SXRECEIVER_H
