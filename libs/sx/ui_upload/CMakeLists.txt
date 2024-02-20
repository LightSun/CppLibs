cmake_minimum_required(VERSION 3.5)

project(ui_upload LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

#set(LINK_QT_STATIC TRUE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#-static-libgcc -static-libstdc++
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc -static-libstdc++")
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -std=c11 -g -O0 -DUSE_C11_ATOMICS")

option(BUILD_FOR_MINGW64 "build for mingw64" ON)
#set(QT_STATIC_PATH "C:/Users/Administrator/Downloads/qtbase-everywhere-src-5.15.2")

message(STATUS "BUILD_FOR_MINGW64: ${BUILD_FOR_MINGW64}")

#SET(QT_CMAKE_DIR $ENV{QT_CMAKE_DIR})
#if(NOT QT_CMAKE_DIR)
#    set(QT_CMAKE_DIR D:/study/cpp/qt_libs/build-qtbase-6.4.2-Desktop_Qt_5_12_9_MinGW_64_bit-Debug/lib/cmake)
#    set(QT_CMAKE_DIR ${QT_STATIC_PATH}/lib/cmake)
#endif()

#if(${LINK_QT_STATIC})
#    set(CMAKE_PREFIX_PATH ${QT_STATIC_PATH})
#    message(STATUS "Link with Qt static lib")
#    message(STATUS "QT_STATIC_PATH: ${QT_STATIC_PATH}")
#    message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
#else()
#    message(STATUS "Link with Qt shared lib")
#endif()

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)


if(BUILD_FOR_MINGW64)
    add_library(upload_core STATIC
        pub_api.h
        pub_api.cpp
        qt_Handler.h
    )
    #target_link_libraries(ui_upload PRIVATE Qt5::Widgets)
    target_link_libraries(upload_core PRIVATE Qt5::Widgets)
else()
#    add_executable(ui_upload
#        main.cpp
#        mainwindow.cpp
#        mainwindow.h
#        mainwindow.ui
#        pub_api.h
#        pub_api.cpp
#    )
    add_library(upload_core SHARED
        pub_api.h
        pub_api.cpp
        qt_Handler.h
    )
    #target_link_libraries(ui_upload PRIVATE Qt5::Widgets)
    target_link_libraries(upload_core PRIVATE Qt5::Widgets)
endif()

#gtk
#add_library(upload_core_gtk SHARED
#    pub_api.h
#    pub_api_gtk.cpp
#    )

if(BUILD_FOR_MINGW64)
    #install gtk3: https://github.com/GtkSharp/GtkSharp/wiki/Installing-Gtk-on-Windows
    SET(MINGW_64_DIR C:/heaven7/msys64/mingw64)
    set(MINGW_INC_DIR ${MINGW_64_DIR}/include)

#    target_include_directories(upload_core_gtk PUBLIC
#        ${MINGW_INC_DIR}/gtk-3.0
#        ${MINGW_INC_DIR}/glib-2.0
#        ${MINGW_INC_DIR}/harfbuzz
#        ${MINGW_64_DIR}/lib/glib-2.0/include
#        ${MINGW_INC_DIR}/atk-1.0
#        ${MINGW_INC_DIR}/gdk-pixbuf-2.0
#        ${MINGW_INC_DIR}/cairo
#        ${MINGW_INC_DIR}/pango-1.0
#        ${MINGW_INC_DIR}/glib-2.0/include
#        )
#    target_link_libraries(upload_core_gtk PRIVATE
#        gtk-3
#        gobject-2.0
#        pangocairo-1.0
#        gio-2.0
#        atk-1.0
#        gdk-3
#        glib-2.0
#        )
else(BUILD_FOR_MINGW64)
    target_include_directories(upload_core_gtk PUBLIC
        /usr/include/gtk-3.0
        /usr/include/glib-2.0/
        /usr/include/atk-1.0/
        /usr/include/gdk-pixbuf-2.0/
        /usr/include/cairo/
        /usr/include/pango-1.0/
        /usr/lib/x86_64-linux-gnu/glib-2.0/include/
        )
    target_link_libraries(upload_core_gtk PRIVATE
        gtk-3
        gobject-2.0
        pangocairo-1.0
        gio-2.0
        atk-1.0
        gdk-3
        glib-2.0
        )
endif()

add_executable(
    test_ui
    h_atomic.h
    h_atomic.c
    CountDownLatch.h
    main_gtk.cpp
    )
if(BUILD_FOR_MINGW64)
    target_link_libraries(test_ui PRIVATE upload_core)
else()
    target_link_libraries(test_ui PRIVATE upload_core_gtk pthread)
endif()
