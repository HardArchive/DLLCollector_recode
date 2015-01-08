#ifndef FUNCTIONS_H
#define FUNCTIONS_H

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
bool isSubPath(const QString& dir, const QString& path);
bool getModulesListFromProcessID(int PID, QList<QString>& modules);
bool getProcessList(QList<ProcessInfo>& list);
QString getFilePathFromPID(int PID);
int getPIDFromHWND(int hWnd);
int getHWindowFromPoint(const QPoint& point);
QString findPathQt();

#endif // FUNCTIONS_H
