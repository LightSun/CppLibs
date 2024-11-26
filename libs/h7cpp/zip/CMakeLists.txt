
add_compile_definitions(ZIP_STD)

file(GLOB_RECURSE ZIP_SRC_CORE 
    src/*.h src/*.cpp)

add_library(h7zip
    ${ZIP_SRC_CORE}
    )
