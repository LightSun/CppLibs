#include <iostream>

//extern "C"{
#include "TH.h"
#include "lua.hpp"
#include "luaT.h"
//}

LUA_EXTERNC DLL_EXPORT int luaopen_libtorch(lua_State *L);

using namespace std;

//THXXXStorage : 包含一堆操作内存的api. CRUD-fill-free-swap等操作.
//THBlas.h:  线性代数运算，比如矩阵乘法
extern "C" int main()
{
    cout << "Hello World!" << endl;

    //THIntStorage* ths_int = THIntStorage_new();
    //THIntStorage_newWithMapping. flags是allocator的flags. TH_ALLOCATOR_XXX
   // THIntStorage_newWithMapping("a.db", 1024, 0);
    lua_State * ls = luaL_newstate();
    luaopen_libtorch(ls);
    lua_close(ls);
    return 0;
}
