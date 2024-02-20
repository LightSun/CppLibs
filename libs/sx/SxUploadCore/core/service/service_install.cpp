
#include "service_install.h"
#include "utils/CmdHelper.h"
 
namespace 
{
    class ServiceHandle 
    {
        public:
        using String = std::string;
            ServiceHandle(SC_HANDLE handle) : m_handle(handle) {}
 
            ~ServiceHandle() 
            {
                if (m_handle) 
                {
                    ::CloseServiceHandle(m_handle);
                }
            }
 
            operator SC_HANDLE() 
            {
                return m_handle;
            }
 
        private:
            SC_HANDLE m_handle;
    };
}
 
//static
bool ServiceInstaller::Install(ServiceBase& service)
{
    char modulePath[MAX_PATH];
 
    if (::GetModuleFileName(nullptr, modulePath, MAX_PATH) == 0)
    {
        printf("Couldn't get module file name: %d\n", ::GetLastError());
        return false;
    }
    //C:\heaven7\sx\build-SxUploadCore-Mingw_64-Debug\unittest.exe
    printf("modulePath = %s\n", modulePath);
    String escapedPath(modulePath);
 
   // escapedPath = "\"" + escapedPath + "\"";
 
    ServiceHandle svcM = ::OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (!svcM)
    {
        printf("Couldn't open service control manager: %d\n", GetLastError());
        return false;
    }
 
    const String& depends = service.GetDependencies();
    const String& acc = service.GetAccount();
    const String& pass = service.GetPassword();
 
    ServiceHandle servHandle = ::CreateService(svcM,
        service.GetName().data(),
        service.GetDisplayName().data(),
        SERVICE_QUERY_STATUS,
        //SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
        SERVICE_WIN32_OWN_PROCESS,
        service.GetStartType(),
        service.GetErrorControlType(),
        escapedPath.data(),
        nullptr,
        nullptr,
        (depends.empty() ? nullptr : depends.data()),
        (acc.empty() ? nullptr : acc.data()),
        (pass.empty() ? nullptr : pass.data()));
    
    if (!servHandle) 
    {
        CloseServiceHandle(svcM);
        printf("Couldn't create service: %d\n", ::GetLastError());
        return false;
    }
    CloseServiceHandle(servHandle);
    CloseServiceHandle(svcM);
    return servHandle;
}
 
//static
bool ServiceInstaller::Uninstall(const ServiceBase& service)
{
    return Uninstall(service.GetName());
}

bool ServiceInstaller::Uninstall(const String& name){
    ServiceHandle svcM = ::OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);

    if (!svcM)
    {
        printf("Couldn't open service control manager: %d\n", GetLastError());
        return false;
    }

    ServiceHandle servHandle = ::OpenService(svcM, name.data(),
        SERVICE_QUERY_STATUS | SERVICE_STOP | DELETE);

    if (!servHandle)
    {
        printf("Couldn't open service control manager: %d\n", ::GetLastError());
        return false;
    }

    SERVICE_STATUS servStatus = {};
    QueryServiceStatus(servHandle, &servStatus);
    if (servStatus.dwCurrentState == SERVICE_RUNNING){
        //SERVICE_STATUS servStatus = {};
        if (::ControlService(servHandle, SERVICE_CONTROL_STOP, &servStatus)){
            printf("Stoping service %s\n", name.data());

            if (servStatus.dwCurrentState != SERVICE_STOPPED)
            {
                printf("Failed to stop the service\n");
            }
            else
            {
                printf("Service stopped\n");
            }
        }else{
            printf("Didn't control service: %d\n", ::GetLastError());
        }
    }else{
        printf("service not running.\n");
    }

    if (!::DeleteService(servHandle))
    {
        CloseServiceHandle(servHandle);
        CloseServiceHandle(svcM);
        printf("Failed to delete the service: %d\n", GetLastError());
        return false;
    }
    CloseServiceHandle(servHandle);
    CloseServiceHandle(svcM);
    return true;
}

//https://learn.microsoft.com/en-us/windows/win32/services/starting-a-service
bool ServiceInstaller::start(const String& name){
    ServiceHandle schSCManager = ::OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (!schSCManager)
    {
        printf("Couldn't open service control manager: %d\n", GetLastError());
        return false;
    }

    ServiceHandle schService = ::OpenService(schSCManager, name.data(), SERVICE_ALL_ACCESS);

    if (!schService)
    {
        printf("Couldn't open service control manager: %d\n", ::GetLastError());
        return false;
    }
    SERVICE_STATUS_PROCESS ssStatus;
    DWORD dwOldCheckPoint;
    DWORD dwStartTickCount;
    DWORD dwWaitTime;
    DWORD dwBytesNeeded;


    // Check the status in case the service is not stopped.

    if (!QueryServiceStatusEx(
            schService,                     // handle to service
            SC_STATUS_PROCESS_INFO,         // information level
            (LPBYTE) &ssStatus,             // address of structure
            sizeof(SERVICE_STATUS_PROCESS), // size of structure
            &dwBytesNeeded ) )              // size needed if buffer is too small
    {
        printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return false;
    }


    // Check if the service is already running. It would be possible
    // to stop the service here, but for simplicity this example just returns.

  if(ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING)
  {
      printf("Cannot start the service because it is already running\n");
      CloseServiceHandle(schService);
      CloseServiceHandle(schSCManager);
      return false;
  }

  // Save the tick count and initial checkpoint.

  dwStartTickCount = GetTickCount();
  dwOldCheckPoint = ssStatus.dwCheckPoint;

  // Wait for the service to stop before attempting to start it.

  while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
  {
      // Do not wait longer than the wait hint. A good interval is
      // one-tenth of the wait hint but not less than 1 second
      // and not more than 10 seconds.

      dwWaitTime = ssStatus.dwWaitHint / 10;

      if( dwWaitTime < 1000 )
          dwWaitTime = 1000;
      else if ( dwWaitTime > 10000 )
          dwWaitTime = 10000;

      Sleep( dwWaitTime );

      // Check the status until the service is no longer stop pending.

      if (!QueryServiceStatusEx(
              schService,                     // handle to service
              SC_STATUS_PROCESS_INFO,         // information level
              (LPBYTE) &ssStatus,             // address of structure
              sizeof(SERVICE_STATUS_PROCESS), // size of structure
              &dwBytesNeeded ) )              // size needed if buffer is too small
      {
          printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
          CloseServiceHandle(schService);
          CloseServiceHandle(schSCManager);
          return false;
      }

      if ( ssStatus.dwCheckPoint > dwOldCheckPoint )
      {
          // Continue to wait and check.

          dwStartTickCount = GetTickCount();
          dwOldCheckPoint = ssStatus.dwCheckPoint;
      }
      else
      {
          if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint)
          {
              printf("Timeout waiting for service to stop\n");
              CloseServiceHandle(schService);
              CloseServiceHandle(schSCManager);
              return false;
          }
      }
  }

  // Attempt to start the service.

     if (!StartService(
             schService,  // handle to service
             0,           // number of arguments
             NULL) )      // no arguments
     {
         printf("StartService failed (%d)\n", GetLastError());
         CloseServiceHandle(schService);
         CloseServiceHandle(schSCManager);
         return false;
     }
     else printf("Service start pending...\n");

     // Check the status until the service is no longer start pending.
     if (!QueryServiceStatusEx(
             schService,                     // handle to service
             SC_STATUS_PROCESS_INFO,         // info level
             (LPBYTE) &ssStatus,             // address of structure
             sizeof(SERVICE_STATUS_PROCESS), // size of structure
             &dwBytesNeeded ) )              // if buffer too small
     {
         printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
         CloseServiceHandle(schService);
         CloseServiceHandle(schSCManager);
         return false;
     }

     // Save the tick count and initial checkpoint.

     dwStartTickCount = GetTickCount();
     dwOldCheckPoint = ssStatus.dwCheckPoint;

     while (ssStatus.dwCurrentState == SERVICE_START_PENDING)
     {
         // Do not wait longer than the wait hint. A good interval is
         // one-tenth the wait hint, but no less than 1 second and no
         // more than 10 seconds.

         dwWaitTime = ssStatus.dwWaitHint / 10;

         if( dwWaitTime < 1000 )
             dwWaitTime = 1000;
         else if ( dwWaitTime > 10000 )
             dwWaitTime = 10000;

         Sleep( dwWaitTime );

         // Check the status again.

         if (!QueryServiceStatusEx(
             schService,             // handle to service
             SC_STATUS_PROCESS_INFO, // info level
             (LPBYTE) &ssStatus,             // address of structure
             sizeof(SERVICE_STATUS_PROCESS), // size of structure
             &dwBytesNeeded ) )              // if buffer too small
         {
             printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
             break;
         }

         if ( ssStatus.dwCheckPoint > dwOldCheckPoint )
         {
             // Continue to wait and check.

             dwStartTickCount = GetTickCount();
             dwOldCheckPoint = ssStatus.dwCheckPoint;
         }
         else
         {
             if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint)
             {
                 // No progress made within the wait hint.
                 break;
             }
         }
     }

     // Determine whether the service is running.

     if (ssStatus.dwCurrentState == SERVICE_RUNNING)
     {
         printf("Service started successfully.\n");
         CloseServiceHandle(schService);
         CloseServiceHandle(schSCManager);
         return true;
     }
     else
     {
         printf("Service not started. \n");
         printf("  Current State: %d\n", ssStatus.dwCurrentState);
         printf("  Exit Code: %d\n", ssStatus.dwWin32ExitCode);
         printf("  Check Point: %d\n", ssStatus.dwCheckPoint);
         printf("  Wait Hint: %d\n", ssStatus.dwWaitHint);
     }

     CloseServiceHandle(schService);
     CloseServiceHandle(schSCManager);
     return false;
}

//need UAC - admin
bool ServiceInstaller::passFirewall(const String& path){
    //printf("passFirewall: %s\n", path.data());
    //delete old
    //netsh advfirewall firewall delete rule name="110"

    //String cmd = "netsh advfirewall firewall delete rule name=\"MedSxUpload\"";
    //using CMD = h7::CmdHelper;
    //String ret;
    //CMD::execCmd(cmd.data(), ret);

    //netsh advfirewall firewall add rule name = "SxUpload" dir=in
    //  program="C:\heaven7\sx\build-SxUploadCore-Mingw_64-Release\SxUploadWinService.exe" action=allow
    //String s = "netsh advfirewall firewall add rule name = \"MedSxUpload\" dir=in program=";
    //s += path;
    //s += " action=allow";
    //return CMD::execCmd(s.data(), ret) == 0;
}
