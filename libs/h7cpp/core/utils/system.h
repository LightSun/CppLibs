#ifndef SYSTEM_H
#define SYSTEM_H

#include "common/c_common.h"
#include <string>

CPP_START

unsigned long long getAvailablePhysMemBytes();

inline bool endian_is_big()
{
    unsigned short test = 0x1234;
    if(*( (unsigned char*) &test ) == 0x12)
        return true;
    else
        return false;
}

CPP_END


#ifdef _WIN32
std::string UTF8ToGBK(const char *utf8);
std::string GBKToUTF8(const char* chGBK);

#define str_to_GBEX(x) UTF8ToGBK(x.data())
#else
#define str_to_GBEX(x) x
#endif

#endif // SYSTEM_H
