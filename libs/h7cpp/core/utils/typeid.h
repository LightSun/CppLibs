/*
    BIND11/detail/typeid.h: Compiler-independent access to type identifiers

    Copyright (c) 2016 Wenzel Jakob <wenzel.jakob@epfl.ch>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE file.
*/
//changed by heaven7.

#pragma once

#include <cstdio>
#include <cstdlib>
#include <string>
#include <memory>

#if defined(__GNUG__)
#    include <cxxabi.h>
#endif

#ifdef _WIN32
#    include <windows.h>
#endif

namespace med {

namespace detail {
static inline void erase_all(std::string &string, const std::string &search) {
    for (size_t pos = 0;;) {
        pos = string.find(search, pos);
        if (pos == std::string::npos) {
            break;
        }
        string.erase(pos, search.length());
    }
}

void clean_type_id(std::string &name){
#if defined(__GNUG__)
    int status = 0;
    std::unique_ptr<char, void (*)(void *)> res{
        abi::__cxa_demangle(name.c_str(), nullptr, nullptr, &status), std::free};
    if (status == 0) {
        name = res.get();
    }
#else
    detail::erase_all(name, "class ");
    detail::erase_all(name, "struct ");
    detail::erase_all(name, "enum ");
#endif
    detail::erase_all(name, "med::");
}
}

template <typename T>
static std::string type_id() {
    std::string name(typeid(T).name());
    detail::clean_type_id(name);
    return name;
}

}
