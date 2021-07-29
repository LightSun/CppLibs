cmake_minimum_required(VERSION 3.5)

project(TH_luaPro LANGUAGES C)

include_directories(..)
include_directories(../internal)
include_directories(../vector)
include_directories(../lua-5.3.6/src)
include_directories(../luaT)

SET(TH_lua_H general.h utils.h generic/luaG.h)
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

add_library(TH_lua STATIC
    ${TH_lua_src}
    ${TH_lua_H}
    )

target_link_libraries(TH_lua luaT TH lua)
