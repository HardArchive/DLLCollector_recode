//STL

//Native
#include <Windows.h>
#include <dbghelp.h>
#include <io.h>

//Qt
#include <QApplication>
#include <QTextStream>
#include <QTextLine>
#include <QTime>
#include <QDebug>
#include <QMessageBox>
#include <QFile>

//Project
#include "mainwindow.h"
#include "info.h"

static MainWindow* local;
void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QTextStream cerr(stderr);
    const QString LS = "\n";
    const QString BEGIN = "====== %1 ======" + LS;
    const QString time = "Time: " + QTime::currentTime().toString("H:m:s") + LS;
    const QString file = "File: " + QString(context.file) + LS;
    const QString function = "Function: " + QString(context.function) + LS;
    const QString line = "Line: " + QString::number(context.line) + LS;
    const QString message = "Message: " + msg + LS;

    const QString formatedMessage = BEGIN + time + file + line + function + message;

    auto makeMessage = [&](const QString& prefix) {
        cerr << formatedMessage.arg(prefix) << endl;
        
        if(local != nullptr){
            local->_addLog(context.function, prefix + ": " + msg);
        }
    };

    switch (type) {
    case QtDebugMsg:
        makeMessage("debug");
        break;
    case QtWarningMsg:
        makeMessage("warning");
        break;
    case QtCriticalMsg:
        makeMessage("critical");
        break;
    case QtFatalMsg:
        makeMessage("fatal");
        abort();
    }
}

#ifdef _MSC_BUILD
LONG WINAPI CustomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExInfo)
{
    const QString progVer = QString("_v%1.%2_").arg(Info::MAJOR).arg(Info::MINOR);
    const QString fileNameDump = Info::ApplicationName + progVer + "dump.dmp";
    QFile file(fileNameDump);
    file.open(QFile::WriteOnly);
    HANDLE hFile = (HANDLE)_get_osfhandle(file.handle());

    MINIDUMP_EXCEPTION_INFORMATION info;
    info.ThreadId = GetCurrentThreadId();
    info.ExceptionPointers = pExInfo;
    info.ClientPointers = FALSE;

    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile,
                      MiniDumpNormal, &info, NULL, NULL);

    QMessageBox::critical(0,
                          QObject::trUtf8("Упс!"),
                          QString("%1\n%2")
                              .arg(QObject::trUtf8("Произошла критическая ошибка!"))
                              .arg(QObject::trUtf8("Создан файл с информацией об ошибке: ").append(fileNameDump)));

    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

int main(int argc, char* argv[])
{

#ifdef _MSC_BUILD
    SetUnhandledExceptionFilter(CustomUnhandledExceptionFilter);
#endif

    qInstallMessageHandler(myMessageOutput);
    setlocale(LC_ALL, "Russian");

    QApplication a(argc, argv);

    MainWindow w;
    local = &w;
    w.show();

    return a.exec();
}
