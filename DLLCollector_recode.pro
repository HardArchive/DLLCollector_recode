#-------------------------------------------------
#
# Project created by QtCreator 2014-11-13T10:36:01
# Version 1.1
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

contains(QT_ARCH, i386) {
    TARGET = DLLCollector_recode_x32
} else {
    TARGET = DLLCollector_recode_x64
}

TEMPLATE = app
CONFIG += C++14

#Версия для разработчиков
DEFINES += DEV_PROJECT

SOURCES += main.cpp\
        mainwindow.cpp \
    functions.cpp \
    selectprocess.cpp \
    info.cpp \

HEADERS  += mainwindow.h \
    functions.h \
    selectprocess.h \
    debug.h \
    info.h \

FORMS    += mainwindow.ui \
    selectprocess.ui

RC_FILE = rc/icon.rc

#3rd
LIBS += -lpsapi

#Для msvc2013
win32-msvc*{
    
    #Поддержка Windows XP
    contains(QT_ARCH, i386) {
        QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01
    } else {
        QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.02
    }
    DEFINES += PSAPI_VERSION=1

    #functions.cpp -> #include <windows.h>
    DEFINES += NOMINMAX

    #Подключаем Google BreakPad
    include($$PWD/3rd/Google_BreakPad/crash_handler.pri)
}
