#ifndef SERVICE_INSTALLER_H_
#define SERVICE_INSTALLER_H_
 
#include "service_base.h"
 
class ServiceInstaller {
public:
    using String = std::string;
    static bool Install(ServiceBase& service);
    static bool Uninstall(const ServiceBase& service);
    static bool Uninstall(const String& name);
    static bool start(const String& name);
    static bool passFirewall(const String& name);
private:
    ServiceInstaller() {}
};
 
#endif // SERVICE_INSTALLER_H_
