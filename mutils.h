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
bool getModulesListFromProcessID(int PID, QStringList &modules);
bool getProcessList(QList<ProcessInfo> &list);
QString getFilePathFromPID(uint PID);
uint getPIDFromHWND(qintptr hWnd);
qintptr getHWindowFromPoint(const QPoint &point);
QString findPathQt();

#endif // MUTILS_H
