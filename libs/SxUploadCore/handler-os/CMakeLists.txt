
file(GLOB_RECURSE SRC_CORE src/*.h src/*.hpp
    src/*.cpp src/*.c
    handler-os/*.h handler-os/*.hpp
    )
file(GLOB_RECURSE SRC_EXAMPLE example/*.h
    example/*.hpp
    example/*.cpp
    example/*.c)

include_directories(./)

add_library(handler_os
    ${SRC_CORE}
    )
target_link_libraries(handler_os PUBLIC
    pthread
    )

add_executable(test_handler_os
    ${SRC_EXAMPLE}
    )

target_link_libraries(test_handler_os PUBLIC
    handler_os
    )
