cmake_minimum_required(VERSION 3.5)

project(h7 LANGUAGES C CXX)

include(../cmake/summary.cmake)

# USE_C11_ATOMICS,USE_MSC_ATOMICS,USE_GCC_ATOMICS,USE_PTHREAD_ATOMICS
add_library(h7
    h_common.h
    h_type_delegate.h
    h_map.h
    h_map.c
    h_mathutils.h
    h_mathutils.c
    h_atomic.h
    h_atomic.c
    h_alloctor.h
    h_alloctor.c
    h_string.h
    h_string.c
    )
