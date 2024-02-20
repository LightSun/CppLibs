

file(GLOB_RECURSE SRC_TAB core/*.h
    core/*.hpp
    core/*.hxx
    core/*.cpp
)

add_library(table
    ${SRC_TAB}
    )

