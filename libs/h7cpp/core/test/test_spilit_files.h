#pragma once

#include <string>
#include "common/c_common.h"

class FileHelper{

public:
    using CString = const std::string&;

    static void split(CString file, CString target_dir, uint64 limit_size);
    static int merge(CString file);
};
