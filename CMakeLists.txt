cmake_minimum_required(VERSION 3.10)
project(heap_history_viewer CXX)

set(CMAKE_CPP_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(conan.cmake)
conan_cmake_run(CONANFILE conanfile.py BASIC_SETUP CMAKE_TARGETS BUILD missing)
if (NOT DEFINED ENV{CMAKE_PREFIX_PATH})
    file(COPY ${CMAKE_BINARY_DIR}/qt.conf DESTINATION ${CMAKE_BINARY_DIR}/bin/)
endif()

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
        CONAN_PKG::gflags
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
