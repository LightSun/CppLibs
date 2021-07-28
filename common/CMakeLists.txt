cmake_minimum_required(VERSION 3.5)

project(common LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(SRC_H
    THCommon.h
    THGeneral.h
    THAtomic.h
    THAllocator.h
    THHalf.h
    THBlas.h
    ThLapack.h
    THDiskFile.h
    THFile.h
    THStorage.h
    THMemoryFile.h

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
    ThLapack.c
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
#add_compile_options: for all target of current
#target_compile_options: for special target

#add_compile_definitions: for all
#target_compile_definitions: only target
#for win32 need 'TH_EXPORTS' or else cause undefined reference '__imp_xxx'
target_compile_definitions(TH PUBLIC TH_EXPORTS)

add_executable(common
    main.cpp)
target_link_libraries(common TH)
