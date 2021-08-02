cmake_minimum_required(VERSION 3.5)

project(TH_luaPro LANGUAGES C CXX)

include_directories(..)
include_directories(../internal)
include_directories(../vector)
include_directories(../lua-5.3.6/src)
include_directories(../luaT)
#include_directories(generic)

#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DTORCH_EXPORTS")
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTORCH_EXPORTS")

SET(TH_lua_H general.h utils.h generic/luaG.h init.h)
SET(TH_lua_src
    DiskFile.c
    File.c
    Generator.c
    MemoryFile.c
    PipeFile.c
    Storage.c
    Tensor.c
    TensorOperator.c
    Timer.c
    utils.c
    TensorMath.c
    Random.c
    init.c
    )

add_library(${TORCH_LIB} STATIC
    ${TH_lua_src}
    ${TH_lua_H}
    )
target_compile_definitions(${TORCH_LIB} PUBLIC TORCH_EXPORTS)
#target_compile_options(TH_lua PUBLIC -DTORCH_EXPORTS -D_WIN32_WINNT=0x0502)

target_link_libraries(${TORCH_LIB} luaT TH ${LUALIB})
