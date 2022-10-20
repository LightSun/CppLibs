cmake_minimum_required(VERSION 2.8.12)
if(POLICY CMP0042)
	cmake_policy(SET CMP0042 NEW)
endif()
if(POLICY CMP0063)
	cmake_policy(SET CMP0063 OLD)
endif()

project(basefirm C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# https://gcc.gnu.org/onlinedocs/libiberty/  provide std c lib for common platform
#-O3
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -fPIC")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g -fPIC")

set(libfirm_VERSION "1.22.1")
set(PROJECT_DESCRIPTION "library implementing the intermediate representation Firm")

set(CMAKE_C_VISIBILITY_PRESET hidden)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c99 -g -DDEBUG_libfirm")

include_directories(.)
include_directories(include/libfirm/adt)
include_directories(ir/adt)
include_directories(ir/common)
file(GLOB H_COMMON ir/adt/*.h)

set(SRC_COMMON
    ${H_COMMON}
    ir/adt/array.c
    ir/adt/bipartite.c
    ir/adt/bitset.c
    ir/adt/cpset.c
    ir/adt/deq.c
   # ir/adt/gaussjordan.c #高斯消元
   # ir/adt/gaussseidel.c #Gauss-Seidel迭代求解线性方程组
   # ir/adt/hungarian.c #Hungarian algorithm(匈牙利算法) 指派问题
    ir/adt/pmap.c
    ir/adt/pqueue.c
    ir/adt/pset.c
    ir/adt/pset_new.c
    ir/adt/set.c
    ir/adt/xmalloc.c

    ir/common/panic.c
    ir/obstack/obstack.c

    h7/array_map.h
    h7/array_map.c
    h7/array_map_n.h
    h7/array_map_n.c
    h7/h7_common.h
    h7/numbers.h
    h7/hash.h
    h7/hash.c
    h7/binary_search.h
    h7/binary_search.c
    h7/mem.h
    h7/mem.c
    h7/sort.h
    h7/sort.c
    h7/EngineEmitter.h
    )

add_library(basefirm
    ${SRC_COMMON}
    )

add_executable(test_basefirm
    demo/main.cpp
    demo/test_array.cpp
    demo/test_deq.cpp
    demo/test_map.cpp
    demo/test_array_map_n.cpp
    demo/test_sort.cpp
    )
target_link_libraries(test_basefirm basefirm)
