#ifndef TORCH_GENERAL_INC
#define TORCH_GENERAL_INC

#ifdef __cplusplus
#define CPP_START extern "C"{
#else
#define CPP_START
#endif

#ifdef __cplusplus
#define CPP_END }
#else
#define CPP_END
#endif

CPP_START
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#ifdef _WIN32
#else
#include <unistd.h>
#endif

CPP_END

#include "luaT.h"
#include "../TH.h"

#if (defined(_MSC_VER) || defined(__MINGW32__))
#define snprintf _snprintf
#define popen _popen
#define pclose _pclose
#endif

#if LUA_VERSION_NUM >= 503
#define luaL_optlong(L,n,d)     ((long)luaL_optinteger(L, (n), (d)))
#define luaL_checklong(L,n)     ((long)luaL_checkinteger(L, (n)))
#define luaL_checkint(L,n)      ((int)luaL_checkinteger(L, (n)))
#endif

#ifdef __cplusplus
# define TORCH_EXTERNC extern "C"
#else
# define TORCH_EXTERNC extern
#endif

#if defined(BUILD_WITH_GCC)
# define TORCH_API TORCH_EXTERNC
#elif defined(_WIN32)
#   if defined(TORCH_EXPORTS)
#   define TORCH_API TORCH_EXTERNC __declspec(dllexport)
#   else
#   define TORCH_API TORCH_EXTERNC __declspec(dllimport)
#   endif
#else
# define TORCH_API TORCH_EXTERNC
#endif

#endif
/*
one can simply enable LUA_COMPAT_5_2 to be backward compatible.
However, this does not work when we are trying to use system-installed lua,
hence these redefines
*/
