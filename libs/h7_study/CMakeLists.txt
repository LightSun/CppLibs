cmake_minimum_required(VERSION 3.5)
project(h7_study LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(common
    lang/avl_tree.h
    lang/avl_tree.c
    lang/rbtree.h
    lang/rbtree.c
    main.c
    )
