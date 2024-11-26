#pragma once

#ifdef H7_CPP_BUILD_LIB
#ifdef _MSC_VER
#define EXPORT_DLL __declspec(dllexport)
#else
#define EXPORT_DLL __attribute__((visibility("default")))
#endif
#else
#ifdef _MSC_VER
#define EXPORT_DLL __declspec(dllimport)
#else
#define EXPORT_DLL
#endif
#endif

#ifdef _MSC_VER
#include <windows.h>
#define fopen64 fopen
#define fseeko64 _fseeki64
#define ftello64 _ftelli64
#define popen _popen
#define pclose _pclose
#endif
