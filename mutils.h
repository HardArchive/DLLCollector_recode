#ifndef MUTILS_H
#define MUTILS_H

//STL

//Native

//Qt
#include <QtGlobal>

//Project

struct ProcessInfo {
    QString name;
    qint32 pid;
};

QString getWinDir();
bool isSubPath(const QString &dir, const QString &path);
bool getModulesListFromProcessID(int PID, QList<QString> &modules);
bool getProcessList(QList<ProcessInfo> &list);
QString getFilePathFromPID(int PID);
qint64 getPIDFromHWND(qintptr hWnd);
qintptr getHWindowFromPoint(const QPoint &point);
QString findPathQt();

#endif // MUTILS_H
