#ifndef DEBUG
#define DEBUG

//STL

//Native

//Qt
#include <QDebug>

//Project
#include "mainwindow.h"

#define addLog(str) MainWindow::_addLog(Q_FUNC_INFO, str);

#endif // DEBUG

