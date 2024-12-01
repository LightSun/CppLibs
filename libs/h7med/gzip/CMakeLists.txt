

add_subdirectory(zlib-1.3.1/)
#
file(GLOB_RECURSE GZIP_SRC_CORE
    src/*.h src/*.cpp)

add_library(gzip
    ${GZIP_SRC_CORE}
    )

target_link_libraries(gzip PUBLIC zlib ModelM)

## test

add_executable(test_gzip
main/main.cpp
)

target_link_libraries(test_gzip PUBLIC gzip)
