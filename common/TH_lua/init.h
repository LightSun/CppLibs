#ifndef INIT_H
#define INIT_H

#include "general.h"

CPP_START
#include <lua.h>
#include <lualib.h>
CPP_END

TORCH_API int luaopen_libtorch(lua_State *L);

#endif // INIT_H
