
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
option(BUILD_WITH_LIBHV "build with libhv or not" OFF)

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
if(BUILD_WITH_LIBHV)
    IF(BUILD_ON_WIN)
        add_subdirectory(3rd/libhv-1.3.0)
        include_directories(3rd/libhv-1.3.0/include)
        set(HV_LIB  hv_static)
    ELSE(BUILD_ON_WIN)
        SET(LIB_HV_DIR $ENV{LIB_HV_DIR})
        if(NOT DEFINED ENV{LIB_HV_DIR})
            SET(LIB_HV_DIR /home/heaven7/heaven7/env/linux/libhv-1.3.0)
        endif()
        include_directories(${LIB_HV_DIR}/include)
        set(HV_LIB  ${LIB_HV_DIR}/lib/libhv.a)
    ENDIF(BUILD_ON_WIN)
endif(BUILD_WITH_LIBHV)


## dep libs
SET(DEP_LIBS ${DEP_RSA_LIBS}
    ${HV_LIB}
    pcre
    )
IF(BUILD_ON_WIN OR BUILD_MINGW64)
    set(DEP_LIBS ${DEP_LIBS}
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
    set(DEP_LIBS ${DEP_LIBS} pthread dl)
ENDIF()
#message("DEP_LIBS = ${DEP_LIBS}")

#common

include_directories(./)
include_directories(${UPLOAD_UI_DIR}/include)
add_compile_definitions(USE_C11_ATOMICS)
add_compile_definitions(SUPPORTS_BACKTRACE)
add_compile_definitions(HAS_DEMANGLE)


## core lib
file(GLOB_RECURSE SRC_CORE
    common/*.h common/*.hpp common/*.cpp common/*.c
    table/*.h table/*.hpp table/*.cpp table/*.c
    io/*.h io/*.cpp
    lang/*.h lang/*.cpp
    cache/*.h cache/*.cpp
    c10/*.h c10/*.hpp c10/*.cpp
    utils/*.h utils/*.hpp utils/*.cpp utils/*.c utils/*.inc
    )
set(SRC_TEST
    test/test_asyncs.cpp
    test/test_FilesSync.cpp
    test/test_CountDownLatch.cpp
    test/test_Barrier.cpp
    test/test_SortedList.cpp
    test/test_XBacktrace.cpp
    test/test_ConfigUtils.cpp
    test/test_Platforms.cpp
    test/test_spilit_files.cpp
    test/test_conrrent_hash_map.cpp
    test/test_stmp.cpp
    test/test_callback_wrapper.cpp
    test/test_LockFreeHashMap.cpp
    test/test_SaveQueue.cpp
    test/test_sfinae.cpp
    test/main.cpp
    )

add_library(h7cpp
    ${SRC_CORE}
    )

add_library(h7cpp_shared SHARED
    ${SRC_CORE}
    )
if(BUILD_WITH_LIBHV)
    target_compile_definitions(h7cpp PUBLIC BUILD_WITH_LIBHV)
    target_compile_definitions(h7cpp_shared PUBLIC BUILD_WITH_LIBHV)
endif()
target_link_libraries(h7cpp PUBLIC ${DEP_LIBS})
target_link_libraries(h7cpp_shared PUBLIC ${DEP_LIBS})

## unit test
add_executable(unittest
    ${SRC_TEST}
    )
target_link_libraries(unittest PUBLIC h7cpp)
if(CPPAST_BUILD_H7CPP)
    target_link_libraries(unittest PUBLIC cppast)
endif()

##
install(TARGETS h7cpp DESTINATION
    ${CMAKE_BINARY_DIR}/_install/lib)
install(DIRECTORY common/ DESTINATION
    ${CMAKE_BINARY_DIR}/_install/include/core/common
        FILES_MATCHING PATTERN "*.h")
install(DIRECTORY common/ DESTINATION
    ${CMAKE_BINARY_DIR}/_install/include/core/common
        FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY table/ DESTINATION
    ${CMAKE_BINARY_DIR}/_install/include/core/table
        FILES_MATCHING PATTERN "*.h")
install(DIRECTORY table/ DESTINATION
    ${CMAKE_BINARY_DIR}/_install/include/core/table
        FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY utils/ DESTINATION
    ${CMAKE_BINARY_DIR}/_install/include/core/utils
    FILES_MATCHING PATTERN "*.h")
install(DIRECTORY utils/ DESTINATION
    ${CMAKE_BINARY_DIR}/_install/include/core/utils
    FILES_MATCHING PATTERN "*.hpp")
