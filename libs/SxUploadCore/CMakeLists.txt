cmake_minimum_required(VERSION 3.5)

project(SxUploadCore LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC -g -O0")
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -fPIC -std=c11 -g -O0")

#-DCMAKE_BUILD_TYPE=MinSizeRel
option(BUILD_MINGW64 "build for windows by mingw64" OFF)
if(BUILD_MINGW64)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
    add_compile_definitions(_WIN32_WINNT=0x600 NDEBUG Q_OS_WIN)
    include(cmake/linux_mingw64.cmake)
else(BUILD_MINGW64)

endif()


## jni
if(BUILD_MINGW64)
    set(WIN_JDK_INC /home/heaven7/heaven7/env/win64/jdk-11.0.2)
    include_directories(${WIN_JDK_INC}/include)
    include_directories(${WIN_JDK_INC}/include/win32)
else(BUILD_MINGW64)
    SET(JAVA_INC /home/heaven7/heaven7/env/linux)
    include_directories(${JAVA_INC}/java_inc/include)
    include_directories(${JAVA_INC}/java_inc/include/linux)
endif()

#debug task delay if need.
add_compile_definitions(PERMIT_TASK_DELAY)
#upload ui
set(UPLOAD_UI_DIR $ENV{UPLOAD_UI_DIR})
if(NOT DEFINED ENV{UPLOAD_UI_DIR})
    SET(UPLOAD_UI_DIR /home/heaven7/heaven7/env/linux/upload_ui)
endif()

# openssl
set(OPEN_SSL_DIR $ENV{OPEN_SSL_DIR})
if(NOT DEFINED ENV{OPEN_SSL_DIR})
    SET(OPEN_SSL_DIR /home/heaven7/heaven7/env/linux/openssl)
endif()

include_directories(${OPEN_SSL_DIR}/include)
set(DEP_RSA_LIBS ${OPEN_SSL_DIR}/lib/libcrypto.a
    ${OPEN_SSL_DIR}/lib/libssl.a)

#libhv
SET(LIB_HV_DIR $ENV{LIB_HV_DIR})
if(NOT DEFINED ENV{LIB_HV_DIR})
    SET(LIB_HV_DIR /home/heaven7/heaven7/env/linux/libhv-1.3.0)
endif()
include_directories(${LIB_HV_DIR}/include)
set(HV_LIB  ${LIB_HV_DIR}/lib/libhv.a)

#add_subdirectory(libhv-1.3.0)
#include_directories(libhv-1.3.0/include)
#set(HV_LIB  hv_static)


include_directories(./)
include_directories(core/)
include_directories(${UPLOAD_UI_DIR}/include)
add_compile_definitions(USE_C11_ATOMICS)

#platform
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "Configuring on/for Linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(STATUS "Configuring on/for macOS")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Configuring on/for Windows")
elseif(CMAKE_SYSTEM_NAME STREQUAL "AIX")
    message(STATUS "Configuring on/for IBM AIX")
else()
    message(STATUS "Configuring on/for ${CMAKE_SYSTEM_NAME}")
endif()


## core lib
file(GLOB_RECURSE SRC_CORE core/*.h core/*.hpp core/*.cpp core/*.c)
set(SRC_TEST
    test/test_asyncs.cpp
    test/test_FilesSync.cpp
    test/test_CountDownLatch.cpp
    test/test_Barrier.cpp
    test/test_SortedList.cpp
    test/test_XBacktrace.cpp
    test/test_ConfigUtils.cpp
    test/test_Platforms.cpp
    test/main.cpp
    )

add_library(h7cpp
    ${SRC_CORE}
    )

add_library(h7cpp_shared SHARED
    ${SRC_CORE}
    )

if(BUILD_MINGW64)
    target_link_libraries(h7cpp PUBLIC
        ${DEP_RSA_LIBS}
        ws2_32
        gdi32
        advapi32
        crypt32
        user32
        winmm
        iphlpapi
        shlwapi
        )
    target_link_libraries(h7cpp_shared PUBLIC
        ${DEP_RSA_LIBS}
        ws2_32
        gdi32
        advapi32
        crypt32
        user32
        winmm
        iphlpapi
        shlwapi
        )
else()
    target_link_libraries(h7cpp PUBLIC
        ${DEP_RSA_LIBS}
        ${HV_LIB}
        pthread
        dl
        )
    target_link_libraries(h7cpp_shared PUBLIC
        ${DEP_RSA_LIBS}
        ${HV_LIB}
        pthread
        dl
        )
endif()

## unit test
add_executable(unittest
    ${SRC_TEST}
    )

target_link_libraries(unittest PUBLIC h7cpp)
if(BUILD_MINGW64)
else(BUILD_MINGW64)
    target_link_libraries(unittest PUBLIC
        ${UPLOAD_UI_DIR}/lib/libupload_core_gtk.so
        )
endif(BUILD_MINGW64)

## ----------------
#if(BUILD_MINGW64)
#    target_link_libraries(SxServer PUBLIC
#        ${LIB_HV_DIR}/lib/libhv.a
#        ${DEP_RSA_LIBS}
#        ws2_32
#        gdi32
#        advapi32
#        crypt32
#        user32
#        winmm
#        iphlpapi
#        shlwapi
#        )
#    else(BUILD_MINGW64)
#        target_link_libraries(SxServer PUBLIC
#            ${HV_LIB}
#            ${DEP_RSA_LIBS}
#            pthread
#            dl
#            )
#    endif()

add_subdirectory(handler-os/)
