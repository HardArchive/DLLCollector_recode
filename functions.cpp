//WinAPI
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>

//Qt
#include <QtCore>

//Project
#include "functions.h"

bool isSubPath(const QString &dir, const QString &path)
{
    if ( dir.isEmpty() ) return false;
    
    QString nativeDir = QDir::toNativeSeparators(dir);
    QString nativePath = QDir::toNativeSeparators(path);
    
    if( nativeDir.lastIndexOf(QDir::separator()) != nativeDir.size()-1 )
    {
        nativeDir += QDir::separator();
    }
    
    return !nativePath.indexOf( nativeDir, Qt::CaseInsensitive );
}

bool getModulesListFromProcessID(qint32 PID, QList<QString> &modules)
{
    auto w2s = [](const wchar_t *str)
    {
        return QString::fromWCharArray( str );
    };
    
    MODULEENTRY32 me32;
    me32.dwSize = sizeof(MODULEENTRY32);

    HANDLE hSnap;
    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, PID);
    if(hSnap != NULL)
    {
        if (Module32First(hSnap, &me32))
        {
            while(Module32Next(hSnap, &me32))
            {
                modules.append( w2s(me32.szExePath) );
            }
            
            CloseHandle(hSnap);
            return true;
        }
        CloseHandle(hSnap);
        return false;
    }
    
    return false;
}

bool getProcessList(QList<ProcessInfo> &list)
{
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);

    if( hProcessSnap == INVALID_HANDLE_VALUE) return false;
    if( Process32First(hProcessSnap, &pe32) )
    {
        do
        {
            QString tmpName =  QString::fromWCharArray( pe32.szExeFile );
            qint32 tmpPID = pe32.th32ProcessID;
            list.append({tmpName, tmpPID}); 
        }
        while ( Process32Next(hProcessSnap, &pe32) );
        
        CloseHandle(hProcessSnap);
        return true;
    }

    CloseHandle(hProcessSnap);
    return false;
}

bool getFilePathFromPID(qint32 PID, QString &fileName)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, PID);
    if( hProcess == 0 ) return false;

    wchar_t bufPath[MAX_PATH+1]{};
    DWORD result = GetModuleFileNameEx(hProcess, NULL, bufPath, MAX_PATH);
    CloseHandle(hProcess);
    if( result == 0 ) return false;

    fileName = QString::fromWCharArray( bufPath );
    return true;
}

bool getPIDFromHWND(qint32 hWnd, qint32 &PID)
{
    DWORD tmpPID;
    DWORD result = GetWindowThreadProcessId( reinterpret_cast<HWND>(hWnd), &tmpPID);
    if( result == 0 ) return false;
    
    PID = tmpPID;
    return true;
}

bool getHWindowFromPoint(const QPoint &point, qint64 &hWnd)
{
    POINT tmpPoint = {point.x(), point.y()};
    HWND tmpHWnd = WindowFromPoint( tmpPoint );
    if( tmpHWnd == 0 ) return false;
    
    hWnd = reinterpret_cast<qint64>( tmpHWnd );
    return true;
}


QString getWinDir()
{
    return QString( qgetenv("WINDIR") );
}

bool copyFile(const QString &filePath, const QString &outDir)
{
    QFile file(filePath);
    QFileInfo fileInfo(file);
    QString copyFilePath = outDir + QDir::separator() + fileInfo.fileName();
    
    return file.copy( copyFilePath );
}
