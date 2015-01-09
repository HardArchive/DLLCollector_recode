//STL

//Native

//Qt
#include <QApplication>
#include <QTextStream>
#include <QTextLine>
#include <QTime>

//Project
#include "mainwindow.h"

MainWindow* local;
void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QTextStream cerr(stderr);
    QString DIV = "; ";
    QString LS = "\n";
    QString BEGIN = "====== %1 ======" + LS;
    QString time = "Time: " + QTime::currentTime().toString("H:m:s") + LS;
    QString file = "File: " + QString(context.file) + LS;
    QString function = "Function: " + QString(context.function) + LS;
    QString line = "Line: " + QString::number(context.line) + LS;
    QString message = "Message: " + msg + LS;

    QString formatedMessage = BEGIN + time + file + line + function + message;

    auto makeMessage = [&](const QString& prefix) {
        cerr << formatedMessage.arg(prefix) << endl;
        
        if(local != nullptr){
            local->_addLog(context.function, prefix + ": " + msg);
        }
    };

    switch (type) {
    case QtDebugMsg:
        makeMessage("debug");
        break;
    case QtWarningMsg:
        makeMessage("warning");
        break;
    case QtCriticalMsg:
        makeMessage("critical");
        break;
    case QtFatalMsg:
        makeMessage("fatal");
        abort();
    }
}

int main(int argc, char* argv[])
{
    qInstallMessageHandler(myMessageOutput);
    setlocale(LC_ALL, "Russian");

    QApplication a(argc, argv);
    MainWindow w;
    local = &w;
    w.show();

    return a.exec();
}
