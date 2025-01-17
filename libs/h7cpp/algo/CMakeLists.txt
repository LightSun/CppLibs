

file(GLOB_RECURSE SRC_ALGOS
    src/*.h
    src/*.cpp
    src/*.hpp)

add_library(h7Algo
    ${SRC_ALGOS}
    )

file(GLOB_RECURSE SRC_ALGOS_MAIN
    main/*.h
    main/*.cpp
    main/*.hpp)

add_executable(h7AlgoMain
    ${SRC_ALGOS_MAIN}
    )
