#include "RegTableManager.h"


//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Windows


bool RegTableManager::write(HKEY reg_root, CString reg_path,CString key, CString val)
{
    //String regPath = "SYSTEM\\CurrentControlSet\\Control\\Windows";
    //NoInteractiveServices : 0 means permit. 1 means not permit ui-service

    HKEY hKey;
    if (ERROR_SUCCESS != RegOpenKeyEx(reg_root, reg_path.data(), 0, KEY_WRITE, &hKey)){
        return FALSE;
    }
    if (ERROR_SUCCESS != RegSetValueEx(hKey, key.data(),
                                       0, REG_SZ, (BYTE*)val.data(), val.length() + 1)){
        printf("write value failed: k, v = %s, %s\n", key.data(), val.data());
        RegCloseKey(hKey);
        return FALSE;
    }
    RegCloseKey(hKey);
    return true;
}

bool RegTableManager::writeLocalMachine(CString reg_path, CString key, CString val){
    return write(HKEY_LOCAL_MACHINE, reg_path, key, val);
}
