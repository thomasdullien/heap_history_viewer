cmake_minimum_required(VERSION 3.10)
project(heap_history_viewer CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(${CMAKE_SOURCE_DIR}/conan.cmake)

if (DEFINED ENV{CMAKE_PREFIX_PATH})
    message(STATUS "Getting Qt from the system. CMAKE_PREFIX_PATH = $ENV{CMAKE_PREFIX_PATH}")
    set(NEEDED_CONAN_DEPENDENCIES gflags/2.2.2@bincrafters/stable)
else ()
    message(STATUS "To use the Qt from your system, set the CMAKE_PREFIX_PATH env var")
    message(STATUS "Trying to get Qt from Conan")
    message(STATUS "If it fails try: 'conan remote add bincrafters \"https://api.bintray.com/conan/bincrafters/public-conan\"'")
    set(NEEDED_CONAN_DEPENDENCIES gflags/2.2.2@bincrafters/stable qt/5.12.6@bincrafters/stable)
endif ()

conan_cmake_run(REQUIRES ${NEEDED_CONAN_DEPENDENCIES} BASIC_SETUP BUILD missing)

set(CMAKE_CPP_STANDARD 17)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5Test REQUIRED)

if(APPLE)
    # Silence OpenGL deprication warnings
    add_definitions(-DGL_SILENCE_DEPRECATION)
endif()

# Not sure which OpenGL_GL_PREFERENCE is the best - the alternative is LEGACY
set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)

# Warnings:
# TODO(patricia-gallardo): Fix all of these
if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set(EXTRA_WARNINGS /WX) # -Werror
    # /wd4244 disables warings around conversion to or from 'long double', possible loss of data
    set(TEMPORARILY_DISABLED_WARNINGS /wd4244)
else ()
    set(EXTRA_WARNINGS -Werror -Wall -Wextra)
    set(TEMPORARILY_DISABLED_WARNINGS)
endif ()

add_executable(HeapVizGL
        heapvizwindow.ui
        resource.qrc
        activeregioncache.cpp
        activeregionsdiagramlayer.cpp
        addressdiagramlayer.cpp
        displayheapwindow.cpp
        eventdiagramlayer.cpp
        glheapdiagram.cpp
        glheapdiagramlayer.cpp
        glsl_simulation_functions.cpp
        gridlayer.cpp
        heapblock.cpp
        heapblockdiagramlayer.cpp
        heaphistory.cpp
        heapvizwindow.cpp
        heapwindow.cpp
        linearbrightnesscolorscale.cpp
        main.cpp
        transform3d.cpp
        vertex.cpp)

target_link_libraries(HeapVizGL
        ${CONAN_LIBS}
        OpenGL::GL
        Qt5::Widgets)

target_compile_options(HeapVizGL PRIVATE
        ${EXTRA_WARNINGS}
        ${TEMPORARILY_DISABLED_WARNINGS})

add_executable(HeapVizGLTest
        heapvizwindow.ui
        resource.qrc
        activeregioncache.cpp
        activeregionsdiagramlayer.cpp
        addressdiagramlayer.cpp
        displayheapwindow.cpp
        eventdiagramlayer.cpp
        glheapdiagram.cpp
        glheapdiagramlayer.cpp
        glsl_simulation_functions.cpp
        gridlayer.cpp
        heapblock.cpp
        heapblockdiagramlayer.cpp
        heaphistory.cpp
        heapvizwindow.cpp
        heapwindow.cpp
        linearbrightnesscolorscale.cpp
        testactiveregioncache.cpp
        testdisplayheapwindow.cpp
        transform3d.cpp
        vertex.cpp)

target_link_libraries(HeapVizGLTest
        OpenGL::GL
        Qt5::Test
        Qt5::Widgets)

target_compile_options(HeapVizGLTest PRIVATE
        ${EXTRA_WARNINGS}
        ${TEMPORARILY_DISABLED_WARNINGS})
