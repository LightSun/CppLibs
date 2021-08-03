#include <iostream>
#include <queue>
#include <future>
#include <functional>

#include "TH.h"
#include "luaT.h"
#include "TH_lua/init.h"
#include "lua.hpp"
#include "luaffifb-master/ffi.h"

//LUA_EXTERNC DLL_EXPORT void luaT_getinnerparent(lua_State *L, const char *tname);

using namespace std;

#define CALL_LUA(L, func)\
{int s = func(L);\
if(s){\
    cout << "CALL_LUA >> error: "<< lua_tostring(L, -1) << endl;\
}}
#define PRINT_RESULT(s)\
{if(s){\
    cout << "CALL_LUA >> error: "<< lua_tostring(L, -1) << endl;\
}}

static const luaL_Reg funcs[] = {
{"libtorch", luaopen_libtorch},
{"ffi", luaopen_ffi},
{NULL, NULL},
};

LUALIB_API void luaL_openlibs2(lua_State *L, const luaL_Reg funcs[]) {
  const luaL_Reg *lib;
  /* "require" functions from 'loadedlibs' and set results to global table */
  for (lib = funcs; lib->func; lib++) {
    luaL_requiref(L, lib->name, lib->func, 1);
    lua_pop(L, 1);  /* remove lib */
  }
}

//-----------
extern char buf[512];
extern "C" void test_call_pppppii(void* a, void* b, void* c, void* d, void* e, int f, int g);

//#define LUA_DIR "E:/study/github/mine/CppLibs/common/lua_script"
#define LUA_DIR "../common/lua_script"

//THXXXStorage : 包含一堆操作内存的api. CRUD-fill-free-swap等操作.
//THBlas.h:  线性代数运算，比如矩阵乘法
extern "C" int main()
{
    cout << "Hello world" << endl;
   // THIntStorage* ths_int = THIntStorage_new();
    //THIntStorage_newWithMapping. flags是allocator的flags. TH_ALLOCATOR_XXX
   // THIntStorage_newWithMapping("a.db", 1024, 0);

    lua_State * L = luaL_newstate();
    luaL_openlibs(L);
    luaL_openlibs2(L, funcs);
    //luaopen_libtorch(L);
    if(luaL_dostring(L, "package.path=\"" LUA_DIR "/?.lua;" 
                     "\"..package.path"
                     ";print('package.path = ', package.path)"
                     ";print('package.cpath = ', package.cpath)")){
        cout << "error: "<< lua_tostring(L, -1) << endl;
    }else{
        cout << "lua do string success." << endl;
    }
    //luaT_getinnerparent(ls, "torch.DiskFile");
    CALL_LUA(L, [](lua_State * L){
        return luaL_dofile(L, LUA_DIR "/initAll.lua");
    });
    CALL_LUA(L, [](lua_State * L){
         PRINT_RESULT(luaL_dofile(L, LUA_DIR "/my_test/tests.lua"));
         return 0;
    });
    lua_close(L);

    int a = 5;
    test_call_pppppii(&a, &a, &a, &a, &a, 3, 2);
    printf(buf);
    printf("\r\n");

    /*
    char arr[2] ={'.', '\0'};
    //get the index which char in str . return the index
    int n = strcspn("torch.DiskFile", arr);
    std::cout << n << std::endl;
    */
    cout << "end" << endl;
    return 0;
}
