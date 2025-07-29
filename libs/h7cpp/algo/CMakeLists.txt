
#
if(OpenCV_DIR)
    add_compile_definitions(OpenCV_DIR)
    find_package(OpenCV 4 REQUIRED)
    include_directories(${OpenCV_INCLUDE_DIRS})

    message(STATUS "OpenCV library status:")
    message(STATUS "    version: ${OpenCV_VERSION}")
    message(STATUS "    libraries: ${OpenCV_LIBS}")
    message(STATUS "    config: ${OpenCV_DIR}")
    message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")
endif(OpenCV_DIR)

##
file(GLOB_RECURSE SRC_ALGOS
    src/*.h
    src/*.cpp
    src/*.hpp
    op_perf/*.h
    op_perf/*.hpp
    op_perf/*.cpp
    )

add_executable(h7Algo
    ${SRC_ALGOS}
    )

file(GLOB_RECURSE SRC_ALGOS_MAIN
    main/*.h
    main/*.cpp
    main/*.hpp)

add_executable(h7AlgoMain
    ${SRC_ALGOS_MAIN}
    )
if(OpenCV_DIR)
    target_link_libraries(h7AlgoMain ${OpenCV_LIBS})
endif(OpenCV_DIR)
