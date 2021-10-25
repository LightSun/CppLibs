
# skia demo: https://fiddle.skia.org/named/

set(SKIA_DIR E:/baidudisk/skia-main)    #work
#set(SKIA_DIR E:/study/google/skia-main) #home
include_directories(${SKIA_DIR})
include_directories(${SKIA_DIR}/include/core)
include_directories(${SKIA_DIR}/modules)
include_directories(../lua-5.3.6/src)

link_directories(${SKIA_DIR}/out/Static)

add_library(sklua
    SkLua.h SkLua.cpp
    SkLuaCanvas.h SkLuaCanvas.cpp
    )

add_executable(skia-example
    skia_example.h
    skia_example.cpp
    )
#target_compile_definitions(sklua PUBLIC SK_GL)

set(SK_LINK_LIBS skia skshaper)
if(WIN32)
    set(SK_LINK_LIBS ${SK_LINK_LIBS} OleAut32 FontSub User32 Usp10 Gdi32 OpenGL32 uuid)
else(WIN32)
    #TODO other platform.
endif(WIN32)

target_link_libraries(sklua ${SK_LINK_LIBS} ${LUALIB})
target_link_libraries(skia-example ${SK_LINK_LIBS})


