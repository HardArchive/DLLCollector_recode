#ifndef FUNCTIONS_H
#define FUNCTIONS_H

//WinAPI

//Qt
#include <QtGlobal>

//Project

struct ProcessInfo
{
    QString name;
    qint32 pid;
};

QString getWinDir();
bool isSubPath(const QString &dir, const QString &path);
bool getModulesListFromProcessID(qint32 PID, QList<QString> &modules);
bool getProcessList(QList<ProcessInfo> &list);
bool getFilePathFromPID(qint32 PID, QString &fileName);
bool getPIDFromHWND(qint32 hWnd, qint32 &PID);
bool getHWindowFromPoint(const QPoint &point, qint64 &hWnd);
bool copyFile(const QString &fileName, const QString &outDir);

#endif // FUNCTIONS_H
