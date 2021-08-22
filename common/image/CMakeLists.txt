cmake_minimum_required(VERSION 3.5)

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

#SET(IMAGE_LINKS luaT TH ${LUALIB})

SET(src ppm.c)

if(EXT_PNG_DIR)
    set(src ${src} png.c)
endif (EXT_PNG_DIR)

if(EXT_JPEG_DIR)
    set(src ${src} jpeg.c)
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
    STATIC
    image.h
    ${src}
    )
target_link_libraries(image ${IMAGE_LINKS})
target_compile_definitions(image PUBLIC IMAGE_EXPORTS HAVE_JPEG_MEM_SRC HAVE_JPEG_MEM_DEST)

#luaT TH ${LUALIB}
