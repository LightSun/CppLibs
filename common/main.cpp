#include <iostream>
#include "TH.h"
#include "luaT.h"
#include "TH_lua/init.h"

//LUA_EXTERNC DLL_EXPORT void luaT_getinnerparent(lua_State *L, const char *tname);

using namespace std;

//THXXXStorage : 包含一堆操作内存的api. CRUD-fill-free-swap等操作.
//THBlas.h:  线性代数运算，比如矩阵乘法
int main()
{
    cout << "Hello world" << endl;
   // THIntStorage* ths_int = THIntStorage_new();
    //THIntStorage_newWithMapping. flags是allocator的flags. TH_ALLOCATOR_XXX
   // THIntStorage_newWithMapping("a.db", 1024, 0);

    lua_State * L = luaL_newstate();
    luaL_openlibs(L);
    luaopen_libtorch(L);
    if(luaL_dostring(L, "package.path=\"../common/lua_script/?.lua;\"..package.path"
                     ";print('package.path = ', package.path)")){
        cout << "error: "<< lua_tostring(L, -1) << endl;
    }else{
        cout << "lua do string success." << endl;
    }
    //luaT_getinnerparent(ls, "torch.DiskFile");
    lua_close(L);

    /*
    char arr[2] ={'.', '\0'};
    //get the index which char in str . return the index
    int n = strcspn("torch.DiskFile", arr);
    std::cout << n << std::endl;
    */
    cout << "end" << endl;
    return 0;
}
