//STL
#include <thread>

//Native
#include <Windows.h>
#include <dbghelp.h>
#include <io.h>

//Qt
#include <QMessageBox>
#include <QFile>

//Project
#include "exceptionhandler.h"
#include "info.h"

#ifdef _MSC_BUILD
static const QString progVer = QString("_v%1.%2_").arg(Info::MAJOR).arg(Info::MINOR);
static const QString fileNameDump = Info::ApplicationName + progVer + "dump.dmp";

void WriteDump(PEXCEPTION_POINTERS pExceptionInfo)
{
    QFile file(fileNameDump);
    file.open(QFile::WriteOnly);
    HANDLE hFile = (HANDLE)_get_osfhandle(file.handle());

    MINIDUMP_EXCEPTION_INFORMATION info;
    info.ThreadId = GetCurrentThreadId();
    info.ExceptionPointers = pExceptionInfo;
    info.ClientPointers = FALSE;

    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile,
                      MiniDumpNormal, &info, NULL, NULL);
}

LONG CALLBACK ExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo)
{
    std::thread threadWriteDump(WriteDump, pExceptionInfo);
    threadWriteDump.join();

    QMessageBox::critical(0, "Oooops!",
                          QString("%1\n%2")
                              .arg(QObject::trUtf8("Произошла критическая ошибка!"))
                              .arg(QObject::trUtf8("Создан файл с информацией об ошибке: ").append(fileNameDump)));

    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

void RegisterDumper()
{
#ifdef _MSC_BUILD
    SetUnhandledExceptionFilter(ExceptionHandler);
    //AddVectoredExceptionHandler(1, ExceptionHandler);
#endif
}
