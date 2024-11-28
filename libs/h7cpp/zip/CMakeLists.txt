
add_compile_definitions(ZIP_STD)

## lib
file(GLOB_RECURSE ZIP_SRC_CORE 
    src/*.h src/*.cpp)

add_library(h7zip
    ${ZIP_SRC_CORE}
    )

# test
add_executable(test_zip
    test/main.cpp
    )

target_link_libraries(test_zip PUBLIC h7zip)
