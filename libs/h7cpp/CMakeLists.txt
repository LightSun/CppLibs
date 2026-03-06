cmake_minimum_required(VERSION 3.5)

project(h7-cpp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#-rdynamic
if(WIN32)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC -g -O0")
else(WIN32)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC -g -O0")
endif(WIN32)
#-std=c11
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -fPIC -g -O0")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS}")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

###

include(cmake/ENV.cmake)
#
if(USE_CPP_BACKWARD)
    #set(BACKWARD_TESTS ON)
    add_compile_definitions(USE_CPP_BACKWARD)
    add_compile_definitions(BACKWARD_HAS_DW=1)
    set(LIBDW_INCLUDE_DIR ${ELFUTILS_DIR}/include)
    set(LIBDW_LIBRARY ${ELFUTILS_DIR}/lib/libdw.so
        )
endif(USE_CPP_BACKWARD)

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

include_directories(core/)
include_directories(valgrind-headers/)
include_directories(./)

## sub dirs
add_subdirectory(backward-cpp-1.6)
add_subdirectory(zip/)
#add_subdirectory(gzip/)
add_subdirectory(core/)
add_subdirectory(handler-os/)
add_subdirectory(table/)
if(NOT WIN32)
    add_subdirectory(EncDec/)
endif()
add_subdirectory(json/)
#add_subdirectory(stu_perfetto/)
add_subdirectory(algo/)
