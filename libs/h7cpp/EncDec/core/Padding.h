#pragma once

#include <string>
#include <vector>

namespace med {

using CString = const std::string&;
using String = std::string;
template<typename T>
using List = std::vector<T>;
using uint32 = unsigned int;

class Padding
{
public:
    static int getPKCS7PaddedLength(int dataLen, int alignSize);
    static String doPKCS7Padding(CString in, int alignSize);
    static String doPKCS7UnPadding(CString in);
};

}

