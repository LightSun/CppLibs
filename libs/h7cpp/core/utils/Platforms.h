#ifndef _PLATFORMS_H
#define _PLATFORMS_H

#include <string>

namespace h7 {

class Platforms{
public:
    using String = std::string;
    using CString = const std::string&;
    static String getMac();
    static String getCpuid();
};

}

#endif // PLATFORMS_H
