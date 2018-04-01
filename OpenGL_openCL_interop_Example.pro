#-------------------------------------------------
#
# Project created by QtCreator 2018-03-16T08:56:11
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS += -msse4.2 -mtune=corei7-avx -Ofast

TARGET = OpenGL_openCL_interop
TEMPLATE = app
DESTDIR = $$PWD/bin


SOURCES += main.cpp\
        mainwindow.cpp \
    displaywindow.cpp \
    myshader.cpp

HEADERS  += mainwindow.h \
    displaywindow.h \
    openglwindow.h \
    myshader.h

FORMS    += mainwindow.ui

DISTFILES += \
    kernels/conv2lum.cl \
    shaders/bypass_v.glsl

unix:!macx: LIBS += -L$$PWD/../../../../../usr/lib/x86_64-linux-gnu/ -lOpenCL
INCLUDEPATH += $$PWD/../../../../../usr/lib/x86_64-linux-gnu
DEPENDPATH += $$PWD/../../../../../usr/lib/x86_64-linux-gnu

#unix:!macx: LIBS += -L$$PWD/../../../../../opt/intel/opencl/ -lOpenCL

#INCLUDEPATH += $$PWD/../../../../../opt/intel/opencl/include
#DEPENDPATH += $$PWD/../../../../../opt/intel/opencl/include
