cmake_minimum_required(VERSION 3.5)

project(luaPro LANGUAGES C)

# msvc: no support c99 since. vs2013. 2015
IF(MSVC)
SET(FLAGS "/0x -DLUA_COMPAT_5_2")
ELSE(MSVC)
SET(FLAGS "-O2 -std=c99 -Wall -Wextra -DLUA_COMPAT_5_2")
ENDIF(MSVC)
set(CMAKE_C_FLAGS ${FLAGS})
set(CMAKE_C_FLAGS_DEBUG ${FLAGS})
set(CMAKE_C_FLAGS_RELEASE ${FLAGS})

INCLUDE_DIRECTORIES(src)
add_library(lua
    src/lapi.c
    src/lauxlib.c
    src/lbaselib.c
    src/lbitlib.c
    src/lcode.c
    src/lcorolib.c
    src/lctype.c

    src/ldblib.c
    src/ldebug.c
    src/ldo.c
    src/ldebug.c
    src/ldump.c

    src/lfunc.c
    src/linit.c
    src/liolib.c
    src/lmathlib.c
    src/lmem.c
    src/loadlib.c
    src/lobject.c
    src/lopcodes.c
    src/loslib.c
    src/llex.c
    src/lparser.c

    src/lstring.c
    src/lstrlib.c
    src/ltable.c
    src/lstrlib.c
    src/ltablib.c

    src/lstate.c
    src/ltm.c
    src/lgc.c
    src/lua.c
    src/lundump.c
    src/lutf8lib.c
    src/lvm.c
    src/lzio.c
    )
SET_TARGET_PROPERTIES(lua PROPERTIES COMPILE_FLAGS "-fPIC")



