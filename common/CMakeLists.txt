cmake_minimum_required(VERSION 3.5)

project(common LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DBUILD_WITH_GCC")
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBUILD_WITH_GCC")
#add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1 -DTORCH_EXPORTS)

# OpenMP support?
#SET(WITH_OPENMP ON CACHE BOOL "OpenMP support if available?")
#    IF (APPLE AND CMAKE_COMPILER_IS_GNUCC)
#        EXEC_PROGRAM (uname ARGS -v  OUTPUT_VARIABLE DARWIN_VERSION)
#        STRING (REGEX MATCH "[0-9]+" DARWIN_VERSION ${DARWIN_VERSION})
#        MESSAGE (STATUS "MAC OS Darwin Version: ${DARWIN_VERSION}")
#        IF (DARWIN_VERSION GREATER 9)
#        SET(APPLE_OPENMP_SUCKS 1)
#        ENDIF (DARWIN_VERSION GREATER 9)
#        EXECUTE_PROCESS (COMMAND ${CMAKE_C_COMPILER} -dumpversion
#            OUTPUT_VARIABLE GCC_VERSION)
#        IF (APPLE_OPENMP_SUCKS AND GCC_VERSION VERSION_LESS 4.6.2)
#        MESSAGE(STATUS "Warning: Disabling OpenMP (unstable with this version of GCC)")
#        MESSAGE(STATUS " Install GCC >= 4.6.2 or change your OS to enable OpenMP")
#        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unknown-pragmas")
#        SET(WITH_OPENMP OFF CACHE BOOL "OpenMP support if available?" FORCE)
#    ENDIF ()
#ENDIF ()

#IF (WITH_OPENMP)
#FIND_PACKAGE(OpenMP)
#IF(OPENMP_FOUND)
#MESSAGE(STATUS "Compiling with OpenMP support")
#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
#SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
#ENDIF(OPENMP_FOUND)
#ENDIF (WITH_OPENMP)

SET(SRC_H
    THCommon.h
    THGeneral.h
    THAtomic.h
    THAllocator.h
    THHalf.h
    THBlas.h

    THLapack.h

    THDiskFile.h
    THFile.h
    THStorage.h
    THMemoryFile.h

    TH.h
    THMath.h

    THLogAdd.h
    THRandom.h
    THSize.h
    THVector.h
    THTensor.h
    )
SET(SRC_CPP
    THGeneral.c
    THAtomic.c
    THAllocator.c
    THHalf.c
    THBlas.c
    THLapack.c
    THDiskFile.c
    THFile.c
    THStorage.c
    THMemoryFile.c

    THLogAdd.c
    THRandom.c
    THSize.c
    THVector.c
    THTensor.c
    )

# micors: _WIN32, _OPENMP, TH_BLAS_MKL. BLAS_F2C(ffloat as double or float)
#        USE_DOUBLE, DEBUG

# sse:  USE_SSE2, USE_SSE3, USE_SSE4_1, USE_SSE4_2
# AVX:  USE_AVX, USE_AVX2
# ARM: __NEON__ (armv7+v8), -mfpu=neon(armv7)
# if have AVX/AVX2/SSE4_1/SSE4_2: SET(simd internal/simd/convolve.c)
# if have SSE4_1 && SSE4_2:
    # SET(simd ${simd} internal/simd/convolve5x5_sse.c)
    # SET_SOURCE_FILES_PROPERTIES(internal/simd/convolve5x5_sse.c PROPERTIES COMPILE_FLAGS "/Ox /fp:fast") --- msvc
    # SET_SOURCE_FILES_PROPERTIES(internal/simd/convolve5x5_sse.c PROPERTIES COMPILE_FLAGS "-O3 -ffast-math") ---others
#    IF(C_AVX_FOUND)
#      IF(MSVC)
#        SET_SOURCE_FILES_PROPERTIES(internal/simd/convolve5x5_avx.c PROPERTIES COMPILE_FLAGS "/Ox /fp:fast ${C_AVX_FLAGS}")
#        SET_SOURCE_FILES_PROPERTIES(vector/AVX.c PROPERTIES COMPILE_FLAGS "/Ox /arch:AVX ${C_AVX_FLAGS}")
#      ELSE(MSVC)
#        SET_SOURCE_FILES_PROPERTIES(internal/simd/convolve5x5_avx.c PROPERTIES COMPILE_FLAGS "-O3 -ffast-math ${C_AVX_FLAGS}")
#        SET_SOURCE_FILES_PROPERTIES(vector/AVX.c PROPERTIES COMPILE_FLAGS "-O3 ${C_AVX_FLAGS}")
#      ENDIF(MSVC)
#      SET(simd ${simd} vector/AVX.c internal/simd/convolve5x5_avx.c)
#    ENDIF(C_AVX_FOUND)

#   IF(C_AVX2_FOUND)
#      IF(MSVC)
#        SET_SOURCE_FILES_PROPERTIES(vector/AVX2.c PROPERTIES COMPILE_FLAGS "/Ox /arch:AVX2 ${C_AVX2_FLAGS}")
#      ELSE(MSVC)
#        SET_SOURCE_FILES_PROPERTIES(vector/AVX2.c PROPERTIES COMPILE_FLAGS "-O3 ${C_AVX2_FLAGS}")
#      ENDIF(MSVC)
#     SET(simd ${simd} vector/AVX2.c)
#    ENDIF(C_AVX2_FOUND)

# HAVE_GCC_GET_CPUID(cpuid.h), USE_GCC_GET_CPUID

# for linux: HAVE_MMAP(mmap), HAVE_SHM_UNLINK(shm_unlink)

# atomic micors: USE_C11_ATOMICS, USE_MSC_ATOMICS, USE_GCC_ATOMICS, USE_PTHREAD_ATOMICS

# BLAS lib: BLAS_USE_CBLAS_DOT, USE_BLAS, TH_REAL_IS_DOUBLE, TH_REAL_IS_FLOAT
# lapack lib: USE_LAPACK

add_library(TH STATIC
    ${SRC_H}
    ${SRC_CPP}
    )
target_compile_definitions(TH PUBLIC USE_C11_ATOMICS) # if only use c.

# need -std=gnu99 for mingw

#add_compile_options: for all target of current
#target_compile_options: for special target

#add_compile_definitions: for all
#target_compile_definitions: only target
#for win32 need 'TH_EXPORTS' or else cause undefined reference '__imp_xxx'
target_compile_definitions(TH PUBLIC TH_EXPORTS)

#set share variable for multi CMakeLists.txt
set(LUA_INCLUDE ${CMAKE_SOURCE_DIR}/lua-5.3.6/src CACHE INTERNAL "lua header")
#set(LUA_LIBDIR E:/study/github/mine/CppLibs/build-common-Desktop_Qt_5_12_9_MinGW_64_bit-Debug/lua-5.3.6 CACHE INTERNAL "lua lib dir")
set(LUA_LIBDIR ${CMAKE_CURRRENT_BINARY_DIR}/lua-5.3.6 CACHE INTERNAL "lua lib dir")
set(LUALIB lua CACHE INTERNAL "lua lib name")
set(TORCH_LIB torch CACHE INTERNAL "torch lib name")
#set_property(GLOBAL PROPERTY LUA_INCLUDE "lua-5.3.6/src") # need get_property when use
#set_property(GLOBAL PROPERTY LUA_LIBDIR "E:/study/github/mine/CppLibs/build-common-Desktop_Qt_5_12_9_MinGW_64_bit-Debug/lua-5.3.6")
#set_property(GLOBAL PROPERTY LUALIB "lua")


# ======= image module ============
SET(EXT_DIR ${CMAKE_SOURCE_DIR}/../ext_libs/mgw64 CACHE INTERNAL "ext dir")
SET(EXT_JPEG_DIR ${EXT_DIR}/libjpeg-turbo CACHE INTERNAL "jpeg dir")
SET(EXT_PNG_DIR ${EXT_DIR}/libpng CACHE INTERNAL "png dir")

set(IMAGE_LINKS luaT TH ${LUALIB} CACHE INTERNAL "image links")
if(EXT_PNG_DIR)
    include_directories(${EXT_PNG_DIR}/include)
    link_directories(${EXT_PNG_DIR}/lib)
    link_directories(${EXT_DIR}/zlib/lib)
    #set(src ${src} png.c)
    SET(IMAGE_LINKS ${IMAGE_LINKS} png_bundled z_bundled)
endif (EXT_PNG_DIR)

if(EXT_JPEG_DIR)
    include_directories(${EXT_JPEG_DIR}/include)
    link_directories(${EXT_JPEG_DIR}/lib)
    #set(src ${src} jpeg.c)
    SET(IMAGE_LINKS ${IMAGE_LINKS} turbojpeg jpeg)
endif (EXT_JPEG_DIR)

# end ======= image module ============

add_subdirectory(lua-5.3.6)
add_subdirectory(luaT)
add_subdirectory(TH_lua)
add_subdirectory(luaffifb-master)
add_subdirectory(nn/THNN/)
add_subdirectory(paths)
add_subdirectory(sys)
add_subdirectory(image)
#add_subdirectory(skia)
#add_subdirectory(lanes)

include_directories(lua-5.3.6/src)
include_directories(luaT)

add_executable(common
    luaffifb-master/luaffi_test.h
    luaffifb-master/test.c
    main.cpp)
target_link_libraries(common image luaT TH ${TORCH_LIB} ffi ${LUALIB} paths sys)

# default support openmp
IF(NOT (FORCE_SKIP_OPENMP))
    INCLUDE (${PROJECT_SOURCE_DIR}/cmake/openmp.cmake)
    TENGINE_CHECK_LIB_OPENMP (_has_openmp)
    IF (_has_openmp)
        #TENGINE_USE_LIB_OPENMP (TH-static ON)
        TENGINE_USE_LIB_OPENMP (TH        ON)
        IF(MSVC)
            SET_SOURCE_FILES_PROPERTIES(${TENGINE_SOURCE_FILES} PROPERTIES LANGUAGE CXX)
            #TARGET_COMPILE_OPTIONS(TH-static PRIVATE /std:c++latest)
            TARGET_COMPILE_OPTIONS(TH        PRIVATE /std:c++latest)
        ENDIF()

        SET (TENGINE_ENABLED_OPENMP ${OpenMP_C_VERSION} CACHE INTERNAL "" FORCE)
    ENDIF()
ENDIF()
UNSET (_has_openmp)

#summary all compile info
include(cmake/summary.cmake)
