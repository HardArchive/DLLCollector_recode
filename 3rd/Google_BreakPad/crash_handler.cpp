//STL
#include <thread>

//Native
#include <Windows.h>
#include <io.h>

//Qt
#include <QtCore>
#include <QString>
#include <QDir>
#include <QProcess>
#include <QMessageBox>
#include <QtDebug>

//Project
#include "crash_handler.h"
#include "info.h"
#include "client/windows/handler/exception_handler.h"

using namespace google_breakpad;

static ExceptionHandler* pHandler;
static const QString progVer = QString("_v%1.%2_").arg(Info::MAJOR).arg(Info::MINOR);
static const QString fileNameDump = Info::ApplicationName + progVer + "dump.dmp";
static QString appFilePath;
static QStringList args;
static QFile file(fileNameDump);
static QString appPath;

bool consumeInvalidHandleExceptions(void* context,
                                    EXCEPTION_POINTERS* exinfo,
                                    MDRawAssertionInfo* assertion)
{
    
    args << "crash" << QString::number(GetCurrentProcessId());
    QProcess::startDetached(appFilePath, args);

    SleepEx(-1, false);
    return true;
}

bool setPrivilege(HANDLE hToken, const LPCTSTR lpPrivilege, bool bEnablePrivilege)
{
    TOKEN_PRIVILEGES tkp;
    LUID luid;
    TOKEN_PRIVILEGES tkpPrevious;
    DWORD cbPrevious = 0;

    if (!LookupPrivilegeValue(NULL, lpPrivilege, &luid))
        return false;
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = luid;
    tkp.Privileges[0].Attributes = 0;
    cbPrevious = sizeof(TOKEN_PRIVILEGES);
    AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(TOKEN_PRIVILEGES), &tkpPrevious, &cbPrevious);
    if (GetLastError() != ERROR_SUCCESS)
        return false;

    tkpPrevious.PrivilegeCount = 1;
    tkpPrevious.Privileges[0].Luid = luid;
    if (bEnablePrivilege)
        tkpPrevious.Privileges[0].Attributes |= (SE_PRIVILEGE_ENABLED);
    else
        tkpPrevious.Privileges[0].Attributes ^= (SE_PRIVILEGE_ENABLED & tkpPrevious.Privileges[0].Attributes);
    AdjustTokenPrivileges(hToken, FALSE, &tkpPrevious, cbPrevious, NULL, NULL);
    if (GetLastError() != ERROR_SUCCESS)
        return false;

    return true;
}

bool setDebugPrivilege(bool bEnable)
{
    bool result = true;

    HANDLE hToken = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        result = false;
    } else if (!setPrivilege(hToken, SE_DEBUG_NAME, bEnable)) {
        result = false;
    }

    CloseHandle(hToken);
    return result;
}

void RegisterDumper()
{
#ifdef _MSC_BUILD

    QStringList args = QCoreApplication::arguments();
    if (args.contains("crash")) {
        if (!setDebugPrivilege(false)) {
            return;
        };

        DWORD processID = (DWORD)(args.at(2).toULongLong());
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE, FALSE, processID);

        file.open(QFile::WriteOnly);
        HANDLE hFile = (HANDLE)_get_osfhandle(file.handle());

        MiniDumpWriteDump(hProcess,
                          processID,
                          hFile,
                          MiniDumpNormal, nullptr, nullptr, nullptr);
        file.close();

        TerminateProcess(hProcess, 0);
        CloseHandle(hProcess);

        QMessageBox::information(0, "Ooooops!", "Произошла критическая ошибка!");
        ExitProcess(0);
    }

    appFilePath = QCoreApplication::applicationFilePath();
    pHandler = new ExceptionHandler(L"",
                                    consumeInvalidHandleExceptions,
                                    0,
                                    0,
                                    ExceptionHandler::HANDLER_ALL);

#endif
}
