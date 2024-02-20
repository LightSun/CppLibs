#ifndef SERVICE_BASE_H_
#define SERVICE_BASE_H_
 
#include <windows.h>
//#include <atlstr.h>
#include <string>
#include <vector>

#include "service/copy_disabler.h"
 
// Base Service class used to create windows services.
class ServiceBase 
{
public:
    //DISABLE_COPY_MOVE(ServiceBase);
    using String = std::string;
   // using String = std::string;
 
    virtual ~ServiceBase() {}
 
    // Called by windows when starting the service.
    bool Run() 
    {
        return RunInternal(this);
    }


    static String getCurrentExePath();
    static bool StartAppAsUser(const String& strAppPath, const String& params);
    static bool StartApp(const String& strAppPath, const String& params);
 
    const String& GetName() const { return m_name; }
    const String& GetDisplayName() const { return m_displayName; }
    const DWORD GetStartType() const { return m_dwStartType; }
    const DWORD GetErrorControlType() const { return m_dwErrorCtrlType; }
    const String& GetDependencies() const { return m_depends; }
 
    // Account info service runs under.
    const String& GetAccount() const { return m_account; }
    const String& GetPassword() const { return m_password; }
protected:
    ServiceBase(const String& name,
        const String& displayName,
        DWORD dwStartType,
        DWORD dwErrCtrlType = SERVICE_ERROR_NORMAL,
        DWORD dwAcceptedCmds = SERVICE_ACCEPT_STOP,
        const String& depends = "",
        const String& account =  "",
        const String& password =  "");
 
    void SetStatus(DWORD dwState, DWORD dwErrCode = NO_ERROR, DWORD dwWait = 0);
 
    // TODO(Olster): Move out of class/make static.
    // Writes |msg| to Windows event log.
    void WriteToEventLog(const String& msg, WORD type = EVENTLOG_INFORMATION_TYPE);
    void WriteErrorEvent(const String& msg){
        WriteToEventLog(msg, EVENTLOG_ERROR_TYPE);
    }
 
    virtual void OnStart(const std::vector<String>& ps){};
    virtual void OnStop() {}
    virtual void OnPause() {}
    virtual void OnContinue() {}
    virtual void OnShutdown() {}
 
    virtual void OnSessionChange(DWORD /*evtType*/,
        WTSSESSION_NOTIFICATION* /*notification*/) {}
private:
    // Registers handle and starts the service.
    static void WINAPI SvcMain(DWORD argc, char* argv[]);

    static void WINAPI SvcMain2(DWORD argc, wchar_t* argv[]);
 
    // Called whenever service control manager updates service status.
    static DWORD WINAPI ServiceCtrlHandler(DWORD ctrlCode, DWORD evtType,
        void* evtData, void* context);
 
    static bool RunInternal(ServiceBase* svc);
 
    void Start(const std::vector<String>& ps);
    void Start2(const std::vector<String>& ps);
    void Stop();
    void Pause();
    void Continue();
    void Shutdown();
 
    String m_name;
    String m_displayName;
    DWORD m_dwStartType;
    DWORD m_dwErrorCtrlType;
    String m_depends;
    String m_account;
    String m_password;
 
    // Info about service dependencies and user account.
    bool m_hasDepends;/* = false*/;
    bool m_hasAcc;/* = false*/;
    bool m_hasPass;/* = false*/;
 
    SERVICE_STATUS m_svcStatus;
    SERVICE_STATUS_HANDLE m_svcStatusHandle;
    HANDLE m_ghSvcStopEvent {nullptr};
 
    static ServiceBase* m_service;
    DISABLE_COPY_AND_MOVE(ServiceBase)

private:
    void ReportSvcStatus(DWORD dwCurrentState,
                              DWORD dwWin32ExitCode,
                              DWORD dwWaitHint);
};
 
#endif // SERVICE_BASE_H_
