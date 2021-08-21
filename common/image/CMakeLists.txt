cmake_minimum_required(VERSION 3.5)

# OpenMP support?
SET(WITH_OPENMP ON CACHE BOOL "OpenMP support if available?")
IF (APPLE AND CMAKE_COMPILER_IS_GNUCC)
EXEC_PROGRAM (uname ARGS -v  OUTPUT_VARIABLE DARWIN_VERSION)
STRING (REGEX MATCH "[0-9]+" DARWIN_VERSION ${DARWIN_VERSION})
MESSAGE (STATUS "MAC OS Darwin Version: ${DARWIN_VERSION}")
IF (DARWIN_VERSION GREATER 9)
SET(APPLE_OPENMP_SUCKS 1)
ENDIF (DARWIN_VERSION GREATER 9)
EXECUTE_PROCESS (COMMAND ${CMAKE_C_COMPILER} -dumpversion
    OUTPUT_VARIABLE GCC_VERSION)
IF (APPLE_OPENMP_SUCKS AND GCC_VERSION VERSION_LESS 4.6.2)
MESSAGE(STATUS "Warning: Disabling OpenMP (unstable with this version of GCC)")
MESSAGE(STATUS " Install GCC >= 4.6.2 or change your OS to enable OpenMP")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unknown-pragmas")
SET(WITH_OPENMP OFF CACHE BOOL "OpenMP support if available?" FORCE)
ENDIF ()
ENDIF ()

IF (WITH_OPENMP)
FIND_PACKAGE(OpenMP)
IF(OPENMP_FOUND)
MESSAGE(STATUS "Compiling with OpenMP support")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
ENDIF(OPENMP_FOUND)
ENDIF (WITH_OPENMP)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

#-----------------------------------
include_directories(../)
include_directories(../image)
include_directories(../lua-5.3.6/src)
include_directories(../TH_lua)
include_directories(../luaT)


#MESSAGE(STATUS "EXT_JPEG_DIR: ${EXT_JPEG_DIR}")
#MESSAGE(STATUS "EXT_PNG_DIR: ${EXT_PNG_DIR}")

#INCLUDE(CheckSymbolExists)
#CHECK_SYMBOL_EXISTS(jpeg_mem_src "jpeglib.h" ___TEST__)
#MESSAGE(STATUS "___TEST__: ${___TEST__}")

SET(IMAGE_LINKS luaT TH ${LUALIB})

SET(src ppm.c)
#SET(EXT_DIR ${CMAKE_SOURCE_DIR}/../ext_libs/mgw64)
#SET(EXT_JPEG_DIR ${EXT_DIR}/libjpeg-turbo)
#SET(EXT_PNG_DIR ${EXT_DIR}/libpng)

if(EXT_PNG_DIR)
    include_directories(${EXT_PNG_DIR}/include)
    link_directories(${EXT_PNG_DIR}/lib)
    link_directories(${EXT_DIR}/zlib/lib)
    set(src ${src} png.c)
    SET(IMAGE_LINKS ${IMAGE_LINKS} png_bundled z_bundled)
endif (EXT_PNG_DIR)

if(EXT_JPEG_DIR)
    include_directories(${EXT_JPEG_DIR}/include)
    #link_directories(E:/study/github/mine/CppLibs/ext_libs/mgw64/libjpeg-turbo/lib)
    link_directories(${EXT_JPEG_DIR}/lib)
    set(src ${src} jpeg.c)
    SET(IMAGE_LINKS ${IMAGE_LINKS} turbojpeg jpeg)
endif (EXT_JPEG_DIR)

#if (JPEG_FOUND)
#    SET(src jpeg.c)
#    include_directories (${JPEG_INCLUDE_DIR})
#    SET(CMAKE_REQUIRED_INCLUDES "${JPEG_INCLUDE_DIR}")
#    SET(CMAKE_REQUIRED_LIBRARIES "${JPEG_LIBRARY}")
#    INCLUDE(CheckSymbolExists)
#    CHECK_SYMBOL_EXISTS(jpeg_mem_src "stddef.h;stdio.h;jpeglib.h" HAVE_JPEG_MEM_SRC)
#    IF (HAVE_JPEG_MEM_SRC)
#      SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_JPEG_MEM_SRC")
#    ENDIF (HAVE_JPEG_MEM_SRC)
#    CHECK_SYMBOL_EXISTS(jpeg_mem_dest "stddef.h;stdio.h;jpeglib.h" HAVE_JPEG_MEM_DEST)
#    IF (HAVE_JPEG_MEM_DEST)
#      SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_JPEG_MEM_DEST")
#    ENDIF (HAVE_JPEG_MEM_DEST)
#    #ADD_TORCH_PACKAGE(jpeg "${src}" "${luasrc}" "Image Processing")
#    #TARGET_LINK_LIBRARIES(jpeg luaT TH ${JPEG_LIBRARIES})
#    IF(LUALIB)
#        TARGET_LINK_LIBRARIES(jpeg ${LUALIB})
#    ENDIF()
#else (JPEG_FOUND)
#    message ("WARNING: Could not find JPEG libraries, JPEG wrapper will not be installed")
#endif (JPEG_FOUND)

#if (PNG_FOUND)
#    SET(src png.c)
#    include_directories (${PNG_INCLUDE_DIR})
#    #ADD_TORCH_PACKAGE(lua_png "${src}" "${luasrc}" "Image Processing")
#    TARGET_LINK_LIBRARIES(lua_png luaT TH ${PNG_LIBRARIES})
#    IF(LUALIB)
#        TARGET_LINK_LIBRARIES(lua_png ${LUALIB})
#    ENDIF()
#else (PNG_FOUND)
#    message ("WARNING: Could not find PNG libraries, PNG wrapper will not be installed")
#endif (PNG_FOUND)

SET(src ${src} image.c)

add_library(
    image
    SHARED
    ${src}
    )
target_link_libraries(image ${IMAGE_LINKS})
target_compile_definitions(image PUBLIC HAVE_JPEG_MEM_SRC HAVE_JPEG_MEM_DEST)

#luaT TH ${LUALIB}
