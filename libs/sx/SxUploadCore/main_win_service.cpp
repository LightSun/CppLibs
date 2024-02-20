#include <iostream>
#include <string>
#include <stdatomic.h>
#include <thread>
#include "utils/CmdHelper.h"

#include "service/service_base.h"
#include "service/service_install.h"

using namespace std;

//#define SX_DEBUG 1

//手动启动的，能隐藏窗口（闪一下）， 可以连接ui
//重启电脑自动启动的，能隐藏窗口（闪一下）， 不可以连接ui
class SxUploadService: public ServiceBase{

private:
public:
#ifdef SX_DEBUG
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
        SERVICE_ACCEPT_STOP
        ){}
#endif
    void OnStart(const std::vector<String>& ps) override{
        OutputDebugString("SxUploadService >> OnStart...");

        String params;
        int size = ps.size();
        for(int i = 0 ; i < size ; ++i){
            params += ps[i];
            if(i != size - 1){
                params += " ";
            }
        }
       _start_SxUploadService(params);
    };
    void OnStop() override{
        OutputDebugString("SxUploadService >> OnStop...");
    }
private:
    void _start_SxTray_direct(const String& params){
        String dir = "C:/heaven7/sx/build-SxUploadCore-Mingw_64-Release/";
        String cmd = dir + "SxTray.exe " + params;
        String ret;
        int code = h7::CmdHelper::execCmd(cmd.data(), ret);
        fprintf(stderr, "code = %d. ret = %s\n", code, ret.data());
        OutputDebugString(ret.data());
    }
    void _start_SxTray(const String& params){
        String dir = "C:/heaven7/sx/build-SxUploadCore-Mingw_64-Release/";
        bool ret = StartAppAsUser(dir + "SxTray.exe", params);
        String retStr = "OnStart >>ret = " + std::to_string(ret);
        OutputDebugString(retStr.data());
    }

    void _start_SxUploadService(const String& params){
        String dir = "C:/heaven7/sx/build-SxUploadCore-Mingw_64-Release/";
        bool ret = StartAppAsUser(dir + "SxUploadService.exe", params);
        String retStr = "OnStart >>ret = " + std::to_string(ret);
        OutputDebugString(retStr.data());
    }
};


//login, continue upload
int main(int argc, char* argv[])
{
    setbuf(stdout, NULL);

    SxUploadService service("SxWinService");

    if (argc > 1){
        if (strcmp(argv[1], _T("install")) == 0) {
            OutputDebugString(_T("SxUploadService:Installing service\n"));
            if (!ServiceInstaller::Install(service)) {
                OutputDebugString(_T("SxUploadService:Couldn't install service\n"));
                return -1;
            }

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
