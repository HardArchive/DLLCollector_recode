#ifndef FUNCTIONS_H
#define FUNCTIONS_H

//WinAPI

//Qt
#include <QtGlobal>

//Project

struct ProcessInfo {
    QString name;
    qint32 pid;
};

QString getWinDir();
bool isSubPath(const QString& dir, const QString& path);
bool getModulesListFromProcessID(int PID, QList<QString>& modules);
bool getProcessList(QList<ProcessInfo>& list);
QString getFilePathFromPID(int PID);
int getPIDFromHWND(int hWnd);
int getHWindowFromPoint(const QPoint& point);
bool copyFile(const QString& fileName, const QString& outDir);
QString findPathQt();

#endif // FUNCTIONS_H
