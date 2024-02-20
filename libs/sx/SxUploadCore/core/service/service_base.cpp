//#include stdafx.h
#include "service_base.h"
#include <cassert>
#include <vector>

#include <userenv.h>
 
ServiceBase* ServiceBase::m_service = nullptr;
 
ServiceBase::ServiceBase(const String& name,
    const String& displayName,
    DWORD dwStartType,
    DWORD dwErrCtrlType,
    DWORD dwAcceptedCmds,
    const String& depends,
    const String& account,
    const String& password):
     m_name(name),
    m_displayName(displayName),
    m_dwStartType(dwStartType),
    m_dwErrorCtrlType(dwErrCtrlType),
    m_depends(depends),
    m_account(account),
    m_password(password),
    m_svcStatusHandle(nullptr) 
{  
 
        m_hasDepends = !m_depends.empty();
        m_hasAcc = !m_account.empty();
        m_hasPass = !m_password.empty();
        m_svcStatus.dwControlsAccepted = dwAcceptedCmds;
        m_svcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
        m_svcStatus.dwCurrentState = SERVICE_START_PENDING;
        m_svcStatus.dwWin32ExitCode = NO_ERROR;
        m_svcStatus.dwServiceSpecificExitCode = 0;
        m_svcStatus.dwCheckPoint = 0;
        m_svcStatus.dwWaitHint = 0;
}
 
void ServiceBase::SetStatus(DWORD dwState, DWORD dwErrCode, DWORD dwWait)
{
    ReportSvcStatus(dwState, dwErrCode, dwWait);
//    m_svcStatus.dwCurrentState = dwState;
//    m_svcStatus.dwWin32ExitCode = dwErrCode;
//    m_svcStatus.dwWaitHint = dwWait;
//    SetServiceStatus(m_svcStatusHandle, &m_svcStatus);
}
 
void ServiceBase::WriteToEventLog(const String& msg, WORD type)
{
    HANDLE hSource = RegisterEventSource(nullptr, m_name.data());
    if (hSource) 
    {
        LPCTSTR msgData[2] = {m_name.data(), msg.data()};
        ReportEvent(hSource, type, 0, 0, nullptr, 2, 0, msgData, nullptr);
        DeregisterEventSource(hSource);
    }
}
 
void WINAPI ServiceBase::SvcMain(DWORD argc, char* argv[])
{
    printf("ServiceBase::SvcMain >> \n");
    assert(m_service);
    //register
    m_service->m_svcStatusHandle = RegisterServiceCtrlHandlerEx(m_service->GetName().data(),
                                                                ServiceCtrlHandler, NULL);
    if (!m_service->m_svcStatusHandle)
    {
        m_service->WriteErrorEvent("Can't set service control handler");
        return;
    }
    std::vector<String> vec;
    for(int i = 0 ; i < argc ; i ++){
        vec.push_back(argv[i]);
    }
    m_service->Start2(vec);
}
 
void WINAPI ServiceBase::SvcMain2(DWORD argc, wchar_t* argv[]){
    printf("ServiceBase::SvcMain2 >> argc = %d\n", argc);
    assert(m_service);
    //register
    m_service->m_svcStatusHandle = RegisterServiceCtrlHandlerEx(m_service->GetName().data(),
                                                                ServiceCtrlHandler, NULL);
    if (!m_service->m_svcStatusHandle)
    {
        m_service->WriteToEventLog("Can't set service control handler", EVENTLOG_ERROR_TYPE);
        return;
    }
    if(argc > 0){
        std::vector<String> vec;
        for(int i = 0 ; i < argc ; i ++){
            vec.push_back(WcharToChar(argv[i]));
        }
        m_service->Start(vec);
    }else{
        m_service->Start({});
    }
     m_service->WriteToEventLog("start sx_SvcMain2 ok.", EVENTLOG_SUCCESS);
}
DWORD WINAPI ServiceBase::ServiceCtrlHandler(DWORD ctrlCode, DWORD evtType, void* evtData, void* context)
{
    printf("ServiceCtrlHandler >> %d \n", ctrlCode);
    switch (ctrlCode) 
    {
    case SERVICE_CONTROL_STOP: //if there is empty. service will not be stooped normally.
        m_service->Stop();
        break;
 
    case SERVICE_CONTROL_PAUSE:
        m_service->Pause();
        break;
 
    case SERVICE_CONTROL_CONTINUE:
        m_service->Continue();
        break;
 
    case SERVICE_CONTROL_SHUTDOWN:
        m_service->Shutdown();
        break;
 
    case SERVICE_CONTROL_SESSIONCHANGE:
        m_service->OnSessionChange(evtType, (WTSSESSION_NOTIFICATION*)(evtData));
        break;
 
    default:
        break;
    }
 
    return 0;
}
 
bool ServiceBase::RunInternal(ServiceBase* svc)
{
    m_service = svc;
    LPSTR svcName = (LPSTR)m_service->GetName().data();

//    SERVICE_TABLE_ENTRYW tableEntry[] =
//    {
//        {(LPWSTR)svcName, SvcMain2},
//        {nullptr, nullptr}
//    };

//    auto ret = StartServiceCtrlDispatcherW(tableEntry);
 
    SERVICE_TABLE_ENTRY tableEntry[] =
    {
        {svcName, SvcMain},
        {nullptr, nullptr}
    };

    auto ret = StartServiceCtrlDispatcher(tableEntry);
    if(!ret){
        printf("StartServiceCtrlDispatcher: failed. code = %d\n", ::GetLastError());
    }
    return ret;
}
 
void ServiceBase::Start(const std::vector<String>& ps){
    SetStatus(SERVICE_START_PENDING);
    OnStart(ps);
    SetStatus(SERVICE_RUNNING);
}
void ServiceBase::Start2(const std::vector<String>& ps)
{
    // Report initial status to the SCM
    ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000 );
    //
    m_ghSvcStopEvent = CreateEvent(
                             NULL,    // default security attributes
                             TRUE,    // manual reset event
                             FALSE,   // not signaled
                             NULL);   // no name

    if (m_ghSvcStopEvent == NULL)
    {
        ReportSvcStatus(SERVICE_STOPPED, GetLastError(), 0 );
        return;
    }
    SetStatus(SERVICE_RUNNING);
    OnStart(ps);
    while(1)
    {
        // Check whether to stop the service.

        WaitForSingleObject(m_ghSvcStopEvent, INFINITE);
        ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0 );
        return;
    }
}
 
void ServiceBase::Stop()
{
    if(m_ghSvcStopEvent == nullptr){
        SetStatus(SERVICE_STOP_PENDING);
        OnStop();
        SetStatus(SERVICE_STOPPED);
    }else{
        SetStatus(SERVICE_STOP_PENDING);

        OnStop();
        // Signal the service to stop.
        SetEvent(m_ghSvcStopEvent);
        SetStatus(SERVICE_STOPPED);
    }
}
 
void ServiceBase::Pause()
{
    SetStatus(SERVICE_PAUSE_PENDING);
    OnPause();
    SetStatus(SERVICE_PAUSED);
}
 
void ServiceBase::Continue()
{
    SetStatus(SERVICE_CONTINUE_PENDING);
    OnContinue();
    SetStatus(SERVICE_RUNNING);
}
 
void ServiceBase::Shutdown()
{
    OnShutdown();
    SetStatus(SERVICE_STOPPED);
}

//----------------------------------------


bool ServiceBase::StartAppAsUser(const String& strAppPath, const String& params)
{
    HANDLE hToken = NULL;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken))
    {
        DWORD nErr = GetLastError();
        fprintf(stderr, "OpenProcessToken >> error code = %d\n", nErr);
        return false;
    }

    HANDLE hTokenDup = NULL;
    bool bRet = DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, NULL, SecurityIdentification, TokenPrimary, &hTokenDup);
    if (!bRet || hTokenDup == NULL)
    {
        DWORD nErr = GetLastError();
        fprintf(stderr, "DuplicateTokenEx >> error code = %d\n", nErr);
        CloseHandle(hToken);
        return false;
    }

    DWORD dwSessionId = WTSGetActiveConsoleSessionId();
    //把服务hToken的SessionId替换成当前活动的Session(即替换到可与用户交互的winsta0下)
    if (!SetTokenInformation(hTokenDup, TokenSessionId, &dwSessionId, sizeof(DWORD)))
    {
        DWORD nErr = GetLastError();
        fprintf(stderr, "SetTokenInformation >> error code = %d\n", nErr);
        CloseHandle(hTokenDup);
        CloseHandle(hToken);
        return false;
    }

    STARTUPINFO si;
    ZeroMemory(&si, sizeof(STARTUPINFO));

    String _lpDesktop = "WinSta0\\Default";

    si.cb = sizeof(STARTUPINFO);
    si.lpDesktop = _lpDesktop.data();
    si.wShowWindow = SW_SHOW;
    si.dwFlags = STARTF_USESHOWWINDOW /*|STARTF_USESTDHANDLES*/;

    //创建进程环境块
    LPVOID pEnv = NULL;
    bRet = CreateEnvironmentBlock(&pEnv, hTokenDup, FALSE);
    if (!bRet)
    {
        DWORD nErr = GetLastError();
        fprintf(stderr, "CreateEnvironmentBlock >> error code = %d\n", nErr);
        CloseHandle(hTokenDup);
        CloseHandle(hToken);
        return false;
    }

    if (pEnv == NULL)
    {
        CloseHandle(hTokenDup);
        CloseHandle(hToken);
        return false;
    }

    //在活动的Session下创建进程
    PROCESS_INFORMATION processInfo;
    ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));
    DWORD dwCreationFlag = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT;

    if (!CreateProcessAsUser(hTokenDup, (char*)strAppPath.data(), (char*)params.data(),
                             NULL, NULL, FALSE, dwCreationFlag, pEnv, NULL, &si, &processInfo))
    {
        DWORD nErr = GetLastError();
        fprintf(stderr, "CreateProcessAsUser >> error code = %d\n", nErr);
        CloseHandle(hTokenDup);
        CloseHandle(hToken);
        return false;
    }

    DestroyEnvironmentBlock(pEnv);
    CloseHandle(hTokenDup);
    CloseHandle(hToken);

    return true;
}

bool ServiceBase::StartApp(const String& strAppPath, const String& params){
    STARTUPINFOA  strStartInfo;
    ZeroMemory(&strStartInfo, sizeof(strStartInfo));
    PROCESS_INFORMATION procInfo;
    ZeroMemory(&procInfo, sizeof(procInfo));

    //CREATE_UNICODE_ENVIRONMENT
    DWORD dwCreationFlag = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT;
    int bRet = CreateProcessA((char*)strAppPath.data(), (char*)params.data(),NULL, NULL, FALSE,
                   dwCreationFlag,  NULL, NULL, &strStartInfo, &procInfo);
    if (bRet){
        WaitForSingleObject(procInfo.hProcess, 4000);
        return true;
    }else{
        fprintf(stderr, "StartApp >> createProcess error %d!\n", GetLastError());
    }
    return false;
}

ServiceBase::String ServiceBase::getCurrentExePath(){
    char modulePath[MAX_PATH];

    if (::GetModuleFileName(nullptr, modulePath, MAX_PATH) == 0)
    {
        printf("Couldn't get module file name: %d\n", ::GetLastError());
        return "";
    }
    return modulePath;
}
void ServiceBase::ReportSvcStatus(DWORD dwCurrentState,
                          DWORD dwWin32ExitCode,
                     DWORD dwWaitHint){
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.
    m_svcStatus.dwCurrentState = dwCurrentState;
    m_svcStatus.dwWin32ExitCode = dwWin32ExitCode;
    m_svcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        m_svcStatus.dwControlsAccepted = 0;
    else m_svcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ( (dwCurrentState == SERVICE_RUNNING) ||
           (dwCurrentState == SERVICE_STOPPED) )
        m_svcStatus.dwCheckPoint = 0;
    else m_svcStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the SCM.
    SetServiceStatus(m_svcStatusHandle, &m_svcStatus );
}

