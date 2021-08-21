cmake_minimum_required(VERSION 3.5)

INCLUDE_DIRECTORIES(${LUA_INCLUDE})
LINK_DIRECTORIES(${LUA_LIBDIR})

add_library(
    sys STATIC
    sys.h
    sys.c
    )

target_compile_definitions(sys PUBLIC SYS_EXPORTS)
