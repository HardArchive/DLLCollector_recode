#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//STL

//Native

//Qt
#include <QMainWindow>
#include <QProcess>

QT_BEGIN_NAMESPACE
class QTreeWidgetItem;
class QSettings;
QT_END_NAMESPACE

//Project
#include "selectprocess.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    const QString KEY_QTLIBS = "QtLibs";
    const QString KEY_QTPLUGINS = "QtPlugins";

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();
    bool eventFilter(QObject* obj, QEvent* event);

    //Private variables
private:
    Ui::MainWindow* ui;
    QSettings* m_settings;
    int m_hWnd{};
    qint64 m_PID{};
    QString m_exePath{};
    QString m_windowsDir{};
    QString m_QtLibs{};
    QString m_QtPlugins{};
    QProcess m_process;

private slots:
    void clearFields();
    void setHWnd(int hWnd);
    void setPID(qint64 PID);
    void setExe(const QString& str);
    void setQtLibs(const QString& str);
    void setQtPlugins(const QString& str);
    void processSelected(int PID);
    void processStarted();
    void processFinished(int exitStatus);
    void processError();
    void updateDependencyTree();
    void loadSettings();

    void do_toolButton_HWnd_release();
    void on_toolButton_HWnd_pressed();
    void on_toolButton_PID_clicked();
    void on_toolButton_Exe_clicked();
    void on_toolButton_CopyDir_clicked();
    void on_toolButton_Exec_clicked();
    void on_toolButton_Kill_clicked();
    void on_toolButton_QtLibs_clicked();
    void on_toolButton_QtPlugins_clicked();
    void on_toolButton_Copy_clicked();
    void on_treeWidget_itemChanged(QTreeWidgetItem* item, int column);
    void on_pushButton_ClearLog_clicked();
    void on_pushButton_Update_clicked();
    void on_checkBox_Log_clicked(bool checked);
    void on_pushButton_FindQt_clicked();

    
public slots:
    void addLog(const QString& str);

    // QWidget interface
protected:
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);
    void closeEvent(QCloseEvent* event);
};

#endif // MAINWINDOW_H
