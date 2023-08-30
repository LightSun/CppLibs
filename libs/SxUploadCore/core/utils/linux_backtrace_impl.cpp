#include <iostream>
#include <assert.h>
#include <stdio.h>
#include <execinfo.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include <execinfo.h>
#include <cxxabi.h>
#include <sstream>

#include <memory.h>
#include <map>

using String = std::string;
static std::vector<String> _map = {
"std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >", "String",
"std::__cxx14::basic_string<char, std::char_traits<char>, std::allocator<char> >", "String",
"std::__cxx17::basic_string<char, std::char_traits<char>, std::allocator<char> >", "String",
"<String, std::allocator<String > >", "<String>"
};

extern "C"
void _backtrace_transform(const char* src, char* dst){
    String _str = src;
    for(int i = 0 ; i < (int)_map.size() ; i += 2){
        String& key = _map[i];
        String& val = _map[i + 1];
        int pos = _str.find(key);
        while(pos >= 0){
            String pre = pos > 0 ? _str.substr(0,pos) : "";
            _str = pre + val + _str.substr(pos + key.size());
            pos = _str.find(key);
        }
    }
    memcpy(dst, _str.data(), (int)_str.length());
}
