#include "WebConnection.h"

#include "hv/WebSocketServer.h"
#include "utils/string_utils.hpp"
#include "utils/FileUtils.h"
#include "utils/url_encode.h"
#include "utils/system.h"
#include "utils/_time.h"
#include "utils/CmdHelper.h"

#include "utils/ByteBuffer.hpp"
#include "ipc/ShareMemory.h"
#include "service/service_base.h"

#include <chrono>

#include "FilesSync.hpp"

using namespace h7;

#define MED_WSC_LIMIT_CHAR "::"

WebConnection::WebConnection(SxClient* client, WebSocketChannelPtr channel):
    m_uploadClient(client),
    m_channel(channel)
{
    hio_set_keepalive_timeout(channel->io(), 0);
    h_atomic_set(&m_stopped, 0);
}

void WebConnection::sendDirectMsg(CString msg){
    if(h_atomic_get(&m_stopped) != 0){
        return;
    }
    m_channel->send(msg.data(), msg.size(), WS_OPCODE_TEXT);
}
void WebConnection::sendErrorMsg(const std::string& cmd,const std::string& msg){
    if(!m_channel ||h_atomic_get(&m_stopped) != 0){
        LOGW("sendErrorMsg >> failed by disconnected. cmd = %s, msg = %s\n", cmd.data(), msg.data());
        return;
    }
    std::string real_msg = "error:" + cmd + "#" + msg;
    m_channel->send(real_msg.data(), real_msg.size(), WS_OPCODE_TEXT);
}
void WebConnection::sendRetMsg(const std::string& cmd, const std::string& msg){
    if(!m_channel || h_atomic_get(&m_stopped) != 0){
        LOGW("sendRetMsg >> failed by disconnected. cmd = %s, msg = %s\n", cmd.data(), msg.data());
        return;
    }
    std::string real_msg = "ret:"+ cmd + "#" + msg;
    m_channel->send(real_msg.data(), real_msg.size(), WS_OPCODE_TEXT);
}
//----------------------------------------------------

void WebConnection::processMsg(const std::string& msg){
    if(h_atomic_get(&m_stopped) != 0){
        return;
    }
    std::thread thd([this, msg](){
        processMsg0(msg);
    });
    thd.detach();
}

void WebConnection::disconnect(){
    if(!h_atomic_cas(&m_stopped, 0, 1)){
        return;
    }
   // m_con_hb.notify_all();
    if(m_channel){
        m_channel->close();
        m_channel = nullptr;
    }
}
bool WebConnection::isConnected(){
    return h_atomic_get(&m_stopped) == 0;
}

void WebConnection::processMsg0(const std::string& str){
    PRINTLN("conn_id = %d, processMsg: %s\n", m_id, str.data());
    std::vector<String> vec = utils::split(MED_WSC_LIMIT_CHAR, str);
    if(vec.size() == 0){
        PRINTERR("empty msg.\n");
        return;
    }
    auto& cmd = vec[0];
    if(cmd == "connect"){
        PRINTLN("rec web: connect\n");
        if(vec.size() < 2 || vec[1].empty()){
            sendErrorMsg("connect","msg=need a token.");
            return;
        }
        if(m_uploadClient->isConnected()){
            PRINTLN("rec web: connect, but already isConnected.\n");
            sendRetMsg("connect","state=true");
            sendRetMsg("login","msg=login success");
        }else{
            auto ret = m_uploadClient->startService(vec[1]);
            sendRetMsg("connect","state=" + String(ret ? "true" : "false"));
        }
    }
    else if(cmd == "task_delay"){
        PRINTLN("rec web: task_delay\n");
        if(vec.size() < 2 || vec[1].empty()){
            sendErrorMsg("task_delay","msg=need a delay time.");
            return;
        }
        m_uploadClient->setTaskDelay(std::stoll(vec[1]));
    }
    else if (cmd == "upload") {
         //upload:<file_path>[:<user_data>]
        PRINTLN("rec web: upload\n");
        if(vec.size() < 2){
            sendErrorMsg("upload","msg=need a file.");
            return;
        }
        PRINTLN("upload: file = %s\n", vec[1].data());

        String _file = url_decode2(vec[1]);
        String file = str_to_GBEX(_file);
        PRINTLN("upload: real file = %s\n", file.data());
        if(!FileUtils::isFileExists(file)){
            sendErrorMsg("upload","msg=file not exist:" + file);
            return;
        }
        String fn = FileUtils::getSimpleFileName(_file);
        std::string usedData = vec.size() > 2 ? vec[2] : "";
        //
        auto res = m_uploadClient->checkState(vec[1]);
//        h7_sx_pro::web::UploadFileRes res = m_uploadClient->sendFileEnc(
//                    vec[1], fn, usedData);
        if(!res.msg.empty()){
            std::string msg = "msg=upload file(" + file + ") failed. " + res.msg;
            sendErrorMsg("upload", msg);
            PRINTERR("%s\n", msg.data());
        }else{
            //add to pool
            m_uploadClient->addHashTask(vec[1], fn, usedData);
            //sendRetMsg("upload", "hash=" + res.hash);
            PRINTERR("%s\n", res.msg.data());
        }
    }
    else if (cmd == "pause") {
        //pause:<hash>[:<user_data>]
        PRINTLN("rec web: pause\n");
        std::string& hash = vec[1];
        if(vec.size() < 2){
            sendErrorMsg("pause","msg=need a file hash.");
            return;
        }
        PRINTLN("pause: hash = %s\n", hash.data());
        std::string usedData = vec.size() > 2 ? vec[2] : "";
        m_uploadClient->pause(hash, usedData);
        sendRetMsg("pause","hash=" + hash);
    }
    else if (cmd == "disconnect") {
        PRINTLN("rec web: disconnect\n");
        disconnect();
       // sendRetMsg("disconnect", "msg=ok");
    }
    else if(cmd == "file_hash"){
        PRINTLN("rec web: file_hash: %s\n", vec[1].data());
        if(vec.size() < 2){
            sendErrorMsg("file_hash","msg=need a file.");
            return;
        }
        String _raw_file = vec[1];
        String _file = url_decode2(_raw_file);
        String file = str_to_GBEX(_file);

        PRINTLN(" >> real file: %s\n", file.data());
        if(!FileUtils::isFileExists(file)){
            sendErrorMsg("file_hash","msg=file not exist:" + file);
            return;
        }
        std::string hash = FileUtils::sha256(file);
        sendRetMsg("file_hash", "hash=" + hash + ",file=" + _file);
        PRINTLN("hash = %s\n", hash.data());
    }
    else if(cmd == "set_concurrent_fc"){
        PRINTLN("rec web: set_concurrent_fc\n");
        if(vec.size() < 2){
            sendErrorMsg("set_concurrent_fc","msg=need count.");
            return;
        }
        m_uploadClient->setConcurrentFileCount(std::stoi(vec[1]));
    }
    else if(cmd == "show_upload_dlg"){
        //show_upload_dlg::<id>::true::<fmt>
        PRINTLN("rec web: show_upload_dlg\n");
#ifdef USE_UI_UPLOAD_INTERNAL
        doSelectFileFromAnotherProcess(vec);
#else
        doSelectFileFromAnotherProcess(vec);
        //doSelectFileFromCmd();
#endif
    }
    else if(cmd == "url_encode"){
        PRINTLN("rec web: url_encode\n");
        if(vec.size() < 2){
            sendErrorMsg("url_encode","msg=need a file.");
            return;
        }
        char* ret = url_encode(vec[1].data());
        std::string str(ret);
        free(ret);
        sendRetMsg("url_encode", "url=" + str);
    }
    else if(cmd == "heart_beat"){
        PRINTLN("rec web: heart_beat\n");
        //ignore
    }else{
        PRINTERR("wrong cmd:%s\n", str.data());
    }
}

//--------------------------------------------------------
void WebConnection::doSelectFileFromCmd(){
    String path = ServiceBase::getCurrentExePath();
    String dir = FileUtils::getFileDir(path);
    String dst_file = dir + "/SxSelectFile.exe";
    if(!FileUtils::isFileExists(dst_file)){
        PRINTERR("file not exist: %s\n", dst_file.data());
        return;
    }
    String ret;
    int code = CmdHelper::execCmd(dst_file.data(), ret);
    PRINTLN("doSelectFileFromCmd >> code = %s\n", code);

    if(ret.empty()){
         sendErrorMsg("show_upload_dlg", "msg=no_select");
    }else{
        std::vector<String> rets = h7::utils::split("\r\n", ret);
        if(rets.size() == 1){
            rets = h7::utils::split("\n", ret);
        }
        MED_ASSERT_X(rets.size() >= 2, "execCmd failed for select file.");
        auto& _str = rets[0];
        auto& _enc_str = rets[1];
        sendRetMsg("show_upload_dlg", "files=" + _str + ";enc_files=" + _enc_str);
    }
}

void WebConnection::doSelectFileFromAnotherProcess(const std::vector<String>& vec){
//#ifndef USE_UI_UPLOAD_INTERNAL
    //start process to select file
    {
    String path = ServiceBase::getCurrentExePath();
    String dir = FileUtils::getFileDir(path);
    String dst_file = dir + "/SxSelectFile.exe";
    if(!FileUtils::isFileExists(dst_file)){
        PRINTERR("file not exist: %s\n", dst_file.data());
        return;
    }
    //build params.
    String params;
    if(vec.size() > 3){
        params += vec[2] + " " + vec[3];
    }else if(vec.size() > 2){
        params += vec[2];
    }
    //ud
    doSelectFileFromAnotherProcess0(dst_file, vec[1], params);
    }
//#endif
}

void WebConnection::doSelectFileFromAnotherProcess0(CString appPath, CString ud, CString params){
//#ifndef USE_UI_UPLOAD_INTERNAL
    int size = 4 << 10;
    //'Global\\' need admin permission
   // m_shareM = sk_make_sp<ShareMemory>("Global\\sxsm_select_file", size);
   sk_sp<h7::ShareMemory> m_shareM = sk_make_sp<ShareMemory>(SM_SHARE_KEY(sxsm_select_file, true), size);
    if(!m_shareM->create()){
        m_shareM->destroy();
        m_shareM = nullptr;
        PRINTERR("create ipc(sxsm_select_file) failed\n");
        return;
    }
    ShareMemory& sm = *m_shareM;
    //
    void* ptr = sm.getDataPtr();

    bool startApp = false;
    while (h_atomic_get(&m_stopped) == 0) {
        int* ip = (int*)ptr;
        if(*ip > 0){
            ByteBuffer buf(ptr, size, size);
            //data rec
            int rec_code = buf.getInt();
            PRINTLN("ipc server: rec_code = %d\n", rec_code);
            auto _str = buf.getString();
            if(_str.empty()){
                sendErrorMsg("show_upload_dlg", "msg=no_select");
            }else{
                auto _enc_str = buf.getString();
                sendRetMsg("show_upload_dlg", "files=;enc_files=" + _enc_str);
                onPostSelectFile(ud, _enc_str);
            }
            break;
        }else{
            if(!startApp){
#ifndef USE_UI_UPLOAD_INTERNAL
                bool ret = ServiceBase::StartAppAsUser(appPath, params);
                PRINTLN("StartAppAsUse r>> ret = %d \n", ret);
#else
                bool ret = ServiceBase::StartApp(appPath, params);
                PRINTLN("StartApp r>> ret = %d \n", ret);
#endif
                startApp = ret;
            }
        }
    }
    PRINTLN("doSelectFileFromAnotherProcess0: end.\n");
    if(m_shareM){
        m_shareM->destroy();
        m_shareM = nullptr;
    }
//#endif
}

void WebConnection::onPostSelectFile(CString ud, CString encs){
    std::vector<String> vec = utils::split(",", encs);
    for(uint32 i = 0 ; i < vec.size() ; ++i){
        String _file = url_decode2(vec[i]);
        String file = str_to_GBEX(_file);
        PRINTLN("onPostSelectFile: real file = %s\n", file.data());
        if(!FileUtils::isFileExists(file)){
            continue;
        }
        String fn = FileUtils::getSimpleFileName(_file);
        m_uploadClient->addHashOnlyTask(vec[i], fn, ud);
    }
}

void WebConnection::doSelectFile(const std::vector<String>& vec){
#ifdef USE_UI_UPLOAD_INTERNAL
//    med_api::SelectFileContext ctx;
//    {
//        bool single = true;
//        if(vec.size() > 1){
//            single = (vec[1] == "true");
//        }
//        String fmt = "*.*";
//        if(vec.size() > 2){
//            fmt = vec[2];
//        }
//        ctx.multi = !single;
//        ctx.fmts = utils::split(",", fmt);
//    }
//    m_uiHelper->showGtkDlg(&ctx);

//    //
//    String _str;
//    _str.reserve(128);
//    String _enc_str;
//    _enc_str.reserve(128);
//    {
//        int size = ctx.files.size();
//        for(int i = 0 ; i < size ; ++i){
//            String dfile = utils::replace2("\\\\", "/", ctx.files[i]);
//            dfile = utils::replace2("\\", "/", dfile);
//            _str += dfile;
//            //
//            _enc_str += url_encode2(dfile.data());
//            if(i != size - 1){
//                _str += ",";
//                _enc_str += ",";
//            }
//        }
//    }
//    PRINTLN("select files: %s\n", _str.data());
//    PRINTLN("select enc files: %s\n", _enc_str.data());
//    if(!_str.empty()){
//        sendRetMsg("show_upload_dlg", "files=" + _str + ";enc_files=" + _enc_str);
//    }else{
//        sendErrorMsg("show_upload_dlg", "msg=no_select");
//    }
#endif
}


