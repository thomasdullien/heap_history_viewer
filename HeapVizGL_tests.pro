#-------------------------------------------------
#
# Project created by QtCreator 2016-05-12T12:51:20
#
#-------------------------------------------------

QT       += core gui opengl testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HeapVizGL
TEMPLATE = app

SOURCES += heapvizwindow.cpp \
    glheapdiagram.cpp \
    heapblock.cpp \
    vertex.cpp \
    transform3d.cpp \
    heaphistory.cpp \
    displayheapwindow.cpp \
    heapwindow.cpp \
    testdisplayheapwindow.cpp \
    glheapdiagramlayer.cpp \
    activeregionsdiagramlayer.cpp \
    addressdiagramlayer.cpp \
    eventdiagramlayer.cpp \
    glsl_simulation_functions.cpp \
    gridlayer.cpp \
    heapblockdiagramlayer.cpp \
    linearbrightnesscolorscale.cpp \
    testactiveregioncache.cpp \
    activeregioncache.cpp

HEADERS  += heapvizwindow.h \
    glheapdiagram.h \
    heapblock.h \
    vertex.h \
    transform3d.h \
    heaphistory.h \
    json.hpp \
    displayheapwindow.h \
    heapwindow.h \
    testdisplayheapwindow.h \
    glheapdiagramlayer.h \
    activeregionsdiagramlayer.h \
    addressdiagramlayer.h \
    eventdiagramlayer.h \
    glsl_simulation_functions.h \
    gridlayer.h \
    heapblockdiagramlayer.h \
    LICENSE_json.h \
    linearbrightnesscolorscale.h \
    ui_heapvizwindow.h \
    testactiveregioncache.h \
    activeregioncache.h

FORMS    += heapvizwindow.ui

#DISTFILES += \

RESOURCES += \
    resource.qrc
