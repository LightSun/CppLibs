cmake_minimum_required(VERSION 3.5)

project(h7-cpp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC -g -O0")
else(WIN32)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC -g -O0 -rdynamic")
endif(WIN32)
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -fPIC -std=c11 -g -O0")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-error=register")

###

include(cmake/ENV.cmake)
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
#add_subdirectory(algo/)
