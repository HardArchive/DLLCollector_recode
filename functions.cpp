//STL

//Native
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>

//Qt
#include <QtCore>

//Project
#include "functions.h"
#include "mainwindow.h"

QString getWinDir()
{
    return QString(qgetenv("WINDIR"));
}

bool isSubPath(const QString& dir, const QString& path)
{
    if (dir.isEmpty())
        return false;

    QString nativeDir = QDir::toNativeSeparators(dir);
    QString nativePath = QDir::toNativeSeparators(path);

    if (nativeDir.lastIndexOf(QDir::separator()) != nativeDir.size() - 1) {
        nativeDir += QDir::separator();
    }

    return !nativePath.indexOf(nativeDir, Qt::CaseInsensitive);
}

bool getModulesListFromProcessID(int PID, QList<QString>& modules)
{
    auto w2s = [](const wchar_t* str) {
        return QString::fromWCharArray( str );
    };

    MODULEENTRY32 me32;
    me32.dwSize = sizeof(MODULEENTRY32);

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, PID);
    if (hSnap != NULL) {
        if (Module32First(hSnap, &me32)) {
            while (Module32Next(hSnap, &me32)) {
                modules.append(w2s(me32.szExePath));
            }

            CloseHandle(hSnap);
            return true;
        }
        CloseHandle(hSnap);
        return false;
    }

    return false;
}

bool getProcessList(QList<ProcessInfo>& list)
{
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);

    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return false;
    if (Process32First(hProcessSnap, &pe32)) {
        do {
            QString tmpName = QString::fromWCharArray(pe32.szExeFile);
            qint32 tmpPID = pe32.th32ProcessID;
            list.append({ tmpName, tmpPID });
        } while (Process32Next(hProcessSnap, &pe32));

        CloseHandle(hProcessSnap);
        return true;
    }

    CloseHandle(hProcessSnap);
    return false;
}

QString getFilePathFromPID(int PID)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, PID);
    if (hProcess == 0) {
        addLogErr(QObject::trUtf8("Ошибка открытия процесса!"));
        return QString();
    }

    wchar_t bufPath[MAX_PATH]{};
    DWORD result = GetModuleFileNameEx(hProcess, NULL, bufPath, MAX_PATH - 1);
    CloseHandle(hProcess);

    if (result == 0) {
        addLogErr(QObject::trUtf8("Ошибка получения пути к образу процесса!"));
        return QString();
    }

    return QString::fromWCharArray(bufPath);
}

qint64 getPIDFromHWND(qintptr hWnd)
{
    DWORD tmpPID{};
    GetWindowThreadProcessId(reinterpret_cast<HWND>(hWnd), &tmpPID);

    return tmpPID;
}

qintptr getHWindowFromPoint(const QPoint& point)
{
    POINT tmpPoint = { point.x(), point.y() };
    HWND tmpHWnd = WindowFromPoint(tmpPoint);

    return reinterpret_cast<qintptr>(tmpHWnd);
}

QString findPathQt()
{
    //Ищем в параметрах среды
    const wchar_t* QT_CORE_FILE = L"Qt5Core.dll";
    const DWORD nBufferLength = 255;
    wchar_t lpBuffer[nBufferLength];
    bool findEnv = SearchPath(NULL,
                              QT_CORE_FILE,
                              NULL,
                              nBufferLength,
                              (LPWSTR)&lpBuffer,
                              NULL);
    if (findEnv) {
        QString tmp = QString::fromWCharArray(lpBuffer);
        QDir dir(tmp);
        dir.cdUp();
        dir.cdUp();
        return dir.absolutePath();
    } else {
    }

    //Ищем в реестре
    QString regKey = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";
    QSettings uninstall(regKey, QSettings::NativeFormat);

    QStringList qtKeys = uninstall.childGroups().filter("Qt ");
    QString qtPath;
    if (!qtKeys.isEmpty()) {
        regKey.append(qtKeys.first());
        qtPath = QSettings(regKey, QSettings::NativeFormat).value("InstallLocation").toString();
    }

    //Ищем в директории Qt
    const QString QT_LOG_FILE = "InstallationLog.txt";

    if (!qtPath.isEmpty()) {
        QFile qtLog(qtPath + QDir::separator() + QT_LOG_FILE);
        qtLog.open(QFile::ReadOnly);
        QString strLog = qtLog.readAll();

        QString var1 = "qtenv2.bat";
        int qtenv2 = strLog.indexOf(var1);

        QString var2 = "arguments: ";
        int arguments = strLog.indexOf(var2, qtenv2 - 100);
        arguments += var2.size();

        QString tmp = strLog.mid(arguments, qtenv2 - arguments);

        QDir dir(tmp);
        dir.cdUp();

        return dir.absolutePath();
    }

    return QString();
}
