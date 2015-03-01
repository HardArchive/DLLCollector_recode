#-------------------------------------------------
#
# Project created by QtCreator 2014-11-13T10:36:01
# Version 1.1
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
CONFIG += c++14

#Для msvc2013, functions.cpp -> #include <windows.h>
win32-msvc2013{
    DEFINES += NOMINMAX
}

#Версия для разработчиков
DEFINES += DEV_PROJECT

LIBS+= -lpsapi

SOURCES += main.cpp\
        mainwindow.cpp \
    functions.cpp \
    selectprocess.cpp

HEADERS  += mainwindow.h \
    functions.h \
    selectprocess.h \
    debug.h \
    info.h

FORMS    += mainwindow.ui \
    selectprocess.ui

RC_FILE = rc/icon.rc
