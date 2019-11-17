cmake_minimum_required(VERSION 3.10)
project(heap_history_viewer CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(${CMAKE_SOURCE_DIR}/conan.cmake)

conan_cmake_run(REQUIRES gflags/2.2.2@bincrafters/stable
        BASIC_SETUP
        BUILD missing)

set(CMAKE_CPP_STANDARD 17)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5Test REQUIRED)

# Not sure which OpenGL_GL_PREFERENCE is the best - the alternative is LEGACY
set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)

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
