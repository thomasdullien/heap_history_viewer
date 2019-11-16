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
find_package(OpenGL REQUIRED)

add_executable(HeapVizGL
        heapvizwindow.ui
        main.cpp
        heapvizwindow.cpp
        glheapdiagram.cpp
        heapblock.cpp
        vertex.cpp
        transform3d.cpp
        heaphistory.cpp
        displayheapwindow.cpp
        heapwindow.cpp
        glheapdiagramlayer.cpp
        gridlayer.cpp
        linearbrightnesscolorscale.cpp
        heapblockdiagramlayer.cpp
        eventdiagramlayer.cpp
        addressdiagramlayer.cpp
        glsl_simulation_functions.cpp
        activeregionsdiagramlayer.cpp
        activeregioncache.cpp
        resource.qrc
        )

target_link_libraries(HeapVizGL Qt5::Widgets OpenGL::GL ${CONAN_LIBS})
