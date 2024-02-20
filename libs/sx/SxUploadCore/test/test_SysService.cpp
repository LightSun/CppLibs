#include "service/service_base.h"
#include "service/service_install.h"
 

class CTestService: public ServiceBase
{
public:
#ifdef _DEBUG
    CTestService(const CString&name)
        :ServiceBase(name,
        name,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        SERVICE_ACCEPT_STOP
        ){}
#else
    CTestService(const String& name)
        :ServiceBase(name,
        name,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        SERVICE_ACCEPT_STOP
        ){}
#endif
private:
    //服务开启时调用的接口（此接口不能阻塞，如果阻塞服务会一直显示开启中）
    void OnStart(const std::vector<String>& ps)
    {
        OutputDebugString("TestService:Application start running!!!\n");
        printf("service: OnStart");
    }
 
    //服务关闭时调用的接口（此接口不能阻塞，如果阻塞服务会一直显示关闭中）
    void OnStop()
    {
        OutputDebugString("TestService:Application end\n");
        printf("service: OnStop");
    }
public:
    //如果是以服务的方式运行就到此函数里面直接调用ServiceBase的Run函数。此函数调用完以后，服务会调用OnStart函数。
    //如果是控制台的方式运行就到此函数里面写主要的逻辑代码。此函数一般也会到内部调用OnStart和OnStop函数。此函数为主线程，所以不能退出。
    bool Run(LPCTSTR param = "")
    {
        if (strcmp(param, _T("console")) == 0)
        {
            //Todo：控制台运行处理 调用OnStart和OnStop
            
            TCHAR cinCmd[128];
            bool bStart = false;
 
            while(1)//控制台运行主线程不能退出 
            {
            printf(_T("->input cmd\r\n"));
 
            scanf_s(_T("%s"), cinCmd, 128);
            if (strncmp(cinCmd, _T("?"), 1) == 0)
            {
                printf(_T("\r\n========================================\r\n"));
                printf(_T("\"?\"     -show cmd help\r\n"));
                printf(_T("\"start\" -start service\r\n"));
                printf(_T("\"stop\"  -stop service\r\n"));
                printf(_T("\"exit\"  -exit service\r\n"));
                printf(_T("========================================\r\n"));
            }
            else if (strncmp(cinCmd, _T("start"), 5) == 0)
            {
                if (!bStart) 
                {
                OnStart({});
 
                printf(_T("\r\n========================================\r\n"));
                printf(_T("-> start service\r\n"));
                printf(_T("========================================\r\n"));
                }
                bStart = true;
            }
            else if (strncmp(cinCmd, _T("stop"), 4) == 0)
            {
                if (bStart)
                {
                OnStop();
 
                printf(_T("\n========================================\n"));
                printf(_T("-> stop service\r\n"));
                printf(_T("========================================\n"));
                }
 
                bStart = false;
            }
            else if (strncmp(cinCmd, _T("exit"), 4) == 0)
            {
 
                printf(_T("\r\n========================================\r\n"));
                printf(_T("-> exit service\r\n"));
                printf(_T("========================================\r\n"));
 
                break;
            }
            else 
            {
                printf(_T("invalid cmd\r\n"));
            }
            }
 
            if (bStart)
            OnStop();
 
            return true;
        }
 
        return ServiceBase::Run();//服务的方式运行
    }
};
 
 
int _tmain(int argc, char* argv[])
{
   // ServiceInstaller::Uninstall("TestService");
    //创建服务对象
    CTestService service("TestService_h7");
    
    //带有参数的调用
    if (argc > 1) 
    {
        if (strcmp(argv[1], _T("install")) == 0) {//安装服务
            OutputDebugString(_T("TestService:Installing service\n"));
            if (!ServiceInstaller::Install(service)) {
                OutputDebugString(_T("TestService:Couldn't install service\n"));
                return -1;
            }
 
            OutputDebugString(_T("TestService:Service installed\n"));
            return 0;
        }
 
        if (strcmp(argv[1], _T("uninstall")) == 0) {//卸载服务
            OutputDebugString(_T("TestService:Uninstalling service\n"));
            if (!ServiceInstaller::Uninstall(service)) {
                OutputDebugString(_T("TestService:Couldn't uninstall service: %d\n"));
                return -1;
            }
 
            OutputDebugString(_T("TestService:Service uninstalled\n"));
            return 0;
        }
 
        if (strcmp(argv[1], _T("console")) == 0) {//以控制台的方式运行
            OutputDebugString(_T("TestService:console running\n"));
            service.Run(_T("console"));
            return 0;
        }
    }
    else
    {//以服务的方式运行
        OutputDebugString(_T("TestService:start service"));
        bool ret = service.Run();
        printf("service.Run = %d\n", ret);
    }    
    return 0;
}
