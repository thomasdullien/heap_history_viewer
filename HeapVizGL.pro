#-------------------------------------------------
#
# Project created by QtCreator 2016-05-12T12:51:20
#
#-------------------------------------------------

QT       += core gui opengl testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HeapVizGL
TEMPLATE = app

SOURCES += main.cpp \
    heapvizwindow.cpp \
    glheapdiagram.cpp \
    heapblock.cpp \
    vertex.cpp \
    transform3d.cpp \
    heaphistory.cpp \
    displayheapwindow.cpp \
    heapwindow.cpp \
    glheapdiagramlayer.cpp \
    gridlayer.cpp \
    linearbrightnesscolorscale.cpp \
    heapblockdiagramlayer.cpp \
    eventdiagramlayer.cpp \
    addressdiagramlayer.cpp \
    glsl_simulation_functions.cpp

HEADERS  += heapvizwindow.h \
    glheapdiagram.h \
    heapblock.h \
    vertex.h \
    transform3d.h \
    heaphistory.h \
    json.hpp \
    displayheapwindow.h \
    heapwindow.h \
    glheapdiagramlayer.h \
    gridlayer.h \
    linearbrightnesscolorscale.h \
    heapblockdiagramlayer.h \
    eventdiagramlayer.h \
    addressdiagramlayer.h \
    glsl_simulation_functions.h

FORMS    += heapvizwindow.ui

#DISTFILES += \

RESOURCES += \
    resource.qrc
