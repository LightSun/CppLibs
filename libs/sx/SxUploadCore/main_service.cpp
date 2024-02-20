#include <iostream>
#include <string>
#include <stdatomic.h>
#include <fstream>

#include "SxWebServer.h"
#include "utils/FileUtils.h"
#include "utils/string_utils.hpp"
#include "utils/FileUtils.h"
#include "utils/Properties.hpp"
//#include "ui_upload/pub_api.h"
#include "service/service_base.h"
#include "service/service_install.h"

using namespace std;
using namespace h7;

static void runService(CString addr, int max_block);

static inline void reg_excep();
static bool createDir(String& dir);
//------------

class SxUploadService: public ServiceBase{

private:
    std::atomic_bool mReqStop {false};
    sk_sp<SxClient>  mClient;
    sk_sp<SxWebServer> mServer;
public:
#ifdef _DEBUG
    SxUploadService(const String& name)
        :ServiceBase(name,
        name,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        SERVICE_ACCEPT_STOP
        ){}
#else
    SxUploadService(const String& name)
        :ServiceBase(name,
        name,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        SERVICE_ACCEPT_STOP,
         "Tcpip"
        ){
    }
#endif
    void OnStart(const std::vector<String>& ps) override{
        OutputDebugString("SxUploadService >> OnStart...");

        mReqStop = false;
        std::thread thd([this, ps](){
            Sleep(2500);
            String _dir = "C:\\WINDOWS\\system32\\config\\systemprofile\\Desktop";
            createDir(_dir);
            _start_service0(ps);
        });
        thd.detach();
    };
    void OnStop() override{
        OutputDebugString("SxUploadService >> OnStop...");
        mReqStop = true;
        if(mServer){
            mServer->stopService();
            mServer = nullptr;
        }
        if(mClient){
            mClient->stopService();
            mClient = NULL;
        }
    }

private:
    void _start_service0(const std::vector<String>& ps){
        std::string addr = "10.0.0.164";
        int max_block = 100 << 20; //100M
        String path = getCurrentExePath();
        if(path.empty()){
            if(ps.size() > 0){
                addr = ps[0];
            }
            if(ps.size() > 1){
                max_block = std::stoi(ps[1]);
            }
        }else{
            auto dir = FileUtils::getFileDir(path);
            String cfg = dir + "/cfg.prop";
            if(!FileUtils::isFileExists(cfg)){
                String msg = "config file not exist: " + cfg;
                WriteToEventLog(msg, EVENTLOG_ERROR_TYPE);
                return;
            }
            Properties prop;
            prop.loadFile(cfg);
            addr = prop.getString("addr");
            if(addr.empty()){
                WriteToEventLog("addr is not exist in config file.", EVENTLOG_ERROR_TYPE);
                return;
            }
            String blockStr = prop.getString("max_block");
            if(!blockStr.empty()){
                max_block = std::stoi(blockStr);
            }
            WriteToEventLog("read config file success.", EVENTLOG_SUCCESS);
        }
        _runService(addr, max_block);
    }
    void _runService(CString addr, int max_block){
        //med_api::ShowUploadHelper helper;
        //helper.init();
        mClient = sk_make_sp<SxClient>(addr, 12390, max_block);
        while (!mReqStop) {
            mServer = sk_make_sp<SxWebServer>();
            PRINTLN("---- new server ---- %p\n", mServer.get());
            //mServer->setShowUploadHelper(&helper);
            mServer->setUploadClient(mClient);
            mServer->startService();
            hv_msleep(200);
           //PRINTLN("---> server end: ref c = %u\n", server->getRefCnt());
        }
    }
};

//qt ui need:
//login, continue upload
int main(int argc, char* argv[])
{
#ifdef _WIN32
    HWND hwnd;
    hwnd = FindWindow("ConsoleWindowClass", NULL);
    if(hwnd)
    {
        ShowWindow(hwnd, SW_HIDE);
    }
#endif

#ifdef _WIN32
    reg_excep();
#endif
    setbuf(stdout, NULL);

    SxUploadService service("SxUploadWinService");

    if (argc > 1){
        if (strcmp(argv[1], _T("install")) == 0) {
            OutputDebugString(_T("SxUploadService:Installing service\n"));
            if (!ServiceInstaller::Install(service)) {
                OutputDebugString(_T("SxUploadService:Couldn't install service\n"));
                //mkdir C:\WINDOWS\system32\config\systemprofile\Desktop
                String _dir = "C:\\WINDOWS\\system32\\config\\systemprofile\\Desktop";
                createDir(_dir);
                return -1;
            }
            ServiceInstaller::start(service.GetName());
            OutputDebugString(_T("SxUploadService:Service installed\n"));
            return 0;
        }

        if (strcmp(argv[1], _T("uninstall")) == 0) {
            OutputDebugString(_T("SxUploadService:Uninstalling service\n"));
            if (!ServiceInstaller::Uninstall(service)) {
                OutputDebugString(_T("SxUploadService:Couldn't uninstall service: %d\n"));
                return -1;
            }

            OutputDebugString(_T("SxUploadService:Service uninstalled\n"));
            return 0;
        }
    }
    else
    {
        //use as service
        OutputDebugString(_T("SxUploadService:start service"));
        bool ret = service.Run();
        printf("service.Run = %d\n", ret);
    }
    return 0;
}

void runService(CString addr, int max_block){
    //med_api::ShowUploadHelper helper;
    //helper.init();
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
}

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
bool createDir(String& path){
    char last_char;
    last_char = path.back();
    if (last_char != '\\')
    {
       path += '\\';
    }
    PCSTR pcPath = path.c_str();
    bool result = MakeSureDirectoryPathExists(pcPath);
    return result;
}
#endif

