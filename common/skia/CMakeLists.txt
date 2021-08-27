
set(SKIA_DIR E:/baidudisk/skia-main)
include_directories(${SKIA_DIR})
include_directories(${SKIA_DIR}/include/core)
include_directories(${SKIA_DIR}/modules)
include_directories(../lua-5.3.6/src)

link_directories(${SKIA_DIR}/out/Static)

add_library(sklua
    SkLua.h SkLua.cpp
    SkLuaCanvas.h SkLuaCanvas.cpp
    )
#
set(SK_LINK_LIBS skia skshaper)
if(WIN32)
    set(SK_LINK_LIBS ${SK_LINK_LIBS} OleAut32 FontSub User32 Usp10 Gdi32 OpenGL32 uuid)
else(WIN32)
    #TODO other platform.
endif(WIN32)

set(SK_LINK_LIBS ${SK_LINK_LIBS} ${LUALIB})

target_link_libraries(sklua ${SK_LINK_LIBS})

