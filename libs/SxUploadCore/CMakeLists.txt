cmake_minimum_required(VERSION 3.5)

project(h7-cpp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC -g -O0")
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -fPIC -std=c11 -g -O0")


#platform
SET(BUILD_ON_WIN OFF)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "Configuring on/for Linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(STATUS "Configuring on/for macOS")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Configuring on/for Windows")
    set(BUILD_ON_WIN ON)
elseif(CMAKE_SYSTEM_NAME STREQUAL "AIX")
    message(STATUS "Configuring on/for IBM AIX")
else()
    message(STATUS "Configuring on/for ${CMAKE_SYSTEM_NAME}")
endif()

#-DCMAKE_BUILD_TYPE=MinSizeRel
option(BUILD_MINGW64 "build for windows by mingw64" OFF)
if(BUILD_MINGW64)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
    add_compile_definitions(_WIN32_WINNT=0x600 NDEBUG Q_OS_WIN)
    include(cmake/linux_mingw64.cmake)
else(BUILD_MINGW64)

endif()


## jni

IF(BUILD_ON_WIN)

else(BUILD_ON_WIN)
    if(BUILD_MINGW64)
        set(WIN_JDK_INC /home/heaven7/heaven7/env/win64/jdk-11.0.2)
        include_directories(${WIN_JDK_INC}/include)
        include_directories(${WIN_JDK_INC}/include/win32)
    else(BUILD_MINGW64)
        SET(JAVA_INC /home/heaven7/heaven7/env/linux)
        include_directories(${JAVA_INC}/java_inc/include)
        include_directories(${JAVA_INC}/java_inc/include/linux)
    endif()
endif()

# openssl
set(OPEN_SSL_DIR $ENV{OPEN_SSL_DIR})
if(NOT DEFINED ENV{OPEN_SSL_DIR})
    IF(BUILD_ON_WIN)
        SET(OPEN_SSL_DIR D:/study/cpp/ENV/openssl)
    ELSE(BUILD_ON_WIN)
        SET(OPEN_SSL_DIR /home/heaven7/heaven7/env/linux/openssl)
    ENDIF(BUILD_ON_WIN)
endif()

include_directories(${OPEN_SSL_DIR}/include)
set(DEP_RSA_LIBS ${OPEN_SSL_DIR}/lib/libcrypto.a
    ${OPEN_SSL_DIR}/lib/libssl.a)

#libhv

SET(HV_LIB)
IF(BUILD_ON_WIN)
    add_subdirectory(libhv-1.3.0)
    include_directories(libhv-1.3.0/include)
    set(HV_LIB  hv_static)
ELSE(BUILD_ON_WIN)
    SET(LIB_HV_DIR $ENV{LIB_HV_DIR})
    if(NOT DEFINED ENV{LIB_HV_DIR})
        SET(LIB_HV_DIR /home/heaven7/heaven7/env/linux/libhv-1.3.0)
    endif()
    include_directories(${LIB_HV_DIR}/include)
    set(HV_LIB  ${LIB_HV_DIR}/lib/libhv.a)
ENDIF(BUILD_ON_WIN)


## dep libs
SET(DEP_LIBS ${DEP_RSA_LIBS}
    ${HV_LIB}
    )
IF(BUILD_ON_WIN OR BUILD_MINGW64)
    set(${DEP_LIBS} ${DEP_LIBS}
        ws2_32
        gdi32
        advapi32
        crypt32
        user32
        winmm
        iphlpapi
        shlwapi
        )
ELSE()
    set(${DEP_LIBS} ${DEP_LIBS}
        pthread
        dl
        )
ENDIF()

#common

include_directories(./)
include_directories(core/)
include_directories(${UPLOAD_UI_DIR}/include)
add_compile_definitions(USE_C11_ATOMICS)


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

target_link_libraries(h7cpp PUBLIC ${DEP_LIBS})

## unit test
add_executable(unittest
    ${SRC_TEST}
    )
target_link_libraries(unittest PUBLIC h7cpp)


## sub dirs
add_subdirectory(handler-os/)
