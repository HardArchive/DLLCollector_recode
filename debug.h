#ifndef DEBUG
#define DEBUG

//STL

//Native

//Qt
#include <QDebug>

//Project
#include "mainwindow.h"

#define addLog(message) MainWindow::_addLog(Q_FUNC_INFO, message)
#define addLogErr(message) MainWindow::_addLog(Q_FUNC_INFO, message, c_warning)

#endif // DEBUG

