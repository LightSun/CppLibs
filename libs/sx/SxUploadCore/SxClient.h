#ifndef SXCLIENT_H
#define SXCLIENT_H

#include "common/SkRefCnt.h"
#include <list>
#include "SxSender.h"
#include "utils/Executor.h"
#include "utils/ByteBuffer.hpp"
#include "utils/url_encode.h"
#include "utils/SortedList.h"
#include "sx_web_protocol.h"
#include "SxReceiver.h"

#include "FilesSync.hpp"

namespace h7 {
class SxClient: public SkRefCnt
{
public:
    struct Callback{
        std::function<void(bool)> func_connn;
        std::function<void(int, CString)> func_login;//code = 0 means ok.
        std::function<void(CString, CString)> func_upload;
        std::function<void(CString, int, int)> func_progress;
    };
    //blockSize must <= int_max
    SxClient(CString addr, int port, uint32 blockSize);
    ~SxClient(){
        MED_ASSERT_X(!m_client.isConnected(), "stopService must called before "
                                              "release SxClient !");
    }
    /**
     * @brief sendFile send file to server. if ok, return the file hash. or else return 0.
     * @param file_enc the file path with url encode
     * @param file the file to upload
     * @param filename the simple filename
     * @param userData the user data
     * @param hash: file hash ,can be empty
     * @param simpleTest true if simple test. that mean without query old info
     * @return res
     */
    h7_sx_pro::web::UploadFileRes sendFile(CString file_enc, CString file,
                                           CString filename, CString userData,
                                           CString hash = "", bool simpleTest = false);

    h7_sx_pro::web::UploadFileRes sendFileEncWithHash(CString file_enc, CString hash,
                                           CString userData, bool* file_exist);

    bool startService(CString token="");
    void stopService();

    bool isConnected(){
        return m_client.isConnected();
    }
    bool isLogined();

    void pause(CString hash, CString userdata, bool sendServer = true);

    void setCallback(Callback* cb){
        std::unique_lock<std::mutex> lock(m_mutex_cb);
        this->m_callback = cb;
    }
    void setTaskDelay(uint64 delay){
        this->m_taskDelay = delay;
    }
    void setConcurrentFileCount(int c){
        if(c > 0){
            h_atomic_set(&m_concurrentFc, HMIN(c, 10));
        }
    }
    int getExeTaskCount(){
        std::unique_lock<std::mutex> lock(m_mutex_exe);
        return m_exeMap.size();
    }

    h7_sx_pro::web::UploadFileRes checkState(CString file_enc);

    void addHashTask(CString file_enc, CString filename,
                     CString userData, bool simpleTest = false);

    void addHashOnlyTask(CString file_enc, CString filename,
                     CString userData);

    String sendFileInfoOnly(CString file_enc, CString hash,
                          CString userData);

    static void post(std::function<void()> func);

private:
    struct HashTask{
        String file_enc;
        String file_path_utf8;
        String filename;
        String userData;
        String hash;
        bool simpleTest {false};
        volatile int shouldUpload {1};
        volatile int canceled {0};
        FUNC_SHARED_PTR(void(std::shared_ptr<HashTask>)) onPostTask;
    };
    struct PendingTask{
        String filename;  //simple filename. like '1.mp4'
        String file_enc;  //url encode path
        std::string hash;
        sint64 blockSize {0};
        sint64 totalLen;
        int lastBlockId {-1};
        int tc;

        int startBlockId;
        int alreadyBlockCount{0};//tmp, the already upload block count
        bool restart {false};    //tmp
        std::string userData;
        std::string file;         //platform filepath
        std::vector<int> blockIds;//to upload
    };
    struct TaskProcessInfo : public SkRefCnt{
        String fname;
        String userdata;
        int preFinishCount {0}; // the finish count of previous
        int reqCount {0};
        volatile int finishCount {0};
        volatile int paused {0};
        //add modify time for latter remove?
    };

    using HT = std::shared_ptr<HashTask>;
    using ETPI = sk_sp<TaskProcessInfo>;
    int m_port;
   // volatile int m_id {0};
    volatile int m_logined {0};
    volatile int m_reqStop {0};
    uint32 m_maxBlockSize;
    uint64 m_taskDelay {0};
    std::string m_addr;
    hv::TcpClient m_client;
    reconn_setting_t m_reconn;
    SxSender m_sender;
    SxReceiver m_rec;
    //
    std::map<String, ETPI> m_exeMap;
    std::mutex m_mutex_exe;
    //
    std::mutex m_mutex_pending;
    std::map<String, PendingTask> m_pendingMap;
    //
    Callback* m_callback {nullptr};
    std::mutex m_mutex_cb;
    //pending file tasks by concurrent
    std::list<PendingTask> m_waitFileTasks;
    std::mutex m_mutex_cfc;
    //
    //concurrent file count
    volatile int m_concurrentFc {4};
    volatile int m_runningCount {0};

    std::mutex m_mutex_hash;
    //FilesSync m_sync {"sx.med"};
    //hash task
    SortedList<HT> m_hashTasks;
    std::mutex m_mutex_ht;
    std::condition_variable m_cv_ht;

private:
    h7_sx_pro::web::UploadFileRes sendFileEnc(CString file_enc,
                                              CString filename,
          CString userData, CString hash, bool simpleTest);

    String cal_hash(HT ht);

    HT nextHashTask(){
        std::unique_lock<std::mutex> lck(m_mutex_ht);
        if(m_hashTasks.size() > 0){
            return m_hashTasks.get(0);
        }
        return nullptr;
    }
    //return true if has. and if has, the func will be run.
    bool checkHasHashTask(CString path, CString ud, std::function<void(HT)> func);

    int getTC();
    h7_sx_pro::web::UploadFileRes _sendFile(PendingTask& task,
                                            bool simpleTest);

    std::vector<int> requireBlockIds(int count){
        std::vector<int> vec;
        requireBlockIds(count, vec);
        return vec;
    }
    PendingTask popFileTask(){
        auto task = m_waitFileTasks.front();
        m_waitFileTasks.pop_front();
        return task;
    }
    void requireBlockIds(int count, std::vector<int>& vec);

    void sendFileImpl(const PendingTask& task){
        std::thread thd([this, task](){
            sendFileImpl0(task);
        });
        thd.detach();
    }
    void sendFileImpl0(const PendingTask& task);

    void sendFileWithUploadedInfo(const SxReceiver::UploadedInfo& info,
                                        PendingTask& task);

    void onGetUploadInfo(const SxReceiver::UploadedInfo& info);
    void doProcessNextFile(CString hash);

    void dohandleMessage(const hv::SocketChannelPtr& channel);
    void syncUploadFailed();
    int startUploadFromRemote(CString filepath,CString ud,
                               const SxReceiver::UploadedInfo& info, String* out_hash);

    void removeExeTask(CString hash){
        std::unique_lock<std::mutex> lock(m_mutex_exe);
        auto it = m_exeMap.find(hash);
        if(it != m_exeMap.end()){
            m_exeMap.erase(it);
        }
        PRINTLN("removeExeTask >> exe task count = %d\n",m_exeMap.size());
    }
    void stopAndClearExeTasks();
    int onPostHashTask(const SxReceiver::UploadedInfo& info, HT ht);
    int checkTaskByHash(CString hash);
};
}

#endif // SXCLIENT_H
