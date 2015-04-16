#---------------------------------------------------------
#
# Project created by QtCreator 3.3.1 - 2014-11-13T10:36:01
# Version 1.2
#
#---------------------------------------------------------

TEMPLATE = app
QT  += core gui widgets
CONFIG += C++14

#Версия для разработчиков
DEFINES += DEV_PROJECT

contains(QT_ARCH, i386) {
    TARGET = DLLCollector_recode_x32
} else {
    TARGET = DLLCollector_recode_x64
}

LIBS += -lpsapi

SOURCES += main.cpp \
           mainwindow.cpp \
           selectprocess.cpp \
           info.cpp \
    mutils.cpp

HEADERS += mainwindow.h \
           selectprocess.h \
           debug.h \
           info.h \
    mutils.h

FORMS   += mainwindow.ui \
           selectprocess.ui \

RC_FILE += rc/icon.rc

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
}


