#include "system.h"

#include <stdlib.h>
#include <stdio.h>
#include <sstream>

#ifdef _WIN32
#include "windows.h"
#else

#endif

unsigned long long getAvailablePhysMemBytes(){
#ifdef _WIN32
    {
        unsigned long long size = 0;
        MEMORYSTATUSEX memoryInfo;
        memoryInfo.dwLength = sizeof(memoryInfo);
        GlobalMemoryStatusEx(&memoryInfo);
        size = memoryInfo.ullAvailPhys;
        return size;
    }
#else
    //Q_OS_UNIX
    char name1[20];
    unsigned long long MemTotal;
    char name2[20];
    unsigned long long MemFree;
    char name3[20];
    unsigned long long MemAvailable;
    //
    char buff[256];
    FILE *fd;
    fd = fopen("/proc/meminfo", "r");
    fgets(buff, sizeof(buff), fd);
    sscanf(buff, "%s %lu ", name1, &MemTotal);
    fgets(buff, sizeof(buff), fd);
    sscanf(buff, "%s %lu ", name2, &MemFree);
    fgets(buff, sizeof(buff), fd);
    sscanf(buff, "%s %lu ", name3, &MemAvailable); //kb
    fgets(buff, sizeof(buff), fd);
    fclose(fd);
    return MemAvailable*1024;
#endif
}

#ifdef _WIN32
//see windows charset: cmd -> chcp. https://blog.csdn.net/matchbox1234/article/details/108169834

std::string UTF8ToGBK(const char *utf8)
{
    if (!utf8 || strlen(utf8) < 1)
        return "";

    std::stringstream ss;
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
    wchar_t *wstr = new wchar_t[len + 1];
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);

    len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
    char *str = new char[len + 1];
    memset(str, 0, len + 1);
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
    ss << str;
    delete[]wstr;
    delete[]str;
    return ss.str();
}

std::string GBKToUTF8(const char* chGBK)
{
    DWORD dWideBufSize = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)chGBK, -1, NULL, 0);
    wchar_t * pWideBuf = new wchar_t[dWideBufSize];
    wmemset(pWideBuf, 0, dWideBufSize);
    MultiByteToWideChar(CP_ACP, 0, (LPCSTR)chGBK, -1, pWideBuf, dWideBufSize);

    DWORD dUTF8BufSize = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)pWideBuf, -1, NULL, 0, NULL, NULL);

    char * pUTF8Buf = new char[dUTF8BufSize];
    memset(pUTF8Buf, 0, dUTF8BufSize);
    WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)pWideBuf, -1, pUTF8Buf, dUTF8BufSize, NULL, NULL);

    //
    std::stringstream ss;
    ss << pUTF8Buf;
    delete[]pWideBuf;
    delete[]pUTF8Buf;
    return ss.str();
}
#endif // WIN32

