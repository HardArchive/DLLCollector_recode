#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//WinAPI

//Qt
#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QTreeWidgetItem;
class QSettings;
QT_END_NAMESPACE

//Project
#include "selectprocess.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    const QString SETTING_KEY = "MinGWQtDir";
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool eventFilter(QObject *obj, QEvent *event);


    //Private variables
private:
    Ui::MainWindow *ui;
    QSettings *settings;
    qint64 m_hWnd{};
    qint32 m_PID{};
    QString m_exePath{};
    QString m_windowsDir{};
    
private slots:
    void do_toolButton_HWnd_release();
    void on_toolButton_HWnd_pressed();
    void on_toolButton_PID_clicked();
    void on_toolButton_Exe_clicked();
    void on_toolButton_CopyDir_clicked();
    void on_toolButton_Exec_clicked();
    void on_toolButton_Qt_clicked();
    void on_toolButton_Copy_clicked();
    void on_treeWidget_itemChanged(QTreeWidgetItem *item, int column);
    void on_pushButton_ClearLog_clicked();
    
    void clearFields();
    void setHWnd(bool status);
    void setPID(bool status);
    void setExe(bool status);
    void processSelected(qint32 PID);
    void updateDependencyTree();
    
public slots:
    void addLog(const QString &str);
    
    // QWidget interface
protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
};

#endif // MAINWINDOW_H
