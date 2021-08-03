
INCLUDE_DIRECTORIES(${LUA_INCLUDE})
LINK_DIRECTORIES(${LUA_LIBDIR})

project(ffiPro LANGUAGES C)

#TODO latter will support arm
#LUA_FFI_BUILD_AS_DLL
#set(FFI_LIB_DIR E:/study/tools/build/ffi)

#current only mingw
#LINK_DIRECTORIES(${FFI_LIB_DIR}/x64)
#IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
#    LINK_DIRECTORIES(${FFI_LIB_DIR}/x64)
#ELSE(CMAKE_SIZEOF_VOID_P EQUAL 8)
#    LINK_DIRECTORIES(${FFI_LIB_DIR}/x86)
#ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 8)

SET(LUA_FFI_SRC ffi.h call.c ctype.c ffi.c parser.c)

add_library(ffi
    STATIC
    ${LUA_FFI_SRC}
    )
target_compile_definitions(ffi PRIVATE LUA_FFI_BUILD_AS_DLL)

TARGET_LINK_LIBRARIES(ffi ${LUALIB})
