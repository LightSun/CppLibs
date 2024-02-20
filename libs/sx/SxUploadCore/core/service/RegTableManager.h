#ifndef REGTABLEMANAGER_H
#define REGTABLEMANAGER_H

#include <windows.h>
#include <string>


class RegTableManager
{
public:
    using String = std::string;
    using CString = const std::string&;
    static bool write(HKEY reg_root, CString reg_path, CString key, CString val);
    static bool writeLocalMachine(CString reg_path, CString key, CString val);

};

#endif // REGTABLEMANAGER_H
