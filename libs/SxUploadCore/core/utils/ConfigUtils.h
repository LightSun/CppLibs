#ifndef UTILS_H
#define UTILS_H

#include "common/common.h"
#include <map>

namespace h7 {

class ConfigUtils
{
public:
    static void loadProperties(CString prop_file, std::map<String, String>& out);
    static void resolveProperties(const std::vector<String>& in_dirs,
                                  std::map<String, String>& out);
    static void resolveProperties(std::map<String, String>& out){
        std::vector<String> dirs;
        resolveProperties(dirs, out);
    }
};

}

#endif // UTILS_H