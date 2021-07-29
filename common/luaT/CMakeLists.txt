cmake_minimum_required(VERSION 3.5)
# avoid some cmake warnings
IF(POLICY CMP0026)
 CMAKE_POLICY(SET CMP0026 OLD)
ENDIF()

SET(LUA_INCLUDE ../lua-5.3.6/src)
SET(LUA_LIBDIR E:/study/github/mine/CppLibs/build-common-Desktop_Qt_5_12_9_MinGW_64_bit-Debug/lua-5.3.6)
SET(LUALIB lua)

INCLUDE_DIRECTORIES(${LUA_INCLUDE})
LINK_DIRECTORIES(${LUA_LIBDIR}) # note: must be done before defining target

ADD_LIBRARY(luaT SHARED luaT.h luaT.c)

IF (BUILD_STATIC OR "$ENV{STATIC_TH}" STREQUAL "YES")
  ADD_LIBRARY(luaT_static STATIC luaT.h luaT.c)
  SET_TARGET_PROPERTIES(luaT_static PROPERTIES
    COMPILE_FLAGS "-fPIC")
  SET_TARGET_PROPERTIES(luaT_static PROPERTIES
    PREFIX "lib" IMPORT_PREFIX "lib" OUTPUT_NAME "luaT")
ENDIF()

IF(APPLE)
  SET_TARGET_PROPERTIES(luaT PROPERTIES
    LINK_FLAGS "-undefined dynamic_lookup")
ENDIF()

TARGET_LINK_LIBRARIES(luaT ${LUALIB}) # must be done after ;)
