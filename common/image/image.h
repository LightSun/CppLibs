#ifndef IMAGE_H
#define IMAGE_H

#include "../h7/macros.h"

CPP_START
#include <lua.h>
#include <lauxlib.h>
CPP_END

#if defined(_WIN32) || defined(LUA_WIN)
# ifdef IMAGE_EXPORTS
#  define IMAGE_API H7_EXTERNC __declspec(dllexport)
# else
#  define IMAGE_API H7_EXTERNC __declspec(dllimport)
# endif
#else
# define IMAGE_API H7_EXTERNC /**/
#endif

IMAGE_API int luaopen_libimage(lua_State *L);
IMAGE_API int luaopen_libjpeg(lua_State *L);
IMAGE_API int luaopen_liblua_png(lua_State *L);
IMAGE_API int luaopen_libppm(lua_State *L);


#endif // IMAGE_H
