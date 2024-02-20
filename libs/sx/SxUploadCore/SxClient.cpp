#include "SxClient.h"
#include <iostream>
#include <mutex>

#include "utils/FileUtils.h"
#include "utils/hash.h"
#include "utils/h_atomic.h"
#include "utils/system.h"
#include "utils/string_utils.hpp"
#include "utils/url_encode.h"
#include "utils/FileIO.h"
#include "utils/PerformanceHelper.h"
#include "SxReceiver.h"
#include "sx_log.h"

using namespace hv;
using namespace h7;
using namespace h7_sx_pro;

#define synchronized(a) std::unique_lock<std::mutex> lck(a);
#define DEF_HASH_HT 11

SxClient::SxClient(CString addr, int port, uint32 blockSize):
    m_port(port), m_maxBlockSize(blockSize),m_addr(addr){
    MED_ASSERT(blockSize > 0);
    m_hashTasks.setHashFunction([](const HT& t){
        return fasthash32(t->userData.data(), t->userData.length(), DEF_HASH_HT);
    });
    std::thread thd([this](){
        while (h_atomic_get(&m_reqStop) == 0) {
            HT ht = nextHashTask();
            if(ht){
                //may cost much time
                if(ht->hash.empty()){
                    ht->hash = cal_hash(ht);
                }
                //check if cancelled
                if(h_atomic_get(&ht->canceled) != 0){
                    //canceled.
                    synchronized(m_mutex_ht){
                        m_hashTasks.remove(ht);
                    }
                    continue;
                }
                //no need uplpad, send to server.
                if(h_atomic_get(&ht->shouldUpload) == 0){
                    synchronized(m_mutex_ht){
                        m_hashTasks.remove(ht);
                    }
                    String msg = sendFileInfoOnly(ht->file_enc, ht->hash, ht->userData);
                    if(!msg.empty()){
                        PRINTERR("sendFileInfoOnly: %s\n", msg.data());
                    }else if(ht->onPostTask){
                        (*ht->onPostTask)(ht);
                    }
                    continue;
                }
                //logined remove from queue and post.
                if(isLogined()){
                    {
                    synchronized(m_mutex_ht){
                        m_hashTasks.remove(ht);
                    }
                    }
                    post([this, ht](){
                        auto res = sendFileEnc(ht->file_enc, ht->filename,
                                    ht->userData, ht->hash, ht->simpleTest);
                        PRINTLN("sendFileEnc >> file,hash,msg = %s,%s,%s\n",
                             res.file.data(), res.hash.data(), res.msg.data());
                    });
                }else{
                    //no login wait login.
                    synchronized(m_mutex_ht){
                        m_cv_ht.wait(lck);
                    }
                }
            }else{
                //no task
                synchronized(m_mutex_ht){
                   m_cv_ht.wait(lck);
                }
            }
        }
    });
    thd.detach();
    // sender callback
    SxSender::Callback cb;
    cb.onSendResult = [this](CString hash, int code){
        if(code != SxSender::kTYPE_cb_OK){
            return;
        }
        ETPI tp;
        {
        std::unique_lock<std::mutex> lock(m_mutex_exe);
        auto it = m_exeMap.find(hash);
        if(it != m_exeMap.end()){
            tp = it->second;
        }
        }
        if(!tp){
            return;
        }
        //paused
        if(h_atomic_get(&tp->paused) != 0){
            return;
        }
        {
            int old = h_atomic_add(&tp->finishCount, 1);
            synchronized(m_mutex_cb){
                if(m_callback != NULL){
                    m_callback->func_progress(hash, old + 1 + tp->preFinishCount,
                                              tp->preFinishCount + tp->reqCount);
                }
            }
        }
        if(h_atomic_get(&tp->paused) != 0){
            return;
        }
        //send all done
        if((h_atomic_get(&tp->finishCount) == tp->reqCount)){
            PRINTLN("%s >> send file finished.\n", tp->fname.data());
            FileEnd fs;
            fs.hash = hash;
            fs.userData = tp->userdata;
            m_sender.sendFileEnd(fs);
            //remove
            removeExeTask(hash);
        }
    };
    m_sender.setCallback(&cb);
}
h7_sx_pro::web::UploadFileRes SxClient::checkState(CString file_enc){
    if (!m_client.isConnected()){
        PRINTERR("client not connected.\n");
        return {file_enc, "", "client not connected."};
    }
    if(h_atomic_get(&m_logined) == 0){
        PRINTERR("client not login.\n");
        return {file_enc, "", "client not login."};
    }
    String _file = url_decode2(file_enc);
    String file = str_to_GBEX(_file);
    if(!FileUtils::isFileExists(file)){
        return {file, "", "file not exist."};
    }
    return {file, "", ""};
}
void SxClient::addHashTask(CString file_enc, CString filename,
                 CString userData,  bool simpleTest){
    HT ht = HT(new HashTask());
    ht->file_enc = file_enc;
    ht->filename = filename;
    ht->userData = userData;
    ht->simpleTest = simpleTest;
    {
        synchronized(m_mutex_ht){
            m_hashTasks.add(ht);
            m_cv_ht.notify_all();
        }
    }
}
void SxClient::addHashOnlyTask(CString file_enc, CString filename,
                     CString userData){

    String _file = url_decode2(file_enc);
    HT ht = HT(new HashTask());
    ht->file_path_utf8 = _file;
    ht->file_enc = file_enc;
    ht->filename = filename;
    ht->userData = userData;
    ht->simpleTest = false;
    h_atomic_set(&ht->shouldUpload, 0);
    {
        synchronized(m_mutex_ht){
            m_hashTasks.add(ht);
            m_cv_ht.notify_all();
        }
    }
}
String SxClient::sendFileInfoOnly(CString file_enc, CString hash,
                      CString userData){
    String _file = url_decode2(file_enc);
    String file = str_to_GBEX(_file);

    if(h_atomic_get(&m_logined) != 0){
        FileInfo info;
        info.filepath = _file;
        info.userdata = userData;
        info.hash = hash;
        m_sender.sendFileInfo(info);
        return "";
    }else{
        return "client not logined";
    }
}
void SxClient::post(std::function<void()> func){
    auto ptr = FUNC_MAKE_SHARED_PTR_0(void(), func);
    std::thread thd([ptr](){
        (*ptr)();
    });
    thd.detach();
}
bool SxClient::startService(CString token){
    if(m_client.isConnected()){
        return true;
    }
    int connfd = m_client.createsocket(m_port, m_addr.data());
    if (connfd < 0) {
        PRINTLN("SxClient::startService >> createsocket failed.\n");
        return false;
    }
    m_client.onConnection = [this, token](const SocketChannelPtr& channel) {
        std::string peeraddr = channel->peeraddr();
        if (channel->isConnected()) {
            hio_set_keepalive_timeout(channel->io(), 0);
            PRINTLN("connected to %s! connfd=%d\n", peeraddr.data(), channel->fd());
            m_sender.setChannel(channel);
            m_sender.startRec();
            //get recommend info
            std::thread thd([this, token](){
                m_sender.sendGetRecommendInfo();
                hv_msleep(20);
                Login lg;
                lg.token = token;
                m_sender.sendLogin(lg);
            });
            thd.detach();
            {
                std::unique_lock<std::mutex> lock(m_mutex_cb);
                if(m_callback){
                    m_callback->func_connn(true);
                }
            }
        } else {
            PRINTLN("disconnected to %s! connfd=%d, err = %d, all task will be remove.\n",
                    peeraddr.data(), channel->fd(), channel->error());
            //PRINTLN("disconnected >> left file count = %d\n", m_sync.getItemCount());
            h_atomic_set(&m_logined, 0);
            stopAndClearExeTasks();
            {
                std::unique_lock<std::mutex> lock(m_mutex_pending);
                m_pendingMap.clear();
            }
            m_sender.stopRec();
            m_sender.setChannel(nullptr);
            {
                std::unique_lock<std::mutex> lock(m_mutex_cb);
                if(m_callback){
                    m_callback->func_connn(false);
                }
            }
            //notify hash tasks loop.
            {
                synchronized(m_mutex_ht){
                    m_cv_ht.notify_all();
                }
            }
        }
    };
    m_client.onMessage = [this](const SocketChannelPtr& channel,
            Buffer* buf) {
        //PRINTLN("onMessage: size = %d\n", buf->size());
        m_rec.putData(buf->data(), buf->size());
        while (true) {
            if(!m_rec.read()){
                //data not enough
                break;
            }
            dohandleMessage(channel);
        }
    };
    reconn_setting_init(&m_reconn);
    m_reconn.min_delay = 1000;
    m_reconn.max_delay = 30000;
    m_reconn.delay_policy = 2;
    m_client.setReconnect(&m_reconn);
    m_client.start(true);
    return true;
}
void SxClient::stopService(){
    PRINTLN("stopService >> called \n");
    if(m_client.isConnected()){
        m_client.stop();
        m_client.closesocket();
    }
}
bool SxClient::isLogined(){
    return h_atomic_get(&m_logined) != 0;
}
void SxClient::pause(CString hash, CString userdata, bool sendServer){
    PRINTLN("rec pause: hash = %s, ud = %s, send_server = %d.\n",
            hash.data(), userdata.data(), sendServer);
    if (!m_client.isConnected() || h_atomic_get(&m_logined) == 0){
        return;
    }
    if(!userdata.empty()){
        synchronized(m_mutex_ht){
            int hash = fasthash32(userdata.data(), userdata.length(), DEF_HASH_HT);
            int task_idx = m_hashTasks.indexOfHash(hash);
            if(task_idx >= 0){
                HT ht = m_hashTasks.get(task_idx);
                h_atomic_set(&ht->canceled, 1);
                m_hashTasks.removeAt(task_idx);
                m_cv_ht.notify_all();
            }
        }
    }
    //no hash means no next.
    if(!hash.empty()){
    //executor
        {
            std::unique_lock<std::mutex> lock(m_mutex_exe);
            auto it = m_exeMap.find(hash);
            if(it != m_exeMap.end()){
                if(it->second){
                    h_atomic_set(&it->second->paused, 1);
                }
                m_exeMap.erase(it);
                PRINTLN("exe task removed: %s\n", hash.data());
            }
        }
        PRINTLN("SxClient::pause >> exe stop done.\n");
        //pending for get upload info
        {
            std::unique_lock<std::mutex> lock(m_mutex_pending);
            auto it = m_pendingMap.find(hash);
            if(it != m_pendingMap.end()){
                m_pendingMap.erase(it);
            }
        }
        PRINTLN("SxClient::pause >> m_pendingMap remove done.\n");
        //wait tasks.
        {
            std::unique_lock<std::mutex> lock(m_mutex_cfc);
            auto it = m_waitFileTasks.begin();
            for(;it != m_waitFileTasks.end();){
                if(it->hash == hash){
                    m_waitFileTasks.erase(it);
                    break;
                }
                it ++;
            }
        }
    }
    PRINTLN("SxClient::pause >> m_waitFileTasks remove done.\n");

    if(!hash.empty()){
        m_sender.removeMsgs(hash);
        PRINTLN("SxClient::pause >> m_sender.removeMsgs done.\n");
    }
    //remove pool msgs
    std::thread thd([this, hash, sendServer, userdata](){
        //notify server
        if(sendServer){
            Pause pau;
            pau.hash = hash;
            pau.userData = userdata;
            m_sender.sendPause(pau);
            PRINTLN("SxClient::pause >> send pause to server done.\n");
        }
    });
    thd.detach();
}

//simpleTest: true means skip old info
web::UploadFileRes SxClient::sendFile(CString file_enc,CString file,
                                      CString filename,
                                      CString userData,
                                      CString _hash, bool simpleTest){
    if (!m_client.isConnected()){
        PRINTERR("client not connected.\n");
        return {file, "", "client not connected."};
    }
    if(!simpleTest && h_atomic_get(&m_logined) == 0){
        PRINTERR("client not login.\n");
        return {file, "", "client not login."};
    }
    int tc = getTC();
    if(tc == 0){
        return {file, "", "send file failed by getAvailablePhysMemBytes is small."};
    }
    uint64 total_len;
    String hash = _hash;
    {
        //no hash provide. we need compute
        if(hash.empty()){
            std::unique_lock<std::mutex> lock(m_mutex_hash);
            hash = FileUtils::sha256(file, &total_len);
            if(hash.empty()){
                return {file, "", "open file failed."};
            }
            //hash = fasthash32(file_content.data(), file_content.length(), HASH_SEED);
            PRINTLN("file = %s, hash = %s, file_content.len = %lu\n",
                    file.data(), hash.data(), total_len);
        }else{
            FileInput fis(file);
            if(!fis.is_open()){
                return {file, "", "open file failed."};
            }
            total_len = fis.getLength();
            fis.close();
        }
    }
    PendingTask task;
    task.file_enc = file_enc;
    task.filename = filename;
    task.hash = hash;
    task.file = file;
    task.tc = tc;
    task.userData = userData;
    task.totalLen = total_len;

    return _sendFile(task, simpleTest);
}

h7_sx_pro::web::UploadFileRes SxClient::sendFileEnc(CString file_enc, CString filename,
                                          CString userData, CString hash,
                                          bool simpleTest){
    String _file = url_decode2(file_enc);
    String file = str_to_GBEX(_file);

    PRINTLN("file_hash: hash = %s, file = %s.\n",
            hash.data(), file.data());
    {
        if(h_atomic_get(&m_logined) != 0){
            FileInfo info;
            info.filepath = _file;
            info.userdata = userData;
            info.hash = hash;
            m_sender.sendFileInfo(info);
        }else{
            return {file, "", "client not login."};
        }
    }
    auto res = sendFile(file_enc, file, filename, userData,
                        hash, simpleTest);
    return res;
}

h7_sx_pro::web::UploadFileRes SxClient::sendFileEncWithHash(CString file_enc,
                              CString hash, CString userData, bool* file_exist){
    String _file = url_decode2(file_enc);
    String file = str_to_GBEX(_file);
    PRINTLN("sendFileEncWithHash >> file: %s\n", file.data());
    if (!m_client.isConnected()){
        PRINTERR("client not connected.\n");
        return {file, "", "client not connected."};
    }
    if(h_atomic_get(&m_logined) == 0){
        PRINTERR("client not login.\n");
        return {file, "", "client not login."};
    }
    int tc = getTC();
    if(tc == 0){
        return {file, "", "send file failed by getAvailablePhysMemBytes is small."};
    }
    if(!FileUtils::isFileExists(file)){
        if(file_exist){
            *file_exist = false;
        }
        return {file, "", "file not exist."};
    }
    if(file_exist){
        *file_exist = true;
    }
    FileInput fis(file);
    if(!fis.is_open()){
        return {file, "", "open file failed."};
    }
    auto total_len = fis.getLength();
    fis.close();
    String fn = FileUtils::getSimpleFileName(_file);
    //
    PendingTask task;
    task.file_enc = file_enc;
    task.filename = fn;
    task.file = file;
    task.hash = hash;
    task.tc = tc;
    task.userData = userData;
    task.totalLen = total_len;

    return _sendFile(task, false);
}

//allow hash empty
int SxClient::startUploadFromRemote(CString filepath, CString ud,
                                     const SxReceiver::UploadedInfo& _info,
                                    String* out_hash){
    //TODO check if file is in hash-ing.
    String file = str_to_GBEX(filepath);
    if(!FileUtils::isFileExists(file)){
        PRINTLN("startUploadFromRemote >> file not exist.\n");
        return kCode_FILE_NOT_EXIST;
    }
    PRINTLN("startUploadFromRemote >> %s\n", file.data());
    int tc = getTC();
    if(tc == 0){
        return kCode_MEMORY_NOT_ENOUGH;
    }
    //have block id must set block size.
    if(_info.blockIds.size() > 0 &&
            (_info.blockSize == 0 || _info.totalLen == 0)){
        PRINTLN("startUploadFromRemote >> param error.\n");
        return kCode_PARAM_ERROR;
    }
    SxReceiver::UploadedInfo info = _info;
    if(info.hash.empty()){
        //check if file is in hash-ing. if is, we add a post task to wait hash done.
        auto func = [this, info](HT ht){
            String cfile = str_to_GBEX(ht->file_path_utf8);
            PRINTLN("startUploadFromRemote >> start onPostHashTask. path, hash, ud = (%s, %s, %s)\n",
                    cfile.data(), ht->hash.data(), ht->userData.data());
            int ret = onPostHashTask(info, ht);
            StartUploadRet sret;
            sret.filepath = ht->file_path_utf8;
            sret.hash = ht->hash;
            sret.userdata = ht->userData;
            sret.code = ret;
            m_sender.sendStartUploadRet(sret);
        };
        if(checkHasHashTask(filepath, ud, func)){
            PRINTLN("startUploadFromRemote >> already has hash task. wait hash done then do it. \n");
            return kCode_WAIT;
        }
        {
        std::unique_lock<std::mutex> lock(m_mutex_hash);
        info.hash = FileUtils::sha256(file);
        *out_hash = info.hash;
        }
    }
    {
        int code = checkTaskByHash(info.hash);
        if(code != kCode_OK){
            return code;
        }
    }
    PendingTask task;
    task.hash = info.hash;
    task.filename = FileUtils::getSimpleFileName(filepath);
    task.file_enc = url_encode2(filepath);
    task.file = file;
    task.tc = tc;
    task.userData = ud;
    //
    sendFileWithUploadedInfo(info, task);
    return kCode_OK;
}

void SxClient::sendFileWithUploadedInfo(const SxReceiver::UploadedInfo& info,
                                     PendingTask& task){
    //no upload record
    if(info.blockIds.empty()){
        //remove old exe queue
        removeExeTask(info.hash);
        sendFile(task.file_enc, task.file, task.filename,
                 task.userData, task.hash, true);
        PRINTLN("sendFileWithUploadedInfo >> sendFile: %s\n",
                task.file.data());
        return;
    }
    //set old info
    task.totalLen = info.totalLen;
    task.blockSize = info.blockSize;
    task.startBlockId = info.blockIds[0];
    task.lastBlockId = info.blockIds[info.blockIds.size() - 1];
    if(info.finishBlockIds.empty()){
        //new
        task.blockIds = info.blockIds;
        task.restart = false;
        sendFileImpl(task);
        PRINTLN("sendFileWithUploadedInfo >> sendFileImpl -> new: %s\n", task.file.data());
    }else{
        //old
        task.blockIds = info.getUndoBlockIds();
        if(task.blockIds.empty()){
            {
                //no need start .we should remove task.
                removeExeTask(info.hash);
                //remove from sync
                //m_sync.removeItem(info.hash);
            }
            PRINTLN("sendFileWithUploadedInfo >> file already uploaded. hash = %s\n",
                    info.hash.data());
            {
                std::unique_lock<std::mutex> lock(m_mutex_cb);
                if(m_callback){
                    m_callback->func_upload(info.hash, "file already uploaded.");
                }
            }
        }else{
            task.alreadyBlockCount = info.finishBlockIds.size();
            task.restart = true;
            sendFileImpl(task);
            PRINTLN("sendFileWithUploadedInfo >> sendFileImpl -> continue: %s\n",
                    task.file.data());
        }
    }
}
void SxClient::onGetUploadInfo(const SxReceiver::UploadedInfo& info){

    PendingTask task;
    {
        std::unique_lock<std::mutex> lock(m_mutex_pending);
        auto it = m_pendingMap.find(info.hash);
        if(it != m_pendingMap.end()){
            task = it->second;
            m_pendingMap.erase(it);
        }else{
            PRINTLN("can't find pending task, may be paused. hash = %s.\n",
                    info.hash.data());
            return;
        }
    }
    sendFileWithUploadedInfo(info, task);
}

void SxClient::sendFileImpl0(const PendingTask& task){
    PRINTERR("sendFileImpl0 >> tc = %d, max_block_size = %lld\n", task.tc,
             task.blockSize);
    MED_ASSERT_X(task.blockSize > 0, "must set blockSize.");
    auto fname0 = FileUtils::getFileName(task.file);
    {
//        if(task.totalLen >= getAvailablePhysMemBytes() / 2){
//             PRINTLN("sendFileImpl0 >> memory not enough -> wait.\n");
//             PendingTask _task = task;
//             std::unique_lock<std::mutex> lock(m_mutex_cfc);
//             m_waitFileTasks.push_back(std::move(_task));
//             return;
//        }
        if(h_atomic_get(&m_runningCount) >= h_atomic_get(&m_concurrentFc)){
            PendingTask _task = task;
            std::unique_lock<std::mutex> lock(m_mutex_cfc);
            m_waitFileTasks.push_back(std::move(_task));
            PRINTLN("task is full >> wait tasks.\n");
            return;
        }
    }
    ETPI pi;
    {
        std::unique_lock<std::mutex> lock(m_mutex_exe);
        pi = m_exeMap[task.hash];
        if(!pi){
            PRINTLN("can't find execute info for hash = %s, may be paused.\n",
                    task.hash.data());
            return;
        }
    }
    if(h_atomic_get(&pi->paused) != 0){
        removeExeTask(task.hash);
        doProcessNextFile(task.hash);
        return;
    }
    //add running count
    h_atomic_add(&m_runningCount, 1);

    {
        FileStart fs;
        fs.blockCount = task.blockIds.size();
        fs.blockSize = task.blockSize;
        fs.hash = task.hash;
        fs.name = task.filename;
        fs.totalLen = task.totalLen;
        fs.userData = task.userData;
        fs.blockIds = task.blockIds;
        m_sender.sendFileStart(fs, task.restart);
        PRINTLN("sendFileStart >> %s\n", task.file.data());
    }
    h_atomic_set(&pi->finishCount, 0);
    pi->preFinishCount = task.alreadyBlockCount;
    pi->reqCount = task.blockIds.size();
    pi->userdata = task.userData;
    pi->fname = fname0;

    //callback if need
    {
        const int albc = task.alreadyBlockCount;
        if(albc > 0){
            std::unique_lock<std::mutex> lock(m_mutex_cb);
            if(m_callback != NULL){
                m_callback->func_progress(task.hash, albc, albc + task.blockIds.size());
            }
        }
    }
    {
        std::string file = task.file;
       // int len_ud = task.userData.length();
        std::string userData = task.userData;
        sint64 last_block_size = (task.totalLen % task.blockSize != 0) ?
                    (task.totalLen % task.blockSize) : task.blockSize;
        sint64 blockSize = task.blockSize;
        //
        PRINTLN("%s >> start send file parts: count = %d\n",
                fname0.data(), (int)task.blockIds.size());
        for(uint32 i = 0 ; i < task.blockIds.size(); ++i){
            //all bid start from 0.
            int bid = task.blockIds[i];
            sint64 offset = blockSize * (bid - task.startBlockId);
            sint64 size = bid != task.lastBlockId ? task.blockSize : last_block_size;
            //
            auto vfp = sk_make_sp<VirtualFilePart>();
            vfp->blockId = bid;
            vfp->file = file;
            vfp->hash = task.hash;
            vfp->offset = offset;
            vfp->size = size;
            vfp->userData = task.userData;
            //paused ,break
            if(h_atomic_get(&pi->paused) != 0){
                break;
            }
            m_sender.sendVirtaulFilePart(fname0 + "_" + std::to_string(i), vfp);
        }
        //
        h_atomic_add(&m_runningCount, -1);
        doProcessNextFile(task.hash);
    }

}
void SxClient::doProcessNextFile(CString hash){
    {
        //check if exist wait tasks.
        m_mutex_cfc.lock();;
        if(m_waitFileTasks.size() > 0){
            auto task = popFileTask();
            m_mutex_cfc.unlock();
            sendFileImpl(task);
        }else{
            m_mutex_cfc.unlock();
        }
    }
}

void SxClient::requireBlockIds(int count, std::vector<int>& vec){
    for(int i = 0 ; i < count ; i ++){
        vec.push_back(i);
    }
}
void SxClient::dohandleMessage(const hv::SocketChannelPtr& channel){
    SxReceiver& rec = m_rec;
    //
    switch (rec.getReqType()) {
    case kType_LOGIN:{
        auto& s = rec.getState();
        if(s.code != kCode_OK){
            PRINTERR("login failed, code = %d, msg = %s\n", s.code, s.msg.data());
            {
                std::unique_lock<std::mutex> lock(m_mutex_cb);
                if(m_callback){
                    m_callback->func_login(s.code, s.msg);
                }
            }
        }else{
            //2, check pending tasks
            h_atomic_set(&m_logined, 1);
            PRINTLN("login success.\n");
            //continue upload failed file.
            //syncUploadFailed();
            //
            {
                std::unique_lock<std::mutex> lock(m_mutex_cb);
                if(m_callback){
                    m_callback->func_login(kCode_OK, "login success");
                }
            }
        }
    }
        break;

    case kType_GET_RECOMMEND_INFO:{
        m_maxBlockSize = HMIN(m_maxBlockSize,
                              (uint32)rec.getRecommendInfo().maxBlockSize);
        hio_set_max_write_bufsize(channel->io(), m_maxBlockSize * 2 + (1 << 20));
        m_sender.setMaxWriteBufSize(m_maxBlockSize * 2);
        PRINTLN("get recommend max block size: %d\n", m_maxBlockSize);
    }break;

    case kType_GET_UPLOAD_INFO:{
        SxReceiver::UploadedInfo info = rec.getUploadedInfo();
        log_upload_info("kType_GET_UPLOAD_INFO", info);
        if(info.blockIds.size() > 0 &&
                (info.blockSize == 0 || info.totalLen == 0)){
            PRINTLN("kType_GET_UPLOAD_INFO >> param error.\n");
        }else{
            std::thread thd([this, info](){
                onGetUploadInfo(info);
            });
            thd.detach();
        }
    }break;

    case kType_UPLOAD_OK:{
        //m_sync.removeItem(rec.getFileHash());
        //PRINTLN("kType_UPLOAD_OK: left file count = %d\n", m_sync.getItemCount());
    }break;

    case kType_STOP_UPLOAD:{
        String hash = rec.getFileHash();
        String ud = rec.getUserdata();
        std::thread thd([this, hash, ud](){
            pause(hash, ud, false);
        });
        thd.detach();
    }break;

    case kType_START_UPLOAD:{
        String fp = rec.getFilePath();
        String ud = rec.getUserdata();
        auto info = rec.getUploadedInfo();

        log_upload_info("kType_START_UPLOAD", info);
        std::thread thd([this, fp, ud, info](){
             String _hash = info.hash;
             int ret = startUploadFromRemote(fp, ud, info, &_hash);
             PRINTLN("startUploadFromRemote >> state = %d, hash = %s, path = %s\n",
                     ret, info.hash.data(), fp.data());
             if(ret != kCode_WAIT){
                 //send to server ?
                 StartUploadRet sret;
                 sret.filepath = fp;
                 sret.hash = _hash;
                 sret.userdata = ud;
                 sret.code = ret;
                 m_sender.sendStartUploadRet(sret);
             }
        });
        thd.detach();
    }break;

    case kType_HEART:{
        //PRINTLN("send heart beat as res.\n");
        m_sender.sendHeartBeat();
    }break;

    case kType_PRE_FILE_INFO:{
        //nothing
    }break;

    case kType_REC_SIZE:
    {
        m_sender.reportRecSize(rec.getRecInfo().size);
    }break;

    default:
        break;
    }
}

void SxClient::syncUploadFailed(){
}

h7_sx_pro::web::UploadFileRes SxClient::_sendFile(PendingTask& task,
                                                  bool simpleTest){
    MED_ASSERT(!task.hash.empty());
    //may task is pending or running.
    auto& hash = task.hash;
    auto& userData = task.userData;
    auto& total_len = task.totalLen;
    auto& file = task.file;
    {
        std::unique_lock<std::mutex> lock(m_mutex_pending);
        if(m_pendingMap.find(hash) != m_pendingMap.end()){
            PRINTERR("already pending.\n");
            return {file, "", "already pending"};
        }
    }
    {
        std::unique_lock<std::mutex> lock(m_mutex_exe);
        auto it = m_exeMap.find(hash);
        if(it != m_exeMap.end() && it->second){
            PRINTERR("_sendFile >> already started.\n");
            return {file, "", "already started"};
        }
        m_exeMap[hash] = sk_make_sp<TaskProcessInfo>();
    }
    //get info
    if(!simpleTest){
        //add to pending
        {
            std::unique_lock<std::mutex> lock(m_mutex_pending);
            m_pendingMap[hash] = task;
        }
        //get old info
        GetUploadInfo info;
        info.hash = hash;
        info.userData = userData;
        std::thread thd([this, info](){
            PRINTLN("sendGetUploadInfo >> hash,ud = %s,%s\n", info.hash.data(), info.userData.data());
            m_sender.sendGetUploadInfo(info);
        });
        thd.detach();
        return {file, hash, "op ok. start get old info"};
    }
    sint64 last_block_size = total_len % m_maxBlockSize;
    int block_count = total_len / m_maxBlockSize + (last_block_size != 0 ? 1 : 0);
    PRINTLN("m_maxBlockSize = %u, block_count = %d, last = %lld\n",
            m_maxBlockSize, block_count, last_block_size);
    // send directly
    task.blockIds = requireBlockIds(block_count);
    task.lastBlockId = task.blockIds[task.blockIds.size()-1];
    task.blockSize = m_maxBlockSize;
    task.startBlockId = task.blockIds[0];
    task.restart = false;
    sendFileImpl(task);
    return {file, hash, "op ok. start directly."};
}

//-----------------------------------
void SxClient::stopAndClearExeTasks(){
    std::unique_lock<std::mutex> lock(m_mutex_exe);
    auto it = m_exeMap.begin();
    while(it != m_exeMap.end()){
        auto exe = it->second;
        if(exe){
            h_atomic_set(&exe->paused, 1); //mark paused
        }
        it++;
    }
    m_exeMap.clear();
}
int SxClient::checkTaskByHash(CString hash){
    //check pending
    {
        std::unique_lock<std::mutex> lock(m_mutex_pending);
        auto it = m_pendingMap.find(hash);
        if(it != m_pendingMap.end()){
            PRINTERR("[check pending] file upload already started. hash = %s.\n", hash.data());
            return kCode_ALREADY_START;
        }
    }
    //check wait tasks.
    {
        std::unique_lock<std::mutex> lock(m_mutex_cfc);
        auto it = m_waitFileTasks.begin();
        for(;it != m_waitFileTasks.end();){
            if(it->hash == hash){
                PRINTERR("[check wait] file upload already started. hash = %s.\n", hash.data());
                return kCode_ALREADY_START;
            }
            it ++;
        }
    }

    {
        std::unique_lock<std::mutex> lock(m_mutex_exe);
        auto it = m_exeMap.find(hash);
        if(it != m_exeMap.end() && it->second){
            PRINTERR("m_exeMap >>file upload already started. hash = %s.\n", hash.data());
            return kCode_ALREADY_START;
        }
        m_exeMap[hash] = sk_make_sp<TaskProcessInfo>();
    }
    return kCode_OK;
}
int SxClient::onPostHashTask(const SxReceiver::UploadedInfo& _info, HT ht){
    int tc = getTC();
    if(tc == 0){
        return kCode_MEMORY_NOT_ENOUGH;
    }
    {
        int code = checkTaskByHash(ht->hash);
        if(code != kCode_OK){
            return code;
        }
    }
    SxReceiver::UploadedInfo info = _info;
    info.hash = ht->hash;
    //
    String file = str_to_GBEX(ht->file_path_utf8);
    PendingTask task;
    task.hash = ht->hash;
    task.filename = ht->filename;
    task.file_enc = ht->file_enc;
    task.file = file;
    task.tc = tc;
    task.userData = ht->userData;
    //
    sendFileWithUploadedInfo(info, task);
    return kCode_OK;
}
bool SxClient::checkHasHashTask(CString path, CString userData, std::function<void(HT)> func){
    HT ret;
    {
        std::unique_lock<std::mutex> lck(m_mutex_ht);
        auto hash = fasthash32(userData.data(), userData.length(), DEF_HASH_HT);
        int index = m_hashTasks.indexOfHash(hash);
        if(index < 0){
            return false;
        }
        //may be multi
        for(uint32 i = 0 ; i < m_hashTasks.size() ; ++i){
            auto ht = m_hashTasks.get(i);
            if(ht->userData == userData && ht->file_path_utf8 == path){
                 ret = ht;
                 break;
            }
        }
    }
    if(ret){
        ret->onPostTask = FUNC_MAKE_SHARED_PTR_1(void(HT), func);
        return true;
    }
    return false;
}
int SxClient::getTC(){
    uint64 size = getAvailablePhysMemBytes() / 4;
    int tc = size / m_maxBlockSize;
    if(tc == 0){
        PRINTERR("send file failed by getAvailablePhysMemBytes is small (%lu).\n", size);
    }
    return HMIN(4, tc);
}
String SxClient::cal_hash(HT ht){
    PRINTLN("cal_hash >> %s\n", ht->filename.data());
    PerformanceHelper ph;
    ph.begin();
    String _file = url_decode2(ht->file_enc);
    String file = str_to_GBEX(_file);
    std::unique_lock<std::mutex> lock(m_mutex_hash);
    auto ret = FileUtils::sha256(file, nullptr, &ht->canceled);
    ph.print("cal_hash:"+ret);
    return ret;
}
