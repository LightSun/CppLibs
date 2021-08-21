#ifndef SYS_H
#define SYS_H

#include "../h7/macros.h"

CPP_START
#include <lua.h>
#include <lauxlib.h>
CPP_END

#if defined(_WIN32) || defined(LUA_WIN)
# ifdef SYS_EXPORTS
#  define SYS_API H7_EXTERNC __declspec(dllexport)
# else
#  define SYS_API H7_EXTERNC __declspec(dllimport)
# endif
#else
# define SYS_API H7_EXTERNC /**/
#endif

SYS_API int luaopen_libsys(struct lua_State *L);

#endif // SYS_H
