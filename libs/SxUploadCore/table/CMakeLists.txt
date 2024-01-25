

file(GLOB_RECURSE SRC_TAB core/*.h
    core/*.hpp
    core/*.cpp
)

add_library(table
    ${SRC_TAB}
    )

