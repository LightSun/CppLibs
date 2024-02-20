#pragma once

#include <list>
#include "SxClient.h"
#include "sx_web_protocol.h"
#include "core/common/common.h"

namespace h7 {
    class SelectFileManager{
    public:
        void startFileService(){
            std::thread thd([this](){
                String str;
                while (true) {
                    if(pollSelectFileReq(str)){
//                        std::vector<String> vec = utils::split(MED_WSC_LIMIT_CHAR, str);
//        #ifdef USE_UI_UPLOAD_INTERNAL
//                doSelectFileFromAnotherProcess(vec);
//        #else
//                doSelectFileFromAnotherProcess(vec);
//                //doSelectFileFromCmd();
//        #endif
                    }else{
                        waitSelectFileReq();
                    }
                }
            });
            thd.detach();
        }
    private:
        void addSelectFileReq(CString str){
            std::unique_lock<std::mutex> lock(m_mutex_fs);
            m_strs_fs.push_back(str);
            m_con_fs.notify_all();
        }
        bool pollSelectFileReq(String& req){
            std::unique_lock<std::mutex> lock(m_mutex_fs);
            if(m_strs_fs.size() == 0){
                return false;
            }
            req = m_strs_fs.front();
            m_strs_fs.pop_front();
            return true;
        }
        void waitSelectFileReq(){
            std::unique_lock<std::mutex> lock(m_mutex_fs);
            m_con_fs.wait(lock);
        }

    private:
        std::mutex m_mutex_fs;
        std::condition_variable m_con_fs;
        std::list<String> m_strs_fs;
    };
}
