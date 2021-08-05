cmake_minimum_required(VERSION 3.5)

project(support_test LANGUAGES C)

add_library(stest SHARED
    stest.c
    )

add_executable(support_test main.c)
