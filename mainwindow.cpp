//STL

//Native

//Qt
#include <QtWidgets>
#include <QProcess>
#include <QSettings>
#include <QDebug>
#include <QFileInfo>
#include <QDir>

//Project
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "functions.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    //Титул приложения
    setWindowTitle("DLLCollector_recode 1.1");
    
    //Для работы с параметрами
    m_settings = new QSettings("free", "DLLCollector_recode", this);

    //Загрузка параметров
    loadSettings();

    //Целевой процесс запущен
    connect(&m_process, SIGNAL(started()), SLOT(processStarted()));

    //Целевой процесс завершился
    connect(&m_process, SIGNAL(finished(int)), SLOT(processFinished(int)));

    //Ошибка при запуске целевого процесса
    connect(&m_process, SIGNAL(error(QProcess::ProcessError)), SLOT(processError()));

    //Видимость лога
    ui->widget_Log->setVisible(ui->checkBox_Log->isChecked());

    //Свой обработчик событий
    ui->toolButton_HWnd->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == ui->toolButton_HWnd) {
        if (event->type() == QEvent::MouseButtonRelease) {
            do_toolButton_HWnd_release();
        }
    }

    return QObject::eventFilter(obj, event);
}

void MainWindow::clearFields()
{
    ui->lineEdit_HWnd->clear();
    ui->lineEdit_PID->clear();
    ui->lineEdit_Exe->clear();
    ui->lineEdit_CopyTo->clear();

    m_hWnd = 0;
    m_PID = 0;
    m_exePath.clear();
}

void MainWindow::setHWnd(int hWnd)
{
    if (hWnd > 0) {
        qDebug() << tr("Handle window is successfully received.");

        m_hWnd = hWnd;
        ui->lineEdit_HWnd->setText(QString::number(m_hWnd, 16));
    } else {

        qWarning() << tr("Function getHWindowFromPoint return false.");
    }
}

void MainWindow::setPID(qint64 PID)
{
    if (PID > 0) {
        qDebug() << tr("Process ID is successfully received.");

        m_PID = PID;
        ui->lineEdit_PID->setText(QString::number(PID));

        updateDependencyTree();
    } else {
        qWarning() << tr("Function getPIDFromHWND return false.");
    }
}

void MainWindow::setExe(const QString& str)
{
    if (!str.isEmpty()) {
        qDebug() << tr("File path is successfully received.");
        m_exePath = QDir::toNativeSeparators(str);
        ui->lineEdit_Exe->setText(m_exePath);
        ui->lineEdit_CopyTo->setText(QDir::toNativeSeparators(QFileInfo(m_exePath).absolutePath()));
    } else {
        qWarning() << tr("Function getFilePathFromPID return false.");
    }
}

void MainWindow::setQtLibs(const QString& str)
{
    qDebug() << tr("Qt Libs path is successfully received.");

    m_QtLibs = QDir::toNativeSeparators(str);
    m_settings->setValue(KEY_QTLIBS, m_QtLibs);
    ui->lineEdit_QtLibs->setText(m_QtLibs);
}

void MainWindow::setQtPlugins(const QString& str)
{
    qDebug() << tr("Qt Plugins path is successfully received.");

    m_QtPlugins = QDir::toNativeSeparators(str);
    m_settings->setValue(KEY_QTPLUGINS, m_QtPlugins);
    ui->lineEdit_QtPlugins->setText(m_QtPlugins);
}

void MainWindow::processSelected(int PID)
{
    clearFields();
    setPID(PID);
    setExe(getFilePathFromPID(PID));
}

void MainWindow::processStarted()
{
    qDebug() << tr("Process started.");
    Sleep(500);
    setPID(m_process.processId());
}

void MainWindow::processFinished(int exitStatus)
{
    Q_UNUSED(exitStatus)

    qDebug() << tr("Process finished.");

    ui->lineEdit_HWnd->clear();
    ui->lineEdit_PID->clear();
    m_hWnd = 0;
    m_PID = 0;
}

void MainWindow::processError()
{
    qDebug() << m_process.errorString();
}

void MainWindow::updateDependencyTree()
{

    ui->treeWidget->clear();
    ui->treeWidget->setColumnCount(1);

    if (m_PID > 0) {
        qDebug() << tr("Update dependency tree.");
    } else {
        qDebug() << tr("Please select process or run it.");
        return;
    }

    QList<QString> tmpModuleList;
    getModulesListFromProcessID(m_PID, tmpModuleList);

    if (tmpModuleList.isEmpty()) {
        qDebug() << tr("List of empty modules.");
        return;
    }

    auto makeItem = [](const QString& name, Qt::CheckState state) {
        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setText(0, name);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(0, state);
        return item;
    };

    QTreeWidgetItem* mainLibrary = makeItem(tr("Main Library"), Qt::Checked);
    QTreeWidgetItem* pluginsLibrary = makeItem(tr("Plugins Library"), Qt::Checked);
    QTreeWidgetItem* systemLibrary = makeItem(tr("System Library"), Qt::Unchecked);
    QTreeWidgetItem* otherLibrary = makeItem(tr("Other Library"), Qt::Unchecked);

    const QString LIBS = m_QtLibs;
    const QString PLUGINS = m_QtPlugins;
    const QString SYSTEM = getWinDir();

    for (const QString& str : tmpModuleList) {
        if (isSubPath(LIBS, str)) {
            QTreeWidgetItem* childMain = makeItem(str, Qt::Checked);
            mainLibrary->addChild(childMain);
        } else if (isSubPath(PLUGINS, str)) {
            QTreeWidgetItem* childPlugins = makeItem(str, Qt::Checked);
            pluginsLibrary->addChild(childPlugins);
        } else if (isSubPath(SYSTEM, str)) {
            QTreeWidgetItem* childSystem = makeItem(str, Qt::Unchecked);
            systemLibrary->addChild(childSystem);
        } else {
            QTreeWidgetItem* childOther = makeItem(str, Qt::Unchecked);
            otherLibrary->addChild(childOther);
        }
    }

    ui->treeWidget->addTopLevelItem(mainLibrary);
    ui->treeWidget->addTopLevelItem(pluginsLibrary);
    ui->treeWidget->addTopLevelItem(systemLibrary);
    ui->treeWidget->addTopLevelItem(otherLibrary);
}

void MainWindow::loadSettings()
{
    //Загрузка параметров
    m_QtLibs = m_settings->value(KEY_QTLIBS).toString();
    ui->lineEdit_QtLibs->setText(m_QtLibs);

    m_QtPlugins = m_settings->value(KEY_QTPLUGINS).toString();
    ui->lineEdit_QtPlugins->setText(m_QtPlugins);
}

void MainWindow::on_treeWidget_itemChanged(QTreeWidgetItem* item, int column)
{
    for (int i = 0; i < item->childCount(); i++) {
        item->child(i)->setCheckState(column, item->checkState(column));
    }
}

void MainWindow::do_toolButton_HWnd_release()
{
    unsetCursor();

    clearFields();
    setHWnd(getHWindowFromPoint(QCursor::pos()));
    setPID(getPIDFromHWND(m_hWnd));
    setExe(getFilePathFromPID(m_PID));
}

void MainWindow::on_toolButton_HWnd_pressed()
{
    setCursor(Qt::CrossCursor);
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
    if (!tmpPath.isEmpty()) {
        setExe(QDir::toNativeSeparators(tmpPath));
    }
}

void MainWindow::on_toolButton_Exec_clicked()
{
    if (QFile::exists(m_exePath)) {
        ui->lineEdit_HWnd->clear();
        m_hWnd = 0;

        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

        if (ui->envExec->isChecked()) {
            env.insert("Path", m_QtLibs);
        }

        m_process.close();
        m_process.setProcessEnvironment(env);
        m_process.start("\"" + m_exePath + "\"");

    } else {
        qDebug() << tr("Please select an executable file.");
    }
}

void MainWindow::on_toolButton_Kill_clicked()
{
    m_process.terminate();
}

void MainWindow::on_toolButton_SelectDirCopyTo_clicked()
{
    QString tmpCopy = QFileDialog::getExistingDirectory(this, tr("Open Directory Copy"),
                                                        "",
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!tmpCopy.isEmpty()) {
        ui->lineEdit_CopyTo->setText(QDir::toNativeSeparators(tmpCopy));
    }
}

void MainWindow::on_toolButton_CopyTo_clicked()
{
    auto copyFile = [&](const QString& filePath, const QString& outDir, ItemTypes types) {
        QFile file(filePath);
        QString outFile;

        if (types == ItemPlugins) {
            QString tmp = filePath;
            tmp = tmp.remove(0, m_QtPlugins.size());
            outFile = outDir + tmp;
            QDir().mkpath(QFileInfo(outFile).path());
        } else {
            outFile = outDir + QDir::separator() + QFileInfo(file).fileName();
        }

        return file.copy(outFile);
    };

    QString copyPath = ui->lineEdit_CopyTo->text();
    QDir tmpDir(copyPath);
    if (!copyPath.isEmpty() && tmpDir.exists()) {
        auto tree = ui->treeWidget;

        int topItemCount = tree->topLevelItemCount();
        for (int nTopItem = 0; nTopItem < topItemCount; nTopItem++) {
            auto topItem = tree->topLevelItem(nTopItem);

            int childItemCount = topItem->childCount();
            for (int nChildItem = 0; nChildItem < childItemCount; nChildItem++) {
                auto childItem = topItem->child(nChildItem);

                if (childItem->checkState(0) == Qt::Checked) {
                    copyFile(childItem->text(0), copyPath, ItemTypes(nTopItem));
                }
            }
        }
    } else
        qDebug() << tr("Please select a folder!");
}

void MainWindow::on_toolButton_QtLibs_clicked()
{
    QString tmp = QFileDialog::getExistingDirectory(this, tr("Open Directory Qt Libs"),
                                                    "",
                                                    QFileDialog::ShowDirsOnly);
    if (!tmp.isEmpty()) {
        setQtLibs(tmp);
    }
}

void MainWindow::on_toolButton_QtPlugins_clicked()
{
    QString tmp = QFileDialog::getExistingDirectory(this, tr("Open Directory Qt Plugins"),
                                                    "",
                                                    QFileDialog::ShowDirsOnly);
    if (!tmp.isEmpty()) {
        setQtPlugins(tmp);
    }
}

void MainWindow::on_checkBox_Log_clicked(bool checked)
{
    ui->widget_Log->setVisible(checked);
}

void MainWindow::on_pushButton_FindQt_clicked()
{
    const QString QT_BIN = "bin";
    const QString QT_PLUGINS = "plugins";

    qDebug() << tr("Search Qt...");
    const QString qtDir = findPathQt();
    if (!qtDir.isEmpty()) {
        qDebug() << tr("Qt found!");
        setQtLibs(qtDir + QDir::separator() + QT_BIN);
        setQtPlugins(qtDir + QDir::separator() + QT_PLUGINS);
    } else {
        qDebug() << tr("Qt is not found.");
    }
}

void MainWindow::on_pushButton_UpdateTree_clicked()
{
    updateDependencyTree();
}

void MainWindow::on_pushButton_CleanLog_clicked()
{
    ui->listWidget_Log->clear();
}

void MainWindow::addLog(const QString& str)
{
    ui->listWidget_Log->insertItem(0, str);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    QFileInfo file(QUrl(event->mimeData()->text()).toLocalFile());

    if (file.suffix().compare("exe", Qt::CaseInsensitive) == 0) {
        event->setAccepted(true);
        event->setDropAction(Qt::LinkAction);
    }

    QMainWindow::dragEnterEvent(event);
}

void MainWindow::dropEvent(QDropEvent* event)
{
    if (!event->isAccepted()) {
        setExe(QUrl(event->mimeData()->text()).toLocalFile());
    }

    QMainWindow::dropEvent(event);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    Q_UNUSED(event)

    m_process.close();
}
