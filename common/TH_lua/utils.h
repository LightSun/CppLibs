#ifndef TORCH_UTILS_INC
#define TORCH_UTILS_INC

#include "luaT.h"
#include "../TH.h"
#include "general.h"

CPP_START
#include <lua.h>
#include <lualib.h>
CPP_END

//TORCH_API int luaopen_libtorch(lua_State *L);
TORCH_API THLongStorage* torch_checklongargs(lua_State *L, int index);
TORCH_API int torch_islongargs(lua_State *L, int index);
TORCH_API const char* torch_getdefaulttensortype(lua_State *L);

#endif
