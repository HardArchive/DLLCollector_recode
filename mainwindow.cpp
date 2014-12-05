//WinAPI

//Qt
#include <QtWidgets>
#include <QProcess>
#include <QSettings>
#include <QDebug>
#include <QFileInfo>

//Project
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "functions.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    settings = new QSettings("free", "DLLCollector", this);

    ui->lineEdit_Qt->setText( settings->value(SETTING_KEY).toString() );

    ui->toolButton_HWnd->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if( obj == ui->toolButton_HWnd)
    {
        if( event->type() == QEvent::MouseButtonRelease)
        {
            do_toolButton_HWnd_release();
        }
    }
    
    return QObject::eventFilter(obj, event);
}


void MainWindow::on_toolButton_PID_clicked()
{
    SelectProcess sp(this);
    connect(&sp, SIGNAL(processSelected(qint32)), this, SLOT(processSelected(qint32)));
    sp.show();
    sp.exec();
}

void MainWindow::on_toolButton_Exe_clicked()
{
    QString tmpPath = QFileDialog::getOpenFileName(this, tr("Open Execute File"),
                                                    "",
                                                    tr("Execute file (*.exe)"));
    if( !tmpPath.isEmpty() )
    {
       m_exePath = QDir::toNativeSeparators( tmpPath );
       setExe(true); 
    }
}

void MainWindow::on_toolButton_CopyDir_clicked()
{
    QString tmpCopy = QFileDialog::getExistingDirectory(this, tr("Open Directory Copy"),
                                                        "",
                                                        QFileDialog::ShowDirsOnly |
                                                        QFileDialog::DontResolveSymlinks);
    if( !tmpCopy.isEmpty() )
    {
        ui->lineEdit_Copy->setText( QDir::toNativeSeparators( tmpCopy ) ); 
    }
}

void MainWindow::on_toolButton_Exec_clicked()
{
    if( QFile::exists(m_exePath) )
    {
        qDebug() << tr("Process started.");
        QProcess process;
        process.start( m_exePath );
        Sleep(300);
        m_PID = process.processId();
        setPID(true);
        process.close();
        qDebug() << tr("Process completed.");
    }
    else
        qDebug() << tr("Please select an executable file.");
}

void MainWindow::on_toolButton_Qt_clicked()
{
    QString tmpQt = QDir::toNativeSeparators( QFileDialog::getExistingDirectory(this, tr("Open Directory MinGW+Qt"),
                                                        "",
                                                        QFileDialog::ShowDirsOnly |
                                                        QFileDialog::DontResolveSymlinks) );
    if( !tmpQt.isEmpty() )
    {
        settings->setValue(SETTING_KEY, tmpQt);
        ui->lineEdit_Qt->setText( QDir::toNativeSeparators( tmpQt ) ); 
    }
}

void MainWindow::on_toolButton_Copy_clicked()
{
    qDebug() << tr("Not Implemented!");
    
    /*
    QString copyPath = ui->lineEdit_Copy->text();
    QDir tmpDir( copyPath );
    if( !copyPath.isEmpty() && tmpDir.exists() )
    {
        auto &tree = ui->treeWidget;
        
        int topItemCount = tree->topLevelItemCount();
        for(int i=0; i < topItemCount; i++)
        {
            auto *topItem = tree->topLevelItem(i);
            
            int childItemCount = topItem->childCount();
            for(int i2=0; i2 < childItemCount; i2++)
            {
                if ( topItem->child(i2)->checkState(0) == Qt::Checked)
                {
                    copyFile(topItem->child(i2)->text(0), ui->lineEdit_Copy->text());
                }
            }
        }
    }
    else
        qDebug() << tr("Please select a folder!");
   
    */
}

void MainWindow::on_treeWidget_itemChanged(QTreeWidgetItem *item, int column)
{
    for(int i=0; i < item->childCount(); i++)
    {
        item->child(i)->setCheckState(column, item->checkState(column));
    }
}

void MainWindow::do_toolButton_HWnd_release()
{
    unsetCursor();
    
    clearFields();
    setHWnd( getHWindowFromPoint(QCursor::pos(), m_hWnd) );
    setPID( getPIDFromHWND(m_hWnd, m_PID) );
    setExe( getFilePathFromPID(m_PID, m_exePath) );
}

void MainWindow::on_toolButton_HWnd_pressed()
{
    setCursor(Qt::CrossCursor); 
}

void MainWindow::clearFields()
{
    ui->lineEdit_HWnd->clear();
    ui->lineEdit_PID->clear();
    ui->lineEdit_Exe->clear();
    ui->lineEdit_Copy->clear();
    
    m_hWnd = 0;
    m_PID = 0;
    m_exePath.clear();
}

void MainWindow::setHWnd(bool status)
{
    if( status )
    {
        qDebug() << tr("Handle window is successfully received.");
        ui->lineEdit_HWnd->setText( QString::number(m_hWnd, 16) );
    }
    else
    {
        qWarning() << tr("Function getHWindowFromPoint return false.");
    }
}

void MainWindow::setPID(bool status)
{
    if( status )
    {
        qDebug() << tr("Process ID is successfully received.");
        ui->lineEdit_PID->setText( QString::number(m_PID) );
        updateDependencyTree();
    }
    else
    {
        qWarning() << tr("Function getPIDFromHWND return false.");
    }
}

void MainWindow::setExe(bool status)
{
    if( status )
    {
        qDebug() << tr("File path is successfully received.");
        ui->lineEdit_Exe->setText( m_exePath );
        ui->lineEdit_Copy->setText( QDir::toNativeSeparators( QFileInfo(m_exePath).absolutePath() ) );
    }
    else
    {
        qWarning() << tr("Function getFilePathFromPID return false.");
    }
}

void MainWindow::processSelected(qint32 PID)
{
    clearFields();
    m_PID = PID;
    setPID( true );
    setExe( getFilePathFromPID(m_PID, m_exePath) );
}

void MainWindow::updateDependencyTree()
{   
    QList<QString> tmpModuleList;
    getModulesListFromProcessID(m_PID, tmpModuleList);
    
    ui->treeWidget->clear();
    ui->treeWidget->setColumnCount(1);

    if ( tmpModuleList.count() == 0 )
    {
        qDebug() << tr("List of empty modules.");
        return;
    }
    
    auto makeItem = [](const QString &name, Qt::CheckState state)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setText(0, name);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(0, state);
        
        return item;
    };
    
    QTreeWidgetItem *mainLibrary = makeItem(tr("Main Library"), Qt::Checked);
    QTreeWidgetItem *pluginsLibrary = makeItem(tr("Plugins Library"), Qt::Checked);
    QTreeWidgetItem *systemLibrary = makeItem(tr("System Library"), Qt::Unchecked);
    QTreeWidgetItem *otherLibrary = makeItem(tr("Other Library"), Qt::Unchecked);
    
    const QString MINGW_QT = ui->lineEdit_Qt->text();
    const QString BIN = MINGW_QT + QDir::separator() + "bin";
    const QString PLUGINS = MINGW_QT + QDir::separator() + "plugins";
    const QString SYSTEM = getWinDir();
    
    for(QString &str: tmpModuleList)
    {
        if( isSubPath(BIN , str) )
        {
            QTreeWidgetItem *childMain = makeItem(str, Qt::Checked);
            mainLibrary->addChild(childMain);
        }
        else if( isSubPath(PLUGINS , str) )
        {
            QTreeWidgetItem *childPlugins = makeItem(str, Qt::Checked);
            pluginsLibrary->addChild(childPlugins);
        }
        else if( isSubPath(SYSTEM , str) )
        {
            QTreeWidgetItem *childSystem = makeItem(str, Qt::Unchecked);
            systemLibrary->addChild(childSystem);
        }
        else
        {
            QTreeWidgetItem *childOther = makeItem(str, Qt::Unchecked);
            otherLibrary->addChild(childOther);
        }
    }
    
    ui->treeWidget->addTopLevelItem(mainLibrary);
    ui->treeWidget->addTopLevelItem(pluginsLibrary);
    ui->treeWidget->addTopLevelItem(systemLibrary);
    ui->treeWidget->addTopLevelItem(otherLibrary);
}

void MainWindow::addLog(const QString &str)
{
    ui->listWidget_Log->insertItem(0, str);
}

void MainWindow::on_pushButton_ClearLog_clicked()
{
    ui->listWidget_Log->clear();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    QFileInfo file( QUrl(event->mimeData()->text()).toLocalFile() );

    if( file.suffix().compare("exe", Qt::CaseInsensitive) == 0 )
    {
        event->setAccepted(true);
        event->setDropAction(Qt::LinkAction);
    }
    
    QMainWindow::dragEnterEvent(event);
}

void MainWindow::dropEvent(QDropEvent *event)
{
    if( !event->isAccepted() )
    {
        m_exePath = QUrl(event->mimeData()->text()).toLocalFile();
        setExe( true );
    }
    
    QMainWindow::dropEvent(event);
}
