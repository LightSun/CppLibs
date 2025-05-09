cmake_minimum_required(VERSION 3.5)

project(h7Med LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC -g -O0")
else(WIN32)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC -g -O0 -rdynamic")
endif(WIN32)
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -fPIC -std=c11 -g -O0")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-error=register")
##


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
add_subdirectory(snappy/)
add_subdirectory(core/)
add_subdirectory(gzip/)

