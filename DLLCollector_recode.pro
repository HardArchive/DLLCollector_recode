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

#Версия для разработчиков
DEFINES += DEV_PROJECT

LIBS += -lpsapi

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

#3rd

#Для msvc2013
win32-msvc*{
    #functions.cpp -> #include <windows.h>
    DEFINES += NOMINMAX

    #Подключаем QCrashReport
    LIBS += -lDbgHelp
    QMAKE_CXXFLAGS_RELEASE += -Zi
    QMAKE_LFLAGS_RELEASE += /DEBUG
    
}





