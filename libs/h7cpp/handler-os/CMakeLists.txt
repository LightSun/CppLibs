

#set(BUILD_WITH_QT ON)
#
if(BUILD_WITH_QT)
    message(STATUS " build BUILD_WITH_QT ON.")
    add_compile_definitions(BUILD_WITH_QT=1)
    #find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
    find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core Widgets)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

    set(QT_LIBS Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets)
    message(STATUS "QT_LIBS: ${QT_LIBS}")
endif()

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
if(BUILD_WITH_QT)
    set_target_properties(handler_os PROPERTIES AUTOMOC ON)
    set_target_properties(handler_os PROPERTIES AUTOUIC ON)
    set_target_properties(handler_os PROPERTIES AUTORCC ON)
    target_link_libraries(handler_os PUBLIC ${QT_LIBS})
endif()
target_link_libraries(handler_os PUBLIC
    pthread
    )

add_executable(test_handler_os
    ${SRC_EXAMPLE}
    )

target_link_libraries(test_handler_os PUBLIC
    handler_os
    )
if(BUILD_WITH_QT)
    set_target_properties(test_handler_os PROPERTIES AUTOMOC ON)
    set_target_properties(test_handler_os PROPERTIES AUTOUIC ON)
    set_target_properties(test_handler_os PROPERTIES AUTORCC ON)
endif()
