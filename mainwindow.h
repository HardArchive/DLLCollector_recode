#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//STL

//Native

//Qt
#include <QMainWindow>
#include <QProcess>

QT_BEGIN_NAMESPACE
class QTreeWidgetItem;
class QTableWidget;
class QSettings;
QT_END_NAMESPACE

//Project
#include "selectprocess.h"
#include "debug.h"

namespace Ui
{
    class MainWindow;
}

enum TypesMessage {
    c_general,
    c_debug,
    c_warning,
    c_critical,
    c_fatal,
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
private:
    enum ItemTypes {
        ItemMain,
        ItemPlugin,
        ItemSystem,
        ItemOther,
    };
    struct Profile {
        QString qtLibs;
        QString qtPlugins;
        QStringList inclusions;
    };
    
private:
    Ui::MainWindow *ui{};
    QMap<QString, Profile> m_profiles{};
    QStringList m_inclusions;
    QSettings *m_settings{};
    QProcess m_process{};
    QList<QAction *> m_actions{};
    QStringList m_moduleList{};
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
    static QTableWidget *m_log;
    
private slots:
    void clearFields();
    void saveSettings();
    void loadSettings();
    void setHWnd(qintptr hWnd);
    void setPID(uint PID);
    void setExe(const QString &path);
    void setCopyTo(const QString &path);
    void setQtLibs(const QString &path);
    void setQtPlugins(const QString &path);
    void processSelected(uint PID);
    void processStarted();
    void processFinished(int exitStatus);
    void processError();
    
    //Дерево зависимостей
    void addInclusionFromTree();
    void deleteInclusionFromTree();
    void editorInclusionFromTree();
    void initActionsDependencyTree();
    Qt::CheckState isInclusion(const QString &path);
    void updateCheckedTopItemTree(QTreeWidgetItem *topItem, int column = 0);
    void updateDependencyTree(bool updatedModules = true);
    
    void on_treeWidget_DependencyTree_itemChanged(QTreeWidgetItem *item, int column);
    void on_treeWidget_DependencyTree_itemDoubleClicked(QTreeWidgetItem *item, int column);
    void on_treeWidget_DependencyTree_customContextMenuRequested(const QPoint &pos);
    void do_toolButton_HWnd_release();
    void on_toolButton_HWnd_pressed();
    void on_toolButton_PID_clicked();
    void on_toolButton_Exe_clicked();
    void on_toolButton_Exec_clicked();
    void on_toolButton_Kill_clicked();
    void on_toolButton_SelectDirCopyTo_clicked();
    void on_toolButton_CopyTo_clicked();
    void on_comboBox_QtProfil_activated(int idx);
    void on_toolButton_SaveProfil_clicked();
    void on_toolButton_DeleteProfil_clicked();
    void on_toolButton_QtLibs_clicked();
    void on_toolButton_QtPlugins_clicked();
    void on_checkBox_Log_stateChanged(int arg1);
    void on_pushButton_FindQt_clicked();
    void on_pushButton_UpdateTree_clicked();
    void on_pushButton_CleanLog_clicked();
    void on_pushButton_CopyLog_clicked();
    
public slots:
    static void _addLog(const QString &fun, const QString &mes, TypesMessage type = c_general);
    
    // QObject interface
protected:
    bool eventFilter(QObject *obj, QEvent *event);
    
    // QWidget interface
protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void closeEvent(QCloseEvent *event);
    
};

#endif // MAINWINDOW_H
