#include <iostream>
#include <string>
//#include "breakpad/client/windows/handler/exception_handler.h"


#include "SxWebServer.h"
#include "utils/FileUtils.h"
#include "utils/string_utils.hpp"
#include "utils/Properties.hpp"
//#include "ui_upload/pub_api.h"
#include "service/service_base.h"

extern void do_crash();

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>

LONG WINAPI dump_0(EXCEPTION_POINTERS* exp)
{
    SYSTEMTIME time;
    GetLocalTime(&time);

    auto process = GetCurrentProcess();
    auto processId = GetCurrentProcessId();
    auto threadId = GetCurrentThreadId();

    char szFileName[MAX_PATH];
    snprintf(szFileName, MAX_PATH, "%s-%04d-%02d%02d-%02d%02d.dmp",
                     "SxUploadService", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);

    HANDLE hDumpFile = CreateFileA((char*)szFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ,
                                  0, CREATE_ALWAYS, 0, 0);


    MINIDUMP_EXCEPTION_INFORMATION ExpParam;

    ExpParam.ThreadId = threadId;
    ExpParam.ExceptionPointers = exp;
    ExpParam.ClientPointers = TRUE;

    bool bMiniDumpSuccessful = MiniDumpWriteDump(GetCurrentProcess(), processId, hDumpFile,
                                                 MiniDumpWithDataSegs, &ExpParam, nullptr, nullptr);
    fprintf(stderr, "MiniDumpWriteDump = %d \n", bMiniDumpSuccessful);
    return EXCEPTION_EXECUTE_HANDLER;
}

static inline void reg_excep(){
    SetUnhandledExceptionFilter(dump_0);
}
#endif


using namespace std;
using namespace h7;

static void test_SxClient();
static void test_temp_tcp();
static int runService(CString addr, int max_block);
static int runService2(CString addr, int max_block);

//bool MinidumpCallback0(const wchar_t* dump_path,
//                                  const wchar_t* minidump_id,
//                                  void* context,
//                                  EXCEPTION_POINTERS* exinfo,
//                                  MDRawAssertionInfo* assertion,
//                         bool succeeded){
//    return true;
//}


//#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" ) // 设置入口地址 , not ok
int main(int argc, char* argv[])
{
#ifdef _WIN32
//    HWND hwnd;
//    hwnd = FindWindow("ConsoleWindowClass", NULL);	//处理顶级窗口的类名和窗口名称匹配指定的字符串,不搜索子窗口。
//    if(hwnd)
//    {
//        ShowWindow(hwnd, SW_HIDE);
//    }
    reg_excep();
#endif
    setbuf(stdout, NULL);

   // do_crash();

//    google_breakpad::ExceptionHandler *pCrashHandel =
//           new google_breakpad::ExceptionHandler(
//           L".",
//           NULL,
//           MinidumpCallback0,
//           NULL,
//           google_breakpad::ExceptionHandler::HANDLER_ALL,NULL);


    std::string addr = "10.0.0.164";

    int max_block = 100 << 20; //100M
    if(argc > 1){
        addr = argv[1];
        if(argc > 2){
            max_block = std::stoi(argv[2]);
        }
    }else{
        String path = ServiceBase::getCurrentExePath();
        auto dir = FileUtils::getFileDir(path);
        String cfg = dir + "/cfg.prop";
        if(!FileUtils::isFileExists(cfg)){
            String msg = "config file not exist: " + cfg;
            MED_ASSERT_X(false, msg);
            return 1;
        }
        Properties prop;
        prop.loadFile(cfg);
        addr = prop.getString("addr");
        if(addr.empty()){
            MED_ASSERT_X(false, "addr is not exist in config file.");
            return 1;
        }
        String blockStr = prop.getString("max_block");
        if(!blockStr.empty()){
            max_block = std::stoi(blockStr);
        }
        printf("read config file success.\n");
    }
    PRINTLN("run addr: %s\n", addr.data());
    //test_temp_tcp();
    return runService(addr, max_block);
}

//struct RunContext{
//    med_api::ShowUploadHelper helper;
//    sk_sp<SxClient> client;

//    RunContext(CString addr, int max_block){
//        helper.init();
//        client = sk_make_sp<SxClient>(addr, 12390, max_block);
//    }
//};

//int runService2(CString addr, int max_block){
//    RunContext* rc = new RunContext(addr, max_block);
//    std::thread([rc](){
//        while (true) {
//            sk_sp<SxWebServer> server = sk_make_sp<SxWebServer>();
//            PRINTLN("---- new server ---- %p\n", server.get());
//            server->setShowUploadHelper(&rc->helper);
//            server->setUploadClient(rc->client);
//            server->startService();
//            hv_msleep(200);
//           //PRINTLN("---> server end: ref c = %u\n", server->getRefCnt());
//        }
//    }).detach();
//    return rc->helper.exec();
//}

int runService(CString addr, int max_block){
   // med_api::ShowUploadHelper helper;
   // helper.init();
    auto _client = sk_make_sp<SxClient>(addr, 12390, max_block);
    while (true) {
        sk_sp<SxWebServer> server = sk_make_sp<SxWebServer>();
        PRINTLN("---- new server ---- %p\n", server.get());
        //server->setShowUploadHelper(&helper);
        server->setUploadClient(_client);
        server->startService();
        hv_msleep(200);
       //PRINTLN("---> server end: ref c = %u\n", server->getRefCnt());
    }
    return 0;
}

using namespace hv;
void test_temp_tcp(){
    String addr = "10.0.0.79";
    hv::TcpClient m_client;
    SxReceiver m_rec;
    SxReceiver m_rec2;

    int connfd = m_client.createsocket(12390, addr.data());
    if (connfd < 0) {
        return;
    }
    m_client.onMessage = [&m_rec, &m_rec2](const SocketChannelPtr& channel,
            Buffer* buf) {
        //PRINTLN("onMessage>> %.*s\n", (int)buf->size(), (char*)buf->data());
        PRINTLN("onMessage: size = %d\n", buf->size());

        String data((char*)buf->data(), buf->size());
        //char* data = (char*)buf->data();
        //int len = *(int*)(data.data());
        //int reqType = *(int*)(data.data() + sizeof (int));

        //PRINTLN(">> len = %d, reqType = %d.\n", len, reqType);

        m_rec.putData(data.data(), data.size());
        while (true) {
            if(!m_rec.read()){
                //data not enough
                break;
            }
            PRINTLN("m_rec >> reqType = %d.\n" , m_rec.getReqType());
        }

        m_rec2.putData(data.data(), data.size());
        while (true) {
            if(!m_rec2.read()){
                //data not enough
                break;
            }
            PRINTLN("m_rec2 >> reqType = %d.\n" , m_rec2.getReqType());
        }
    };
    m_client.onConnection = [](const SocketChannelPtr& channel){
        if(channel->isConnected()){
            PRINTLN(">> connected.\n");
        }else{
            PRINTLN(">> disconnected.\n");
        }
    };
    reconn_setting_t m_reconn;
    reconn_setting_init(&m_reconn);
    m_reconn.min_delay = 1000;
    m_reconn.max_delay = 30000;
    m_reconn.delay_policy = 2;
    m_client.setReconnect(&m_reconn);
    m_client.start(true);

    //
    std::string str;
    while (getline(std::cin, str)) {

    }
}

void test_SxClient(){
    //String file = "/media/heaven7/Elements/shengxin/R/RData/hg19/clinvarTrans1.txt";
    SxClient* sc = new SxClient("127.0.0.1", 12390, 1024*20); //20kb
    MED_ASSERT_X(sc->startService(""), "start service failed.");

    std::string str;
    while (getline(std::cin, str)) {
        std::vector<String> vec = utils::split(":", str);
        if(vec.size() == 0){
            PRINTERR("no content\n");
            continue;
        }
        auto& cmd = vec[0];
        if (cmd == "upload") {
            PRINTLN("upload: file = %s\n", vec[1].data());
            String& file = vec[1];
            std::string usedData = vec.size() > 2 ? vec[2] : "";
            h7_sx_pro::web::UploadFileRes res = sc->sendFile(file, file, "", usedData);
            if(res.hash.empty()){
                PRINTERR("upload file failed: %s\n", res.msg.data());
            }else{
                PRINTERR("%s\n", res.msg.data());
            }
        } else if (cmd == "pause") {
            std::string& hash = vec[1];
            PRINTLN("pause: hash = %s\n", hash.data());
            std::string usedData = vec.size() > 2 ? vec[2] : "";
            sc->pause(hash, usedData);
        } else if (cmd == "stop") {
            sc->stopService();
            delete sc;
            break;
        }else{
            PRINTERR("wrong str:%s\n", str.data());
        }
    }
}

static inline void readBigFile(CString rfile, std::vector<char>& _buffer){
    FILE* stream_in = fopen(rfile.data(), "rb");
    fseeko64(stream_in, 0, SEEK_END);
    uint64 _size = ftello64(stream_in);
    _buffer.resize(_size);
    fseeko64(stream_in, 0, SEEK_SET);
    fread(_buffer.data(), 1, _buffer.size(), stream_in);
    fclose(stream_in);
}
