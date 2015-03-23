HEADERS += $$PWD/crash_handler.h
SOURCES += $$PWD/crash_handler.cpp

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/src

win32{
    LIBS += -lDbgHelp -lAdvapi32
    QMAKE_CXXFLAGS_RELEASE += -Zi
    QMAKE_LFLAGS_RELEASE += /INCREMENTAL:NO /DEBUG /OPT:REF
    
    HEADERS += $$PWD/src/client/windows/handler/exception_handler.h
    SOURCES += $$PWD/src/client/windows/handler/exception_handler.cc
    
    SOURCES += $$PWD/src/common/windows/guid_string.cc
    
    SOURCES += $$PWD/src/client/windows/crash_generation/crash_generation_client.cc

    HEADERS += $$PWD/client/windows/sender/crash_report_sender.h
}
