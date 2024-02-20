
#include <sstream>
#include "sx_protocol.h"
#include "SxReceiver.h"

using namespace h7;

void log_upload_info(CString tag, SxReceiver::UploadedInfo& info){
    std::stringstream ss;

    ss << "tag = " << tag << NEW_LINE;
    ss << " hash = " << info.hash << NEW_LINE;
    ss << " blockSize = " << info.blockSize << " ,total_len = " << info.totalLen << NEW_LINE;

    ss << " blockIds = ";
    for(int i= 0 ; i < (int)info.blockIds.size() ; ++i){
        ss << info.blockIds[i];
        if(i != info.blockIds.size() - 1){
            ss << ",";
        }
    }
    ss << NEW_LINE;

    ss << " finishBlockIds = ";
    for(int i= 0 ; i < (int)info.finishBlockIds.size() ; ++i){
        ss << info.finishBlockIds[i];
        if(i != info.finishBlockIds.size() - 1){
            ss << ",";
        }
    }
    ss << NEW_LINE;
    auto str = ss.str();

    PRINTLN("log_upload_info: %s\n", str.data());
}

