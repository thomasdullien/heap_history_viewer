cmake_minimum_required(VERSION 3.10)
project(heap_history_viewer CXX)
# https://github.com/conan-io/cmake-conan

if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(WARNING "Building for anything but 64 bit is currently untested")
endif ()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(${CMAKE_SOURCE_DIR}/conan.cmake)

conan_check(VERSION 1.20.3 REQUIRED)

if (DEFINED ENV{CMAKE_PREFIX_PATH})
    message(STATUS "Getting Qt from the system. CMAKE_PREFIX_PATH = $ENV{CMAKE_PREFIX_PATH}")
    set(PROJECT_CONAN_FILE conanfile_no_qt.txt)
else ()
    message(STATUS "To use the Qt from your system, set the CMAKE_PREFIX_PATH env var")
    message(STATUS "Trying to get Qt from Conan - adding the bincrafters remote")
    conan_add_remote(NAME bincrafters INDEX 1 URL https://api.bintray.com/conan/bincrafters/public-conan)
    set(PROJECT_CONAN_FILE conanfile_qt.txt)
endif ()

conan_cmake_run(CONANFILE ${PROJECT_CONAN_FILE} BASIC_SETUP BUILD missing)

if (QT_GENERATOR)
    # Copy the qt.conf file to be next to the binary
    message(STATUS "Copy qt.conf from ${CMAKE_CURRENT_BINARY_DIR} to ${CMAKE_CURRENT_BINARY_DIR}/bin/")
    file(COPY ${CMAKE_CURRENT_BINARY_DIR}/qt.conf DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bin/)
endif ()

set(CMAKE_CPP_STANDARD 17)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5Test REQUIRED)

if (APPLE)
    # Silence OpenGL deprication warnings
    add_definitions(-DGL_SILENCE_DEPRECATION)
endif ()

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

set(COMMON_FILES
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
        transform3d.cpp
        vertex.cpp)

add_executable(HeapVizGL ${COMMON_FILES} main.cpp)

target_link_libraries(HeapVizGL
        ${CONAN_LIBS}
        OpenGL::GL
        Qt5::Widgets)

target_compile_options(HeapVizGL PRIVATE
        ${EXTRA_WARNINGS}
        ${TEMPORARILY_DISABLED_WARNINGS})

add_executable(HeapVizGLTest
        ${COMMON_FILES}
        testactiveregioncache.cpp
        testdisplayheapwindow.cpp)

target_link_libraries(HeapVizGLTest
        OpenGL::GL
        Qt5::Test
        Qt5::Widgets)

target_compile_options(HeapVizGLTest PRIVATE
        ${EXTRA_WARNINGS}
        ${TEMPORARILY_DISABLED_WARNINGS})
