cmake_minimum_required(VERSION 3.5)

project(SxUploadCore LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -O2")
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -std=c11 -g -O2")

#-DCMAKE_BUILD_TYPE=MinSizeRel
option(BUILD_MINGW64 "build for windows by mingw64" ON)
set(BUILD_MINGW64 ON)
SET(MINGW_BUILD_DST_DIR C:/heaven7/mingw-build)
#set(UPLOAD_UI_DIR ${MINGW_BUILD_DST_DIR}/ui-upload)
#set(OPEN_SSL_DIR ${MINGW_BUILD_DST_DIR}/openssl)
#set(LIB_HV_DIR ${MINGW_BUILD_DST_DIR}/libhv-1.2.6)

#if(BUILD_MINGW64)
   # set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
    add_compile_definitions(_WIN32_WINNT=0x600 NDEBUG)
    #include(cmake/win64_mingw64.cmake)
    add_compile_definitions(Q_OS_WIN)
#else(BUILD_MINGW64)
#
#endif()
#debug task delay if need.
add_compile_definitions(PERMIT_TASK_DELAY)
#upload ui
set(UPLOAD_UI_DIR $ENV{UPLOAD_UI_DIR})
if(NOT DEFINED ENV{UPLOAD_UI_DIR})
    SET(UPLOAD_UI_DIR C:/heaven7/sx/lib_ui_upload)
endif()

#set(LIB_UI_UPLPAD ${UPLOAD_UI_DIR}/lib/libupload_core_gtk.dll)
set(LIB_UI_UPLPAD ${UPLOAD_UI_DIR}/lib/libupload_core.a
    ${UPLOAD_UI_DIR}/lib/Qt5Core.dll
    ${UPLOAD_UI_DIR}/lib/Qt5Gui.dll
    ${UPLOAD_UI_DIR}/lib/Qt5Widgets.dll
    )

# openssl
set(OPEN_SSL_DIR $ENV{OPEN_SSL_DIR})
if(NOT DEFINED ENV{OPEN_SSL_DIR})
    SET(OPEN_SSL_DIR ${MINGW_BUILD_DST_DIR}/openssl)
endif()

include_directories(${OPEN_SSL_DIR}/include)
set(DEP_RSA_LIBS ${OPEN_SSL_DIR}/lib/libcrypto.a
    ${OPEN_SSL_DIR}/lib/libssl.a)

#libhv
SET(LIB_HV_DIR $ENV{LIB_HV_DIR})
if(NOT DEFINED ENV{LIB_HV_DIR})
   SET(LIB_HV_DIR ${MINGW_BUILD_DST_DIR}/libhv-1.3.0)
endif()
include_directories(${LIB_HV_DIR}/include)
SET(HV_LIB ${LIB_HV_DIR}/lib/libhv.a)

#add_subdirectory(libhv-1.3.0)
#include_directories(libhv-1.3.0/include)
#SET(HV_LIB hv_static)

## lib blk3
SET(LIB_BLK3_DIR $ENV{LIB_BLK3_DIR})
if(NOT DEFINED ENV{LIB_BLK3_DIR})
   SET(LIB_BLK3_DIR C:/heaven7/sx/lib_blk3)
endif()

include_directories(${LIB_BLK3_DIR}/include)
SET(LIB_BLK3_LIB ${LIB_BLK3_DIR}/lib/libblake3.a)

# breakpad

#SET(BREAK_PAD  C:/heaven7/msys64/mingw64/lib/libbreakpad_client.a)
SET(LIB_Dbg Dbghelp)

SET(WIN_DEPS
    ws2_32
    gdi32
    advapi32
    crypt32
    user32
    winmm
    iphlpapi
    shlwapi
    userenv
    )

SET(SRC_UPLOAD_SERVICE
    SxServer.h
    SxServer.cpp
    SxClient.h
    SxClient.cpp
    SxWebServer.h
    SxWebServer.cpp
    WebConnection.h
    WebConnection.cpp
    SxSender.h
    SxSender.cpp
    SxReceiver.h
    SxReceiver.cpp
    sx_protocol.h
    sx_web_protocol.h
    FilesSync.hpp
    do_crash.cpp
    sx_log.cpp
    sx_log.h
    SelectFileManager.h
    )

SET(SRC_WIN_SERVICE
    core/service/service_base.h
    core/service/service_base.cpp
    core/service/service_install.h
    core/service/service_install.cpp
    core/service/RegTableManager.h
    core/service/RegTableManager.cpp
    )
#
set(DEP_LIBS ${DEP_LIBS}
    ${LIB_Dbg}
    ${HV_LIB}
    #${LIB_UI_UPLPAD}
    ${DEP_RSA_LIBS}
    ${WIN_DEPS}
    ${LIB_BLK3_LIB}
    )
#
include_directories(core/)
include_directories(${UPLOAD_UI_DIR}/include)
add_compile_definitions(USE_C11_ATOMICS)

file(GLOB_RECURSE SRC_CORE core/*.h core/*.hpp core/*.cpp core/*.c)
add_library(SxUploadCore
    ${SRC_CORE}
    )
target_link_libraries(SxUploadCore PUBLIC ${DEP_LIBS})

set(SRC_TEST
    test/test_asyncs.cpp
    test/test_SysService.cpp
    test/test_ShareMemory.cpp
    test/main.cpp
    )

add_executable(unittest
    ${SRC_TEST}
    )

target_link_libraries(unittest PUBLIC SxUploadCore
    ${LIB_UI_UPLPAD}
    )

## sx upload with ui
add_executable(SxUploadService
    ${SRC_UPLOAD_SERVICE}
    main.cpp
    )

target_compile_definitions(SxUploadService PRIVATE USE_UI_UPLOAD_INTERNAL)
target_link_libraries(SxUploadService PUBLIC SxUploadCore
    #${LIB_UI_UPLPAD}
    )


## sx upload with win service
add_executable(SxUploadWinService
    ${SRC_UPLOAD_SERVICE}
    main_service.cpp
    )

target_link_libraries(SxUploadWinService PUBLIC SxUploadCore)

## select file
add_executable(SxSelectFile
    SxSelectFile.cpp
    )
target_link_libraries(SxSelectFile PUBLIC SxUploadCore
    ${LIB_UI_UPLPAD}
    )

## win service
#add_executable(SxWinService
#    ${SRC_WIN_SERVICE}
#    main_win_service.cpp
#    )
#target_link_libraries(SxWinService PUBLIC SxUploadCore)

## test sys service
#add_executable(test_sys_service
#    ${SRC_WIN_SERVICE}
#    test/test_SysService.cpp
#    test/main_sys_service.cpp
#    )
#target_link_libraries(test_sys_service PUBLIC ${WIN_DEPS})

## tuo pan
#add_executable(SxTray
#    main_td.cpp
#    )


