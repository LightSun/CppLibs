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
    )

# micors: _WIN32, _OPENMP, TH_BLAS_MKL
# for linux: HAVE_MMAP, HAVE_SHM_UNLINK
# BLAS_F2C: ffloat as double or float
# BLAS lib: BLAS_USE_CBLAS_DOT, USE_BLAS, TH_REAL_IS_DOUBLE, TH_REAL_IS_FLOAT
# lapack lib: USE_LAPACK

#for win32 need 'TH_EXPORTS' or else cause undefined reference '__imp_xxx'
#add_compile_definitions(TH_EXPORTS)
#end win32

add_library(TH STATIC
    ${SRC_H}
    ${SRC_CPP}
    )
#add_compile_options: for all target of current
#target_compile_options: for special target

#add_compile_definitions: for all
#target_compile_definitions: only target
target_compile_definitions(TH PUBLIC TH_EXPORTS)

add_executable(common
    main.cpp)
target_link_libraries(common TH)
