//WinAPI

//Qt
#include <QApplication>
#include <QTextStream>
#include <QTextLine>
#include <QTime>

//Project
#include "mainwindow.h"

MainWindow *local;
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context)
    QTextStream cerr(stderr);
    
    QString fomratedText = QTime::currentTime().toString("H:m:s: ") + msg;
    
    switch (type)
    {
    case QtDebugMsg:
        
        cerr << "debug_" << fomratedText << endl;
        
        if(local != nullptr)
            local->addLog("debug_" + fomratedText);
        break;
    case QtWarningMsg:
        cerr << "warning_" << fomratedText << endl;
        
        if(local != nullptr)
            local->addLog("warning_" + fomratedText);
        break;
    case QtCriticalMsg:
        cerr << "critical_" << fomratedText << endl;
        
        if(local != nullptr)
            local->addLog("critical_" + fomratedText);
        break;
    case QtFatalMsg:
        cerr << "fatal_" << fomratedText << endl;
        
        if(local != nullptr)
            local->addLog("fatal_" + fomratedText);
        abort();
    }
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(myMessageOutput);
    setlocale( LC_ALL, "Russian" );
    
    QApplication a(argc, argv);
    MainWindow w;
    local = &w;
    w.show();
    
    return a.exec();
}
