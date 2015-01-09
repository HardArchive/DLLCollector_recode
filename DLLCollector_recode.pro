#-------------------------------------------------
#
# Project created by QtCreator 2014-11-13T10:36:01
# Version 1.0
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

contains(QT_ARCH, x86_64) {
    TARGET = DLLCollector_recode_x64
} else {
    TARGET = DLLCollector_recode_x32
}

TEMPLATE = app
CONFIG += C++14

LIBS+= -lpsapi

SOURCES += main.cpp\
        mainwindow.cpp \
    functions.cpp \
    selectprocess.cpp

HEADERS  += mainwindow.h \
    functions.h \
    selectprocess.h \
    debug.h

FORMS    += mainwindow.ui \
    selectprocess.ui
