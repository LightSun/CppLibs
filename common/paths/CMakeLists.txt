
cmake_minimum_required(VERSION 3.5)

INCLUDE_DIRECTORIES(${LUA_INCLUDE})
LINK_DIRECTORIES(${LUA_LIBDIR})

add_library(
    paths
    STATIC
    paths.h
    paths.c
    )
target_compile_definitions(paths PUBLIC paths_EXPORTS)
