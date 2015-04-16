//STL

//Native

//Qt
#include <QtWidgets>
#include <QProcess>
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QClipboard>
#include <QDebug>
#include <QInputDialog>

//Project
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mutils.h"
#include "info.h"

//Определение переменной, для вывода сообщений
QTableWidget *MainWindow::m_log;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    //!!! Титул приложения !!!
    //Разрядность приложения
#ifdef Q_OS_WIN64
    static const QString &processType = trUtf8("64-разрядная версия");
#else
    static const QString &processType = trUtf8("32-разрядная версия");
#endif
    
    //Версия для разработчиков
#ifdef DEV_PROJECT
    static const QString &projectType = tr("Developer Edition");
#else
    static const QString &projectType = tr("User Edition");
#endif
    
    setWindowTitle(QString("%1 %2.%3 %4 - %5").arg(Info::ApplicationName).arg(Info::MAJOR).arg(Info::MINOR).arg(projectType).arg(processType));
    
    //!!! Инициализация истории действий !!!
    
    ui->tableWidget_Log->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableWidget_Log->horizontalHeader()->resizeSection(0, 200);
    m_log = ui->tableWidget_Log;
    
    //!!! Настройки программы !!!
    
    //Инициализация настроек
    m_settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, Info::ApplicationName, "settings", this);
    
    //Загрузка параметров
    loadSettings();
    
    //Целевой процесс запущен
    connect(&m_process, SIGNAL(started()), SLOT(processStarted()));
    
    //Целевой процесс завершился
    connect(&m_process, SIGNAL(finished(int)), SLOT(processFinished(int)));
    
    //Ошибка при запуске целевого процесса
    connect(&m_process, SIGNAL(error(QProcess::ProcessError)), SLOT(processError()));
    
    //Инициализация действий для контексного меню дерева зависимостей
    initActionsDependencyTree();
    
    //!!! Прицельный выбор окна
    //Свой обработчик событий, для кнопки "прицельного" выбора окна
    ui->toolButton_HWnd->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    m_log = nullptr;
    delete ui;
}

void MainWindow::clearFields()
{
    ui->lineEdit_HWnd->clear();
    ui->lineEdit_PID->clear();
    ui->lineEdit_Exe->clear();
    ui->lineEdit_CopyTo->clear();
}

void MainWindow::saveSettings()
{
    m_settings->clear();
    
    //Сохранение параметров GUI
    m_settings->setValue(Settings::GUI::EnvChecked, ui->envExec->isChecked());
    m_settings->setValue(Settings::GUI::VisibleLog, ui->checkBox_Log->isChecked());
    m_settings->setValue(Settings::GUI::WindowSize, size());
    
    //Сохранение профилей
    const int profCount = ui->comboBox_QtProfil->count();
    
    if(profCount)
        m_settings->setValue(Settings::Profile::SelectedProfil, ui->comboBox_QtProfil->currentIndex());
        
    m_settings->beginGroup(Settings::Profile::QtProfile);
    for(int i = 0; i < profCount; ++i) {
        const QString profilName = ui->comboBox_QtProfil->itemText(i);
        m_settings->beginGroup(profilName);
        
        m_settings->setValue(Settings::Profile::Keys::Inclusions, m_profiles[profilName].inclusions);
        m_settings->setValue(Settings::Profile::Keys::QtLibs, m_profiles[profilName].qtLibs);
        m_settings->setValue(Settings::Profile::Keys::QtPlugins, m_profiles[profilName].qtPlugins);
        
        m_settings->endGroup();
    }
    m_settings->endGroup();
}

void MainWindow::loadSettings()
{
    //!!! Загрузка параметров !!!
    
    //1
    {
        bool tmpBool = true;
        const QVariant &tmpVar = m_settings->value(Settings::GUI::EnvChecked);
        if(!tmpVar.isNull())
            tmpBool = tmpVar.toBool();
        ui->envExec->setChecked(tmpBool);
    }
    
    //2
    {
        bool tmpBool = true;
        const QVariant tmpVar = m_settings->value(Settings::GUI::VisibleLog);
        if(!tmpVar.isNull())
            tmpBool = tmpVar.toBool();
        ui->checkBox_Log->setChecked(tmpBool);
    }
    
    //3
    resize(m_settings->value(Settings::GUI::WindowSize).toSize());
    
    //4
    m_settings->beginGroup(Settings::Profile::QtProfile);
    {
        for(const QString &group : m_settings->childGroups()) {
            m_settings->beginGroup(group);
            Profile profile;
            
            QVariant tmp = m_settings->value(Settings::Profile::Keys::Inclusions);
            if(tmp.isValid())
                profile.inclusions = tmp.toStringList();
                
            tmp = m_settings->value(Settings::Profile::Keys::QtLibs);
            if(tmp.isValid())
                profile.qtLibs = tmp.toString();
                
            tmp = m_settings->value(Settings::Profile::Keys::QtPlugins);
            if(tmp.isValid())
                profile.qtPlugins = tmp.toString();
                
            ui->comboBox_QtProfil->addItem(group);
            m_profiles.insert(group, profile);
            
            m_settings->endGroup();
        }
    }
    m_settings->endGroup();
    
    //5
    const QVariant &profile = m_settings->value(Settings::Profile::SelectedProfil);
    if(!profile.isNull()) {
        const int indexProfil = profile.toInt();
        on_comboBox_QtProfil_activated(indexProfil);
    }
}

void MainWindow::setHWnd(qintptr hWnd)
{
    if(hWnd > 0) {
        addLog(trUtf8("Дескриптор окна - задан."));
        
        ui->lineEdit_HWnd->setText(QString::number(hWnd, 16));
    } else {
        addLogErr(trUtf8("Дескриптор окна - не задан."));
    }
}

void MainWindow::setPID(uint PID)
{
    if(PID > 0) {
        addLog(trUtf8("Идентификатор процесса - задан."));
        
        ui->lineEdit_PID->setText(QString::number(PID));
        
        updateDependencyTree();
    } else {
        addLogErr(trUtf8("Идентификатор процесса - не задан."));
    }
}

void MainWindow::setExe(const QString &path)
{
    if(!path.isEmpty()) {
        addLog(trUtf8("Путь к исполняемому файлу - задан."));
        
        QString ExePath = QDir::toNativeSeparators(path);
        ui->lineEdit_Exe->setText(ExePath);
        setCopyTo(QFileInfo(ExePath).absolutePath());
    } else {
        addLogErr(trUtf8("Путь к исполняемому файлу - не задан."));
    }
}

void MainWindow::setCopyTo(const QString &path)
{
    if(!path.isEmpty()) {
        addLog(trUtf8("Путь назначения для копирования библиотек - задан!"));
        
        QString CopyTo = QDir::toNativeSeparators(path);
        
        ui->lineEdit_CopyTo->setText(CopyTo);
    } else {
        addLogErr(trUtf8("Путь назначения для копирования библиотек - не задан!"));
    }
}

void MainWindow::setQtLibs(const QString &path)
{
    if(!path.isEmpty()) {
        addLog(trUtf8("Путь к библиотекам Qt - задан."));
        
        QString QtLibs = QDir::toNativeSeparators(path);
        
        ui->lineEdit_QtLibs->setText(QtLibs);
    } else {
        addLogErr(trUtf8("Путь к библиотекам Qt - не задан."));
    }
}

void MainWindow::setQtPlugins(const QString &path)
{
    if(!path.isEmpty()) {
        addLog(trUtf8("Путь к плагинам Qt - задан."));
        
        QString QtPlugins = QDir::toNativeSeparators(path);
        ui->lineEdit_QtPlugins->setText(QtPlugins);
    } else {
        addLogErr(trUtf8("Путь к плагинам Qt - не задан."));
    }
}

void MainWindow::processSelected(uint PID)
{
    clearFields();
    setPID(PID);
    setExe(getFilePathFromPID(PID));
}

void MainWindow::processStarted()
{
    addLog(trUtf8("Процесс запущен!"));
    
    Sleep(500);   //Даём системе время, для обновления информации о процессе
    
    setPID(m_process.processId());
}

void MainWindow::processFinished(int exitStatus)
{
    Q_UNUSED(exitStatus)
    
    addLog(trUtf8("Процесс завершён!"));
    
    m_process.close();
    
    ui->lineEdit_HWnd->clear();
    ui->lineEdit_PID->clear();
}

void MainWindow::processError()
{
    addLogErr(trUtf8("Ошибка запуска процесса: ") + m_process.errorString());
}

void MainWindow::addInclusionFromTree()
{
    QTreeWidget *tree = ui->treeWidget_DependencyTree;
    for(QTreeWidgetItem *item : tree->selectedItems()) {
        if(item && item->parent()) {
            QString path = item->text(0);
            
            if(!m_inclusions.contains(path, Qt::CaseInsensitive)) {
                m_inclusions << path;
            }
        }
    }
}

void MainWindow::deleteInclusionFromTree()
{
    QTreeWidget *tree = ui->treeWidget_DependencyTree;
    for(QTreeWidgetItem *item : tree->selectedItems()) {
        if(item && item->parent()) {
            QString path = item->text(0);
            m_inclusions.removeOne(path);
        }
    }
}

void MainWindow::editorInclusionFromTree()
{
    bool ok;
    QString tmp = QInputDialog::getMultiLineText(
                      this,
                      trUtf8("Редактор включений"),
                      trUtf8("Включения"),
                      m_inclusions.join('\n'),
                      &ok
                  );
    if(ok) {
        m_inclusions = tmp.split('\n');
        m_inclusions.removeDuplicates();
    }
}

void MainWindow::initActionsDependencyTree()
{
    QAction *act1 = new QAction(trUtf8("Добавить во включения"), this);
    connect(act1, SIGNAL(triggered()), this, SLOT(addInclusionFromTree()));
    
    QAction *act2 = new QAction(trUtf8("Убрать из включений"), this);
    connect(act2, SIGNAL(triggered()), this, SLOT(deleteInclusionFromTree()));
    
    QAction *act3 = new QAction(trUtf8("Редактор включений"), this);
    connect(act3, SIGNAL(triggered()), this, SLOT(editorInclusionFromTree()));
    
    m_actions << act1 << act2 << act3;
}

Qt::CheckState MainWindow::isInclusion(const QString &path)
{
    for(const QString &line : m_inclusions) {
        QRegExp rx(line, Qt::CaseInsensitive, QRegExp::WildcardUnix);
        if(rx.exactMatch(path)) {
            return Qt::Checked;
        }
        
    }
    
    return Qt::Unchecked;
}

//Обновляем состояние верхнего элемента
void MainWindow::updateCheckedTopItemTree(QTreeWidgetItem *topItem, int column)
{
    if(topItem && !topItem->parent()) {
        const int childCount = topItem->childCount();
        int checkedCount = 0;
        
        //Считаем общее к-во элементов с установленным статусом - Qt::Checked
        for(int i = 0; i < childCount; i++)
            if(topItem->child(i)->checkState(column) == Qt::Checked)
                ++checkedCount;
                
        //В зависимости от checkedCount,
        //устанавливаем статус главному элементу.
        if(checkedCount == 0)
            topItem->setCheckState(column, Qt::Unchecked);
        else if(checkedCount == childCount)
            topItem->setCheckState(column, Qt::Checked);
        else
            topItem->setCheckState(column, Qt::PartiallyChecked);
    }
}

void MainWindow::updateDependencyTree(bool updatedModules)
{
    ui->treeWidget_DependencyTree->clear();
    
    int PID = ui->lineEdit_PID->text().toUInt();
    
    if(updatedModules) {
        if(PID == 0) {
            addLogErr(trUtf8("Сначала выберите процесс или запустите его!"));
            return;
        }
        
        getModulesListFromProcessID(PID, m_moduleList);
        
        if(m_moduleList.isEmpty()) {
            addLogErr(trUtf8("Не удалось получить список библиотек."));
            return;
        }
    } else if(m_moduleList.isEmpty()) {
        return;
    }
    
    auto makeItem = [](const QString & text, Qt::CheckState state) {
        static const int column = 0;
        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setText(column, text);
        item->setToolTip(column, text);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(column, state);
        
        return item;
    };
    
    QTreeWidgetItem *mainLibrary = makeItem(trUtf8("Основные библиотеки Qt"), Qt::Unchecked);
    QTreeWidgetItem *pluginsLibrary = makeItem(trUtf8("Плагины Qt"), Qt::Unchecked);
    QTreeWidgetItem *systemLibrary = makeItem(trUtf8("Системные библиотеки"), Qt::Unchecked);
    QTreeWidgetItem *otherLibrary = makeItem(trUtf8("Остальные библиотеки"), Qt::Unchecked);
    
    const QString &LIBS = ui->lineEdit_QtLibs->text();
    const QString &PLUGINS = ui->lineEdit_QtPlugins->text();
    const QString &SYSTEM = getWinDir();
    
    for(const QString &path : m_moduleList) {
        if(isSubPath(LIBS, path)) {
            QTreeWidgetItem *childMain = makeItem(path, Qt::Checked);
            mainLibrary->addChild(childMain);
        } else if(isSubPath(PLUGINS, path)) {
            QTreeWidgetItem *childPlugins = makeItem(path, Qt::Checked);
            pluginsLibrary->addChild(childPlugins);
        } else if(isSubPath(SYSTEM, path)) {
            Qt::CheckState state = isInclusion(path);
            QTreeWidgetItem *childSystem = makeItem(path, state);
            systemLibrary->addChild(childSystem);
        } else {
            Qt::CheckState state = isInclusion(path);
            QTreeWidgetItem *childOther = makeItem(path, state);
            otherLibrary->addChild(childOther);
        }
    }
    
    //Обновляем состояния верхних элементов
    updateCheckedTopItemTree(mainLibrary);
    updateCheckedTopItemTree(pluginsLibrary);
    updateCheckedTopItemTree(systemLibrary);
    updateCheckedTopItemTree(otherLibrary);
    
    //Добовляем элементы в дерево
    ui->treeWidget_DependencyTree->addTopLevelItem(mainLibrary);
    ui->treeWidget_DependencyTree->addTopLevelItem(pluginsLibrary);
    ui->treeWidget_DependencyTree->addTopLevelItem(systemLibrary);
    ui->treeWidget_DependencyTree->addTopLevelItem(otherLibrary);
    
    addLog(trUtf8("Дерево зависимостей обновлено."));
}

//Реализация выбора элементов в дереве зависимостей
void MainWindow::on_treeWidget_DependencyTree_itemChanged(QTreeWidgetItem *item, int column)
{
    //Блокируем сигналы дерева зависимостей (treeWidget),
    //дабы избежать рекурсии при ручном управлении состоянием элементов.
    ui->treeWidget_DependencyTree->blockSignals(true);
    
    //Изменяем состояние всех дочерних элементов
    const int childCount = item->childCount();
    for(int i = 0; i < childCount; i++)
        item->child(i)->setCheckState(column, item->checkState(column));
        
    //Установка статусов Qt::Checked, Qt::Unchecked, Qt::PartiallyChecked
    QTreeWidgetItem *topItem = item->parent();
    
    //Обновляем состояние верхнего элемента
    updateCheckedTopItemTree(topItem);
    
    //Убираем блокировку сигналов
    ui->treeWidget_DependencyTree->blockSignals(false);
}

void MainWindow::on_treeWidget_DependencyTree_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if(item->parent()) {
        const int role = 0;
        const QString &filePath = item->data(column, role).toString();
        QProcess::execute(QString("explorer.exe /select,\"%1\"").arg(filePath));
    }
}

void MainWindow::on_treeWidget_DependencyTree_customContextMenuRequested(const QPoint &pos)
{
    QTreeWidget *tree = ui->treeWidget_DependencyTree;
    QTreeWidgetItem *item = tree->currentItem();
    
    QMenu menu;
    
    if(item && item->parent()) {
        menu.addActions(m_actions);
    } else {
        menu.addAction(m_actions.last());
    }
    
    menu.exec(tree->viewport()->mapToGlobal(pos));
}

void MainWindow::do_toolButton_HWnd_release()
{
    unsetCursor();
    
    clearFields();
    setHWnd(getHWindowFromPoint(QCursor::pos()));
    
    quintptr hWnd = ui->lineEdit_HWnd->text().toULongLong(0, 16);
    setPID(getPIDFromHWND(hWnd));
    
    uint PID = ui->lineEdit_PID->text().toUInt();
    setExe(getFilePathFromPID(PID));
}

void MainWindow::on_toolButton_HWnd_pressed()
{
    setCursor(Qt::CrossCursor);
}

void MainWindow::on_toolButton_PID_clicked()
{
    SelectProcess sp(this);
    connect(&sp, SIGNAL(processSelected(uint)), this, SLOT(processSelected(uint)));
    sp.show();
    sp.exec();
}

void MainWindow::on_toolButton_Exe_clicked()
{
    const QString exePath = ui->lineEdit_Exe->text();
    const QString &tmpPath = QFileDialog::getOpenFileName(
                                 this,
                                 trUtf8("Выбор исполняемого файла"),
                                 exePath,
                                 trUtf8("Исполняемый файл (*.exe)"));
    if(!tmpPath.isEmpty()) {
        setExe(QDir::toNativeSeparators(tmpPath));
    }
}

void MainWindow::on_toolButton_Exec_clicked()
{
    const QString exePath = ui->lineEdit_Exe->text();
    const QString QtLibs = ui->lineEdit_QtLibs->text();
    
    if(QFile::exists(exePath)) {
        ui->lineEdit_HWnd->clear();
        
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        
        if(ui->envExec->isChecked()) {
            env.insert("Path", QtLibs);
        }
        
        m_process.close();
        m_process.setProcessEnvironment(env);
        m_process.start("\"" + exePath + "\"");
    } else {
        addLogErr(trUtf8("Сначала выберите исполняемый файл!"));
    }
}

void MainWindow::on_toolButton_Kill_clicked()
{
    if(m_process.isOpen()) {
        addLog(trUtf8("Завершаем процесс."));
        m_process.terminate();
    } else {
        addLogErr(trUtf8("Сначала запустите приложение!"));
    }
}

void MainWindow::on_toolButton_SelectDirCopyTo_clicked()
{
    const QString copyTo = ui->lineEdit_CopyTo->text();
    const QString &tmpCopy = QFileDialog::getExistingDirectory(
                                 this,
                                 trUtf8("Выбор директории"),
                                 copyTo,
                                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(!tmpCopy.isEmpty()) {
        setCopyTo(tmpCopy);
    }
}

void MainWindow::on_toolButton_CopyTo_clicked()
{
    const QString QtPlugins = ui->lineEdit_QtPlugins->text();
    
    auto copyFile = [&QtPlugins](const QString & filePath, const QString & outDir, ItemTypes types) {
        QFile file(filePath);
        QString outFile;
        
        if(types == ItemPlugin) {
            QString tmp = filePath;
            tmp = tmp.remove(0, QtPlugins.size());
            outFile = outDir + tmp;
            QDir().mkpath(QFileInfo(outFile).path());
        } else {
            outFile = outDir + QDir::separator() + QFileInfo(file).fileName();
        }
        
        return file.copy(outFile);
    };
    
    const QString copyTo = ui->lineEdit_CopyTo->text();
    QDir tmpDir(copyTo);
    const int column = 0;
    if(!copyTo.isEmpty() && tmpDir.exists()) {
        auto tree = ui->treeWidget_DependencyTree;
        
        QTreeWidgetItemIterator it(tree);
        bool copyError = false;
        while(*it) {
            QTreeWidgetItem *item = *it;
            
            auto topItem = item->parent();
            
            if(item->checkState(column) == Qt::Checked && topItem) {
                int indexTopItem = tree->indexOfTopLevelItem(topItem);
                const QString &filePath = item->text(column);
                QFileInfo fileInfo(filePath);
                
                bool copyFileStatus = copyFile(filePath, copyTo, ItemTypes(indexTopItem));
                
                if(copyFileStatus)
                    addLog(trUtf8("Файл \"%1\" скопирован!").arg(fileInfo.fileName()));
                else {
                    copyError = true;
                    addLogErr(trUtf8("Ошибка копирования файла - \"%1\"!").arg(fileInfo.fileName()));
                }
            }
            ++it;
        }
        if(!copyError)
            addLog(trUtf8("Выбранные файлы успешно скопированы!"));
        else
            addLogErr(trUtf8("Ошибка копирования файлов!"));
    } else
        addLogErr(trUtf8("Не указана директория в которую требуется скопировать библиотеки Qt!"));
}

void MainWindow::on_comboBox_QtProfil_activated(int idx)
{
    ui->comboBox_QtProfil->setCurrentIndex(idx);
    const QString nameProfil = ui->comboBox_QtProfil->itemText(idx);
    
    addLog(trUtf8("Выбран профиль: \"%1\"").arg(nameProfil));
    
    const QString nameProfile = ui->comboBox_QtProfil->currentText();
    
    m_inclusions = m_profiles[nameProfil].inclusions;
    setQtLibs(m_profiles[nameProfil].qtLibs);
    setQtPlugins(m_profiles[nameProfil].qtPlugins);
    
    updateDependencyTree(false);
}

void MainWindow::on_toolButton_SaveProfil_clicked()
{
    const QString qtLibs = ui->lineEdit_QtLibs->text();
    const QString qtPlugins = ui->lineEdit_QtPlugins->text();
    
    QDir dir(qtLibs);
    dir.cdUp();
    
    bool selection = false;
    Qt::WindowFlags flags = Qt::WindowSystemMenuHint | Qt::WindowTitleHint;
    const QString nameProfil = QInputDialog::getText(
                                   this,
                                   trUtf8("Название профиля Qt"),
                                   trUtf8("Ведите название для профиля Qt"),
                                   QLineEdit::Normal,
                                   dir.dirName(),
                                   &selection,
                                   flags
                               );
    if(selection && !nameProfil.isEmpty()) {
    
        Profile profile;
        profile.inclusions = m_inclusions;
        profile.qtLibs = qtLibs;
        profile.qtPlugins = qtPlugins;
        
        if(!m_profiles.contains(nameProfil)) {
            ui->comboBox_QtProfil->addItem(nameProfil);
            m_profiles.insert(nameProfil, profile);
            
            addLog(trUtf8("Профиль \"%1\" сохранён!").arg(nameProfil));
        } else {
            m_profiles[nameProfil] = profile;
            
            addLog(trUtf8("Профиль \"%1\" перезаписан!").arg(nameProfil));
        }
        
        int idx = ui->comboBox_QtProfil->findText(nameProfil);
        on_comboBox_QtProfil_activated(idx);
    }
}

void MainWindow::on_toolButton_DeleteProfil_clicked()
{
    //Удаляем выбранный профиль
    const int currentIdx = ui->comboBox_QtProfil->currentIndex();
    const QString currentText = ui->comboBox_QtProfil->currentText();
    
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,
                                  trUtf8("Удаление профиля"),
                                  trUtf8("Вы уверены, что хотите удалить профиль \"%1\"?").arg(currentText),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No);
                                  
    if(reply == QMessageBox::Yes) {
        ui->comboBox_QtProfil->removeItem(currentIdx);
        m_profiles.remove(currentText);
        addLog(trUtf8("Профиль \"%1\" удалён!").arg(currentText));
        
        int idx = ui->comboBox_QtProfil->currentIndex();
        on_comboBox_QtProfil_activated(idx);
    }
}

void MainWindow::on_toolButton_QtLibs_clicked()
{
    const QString QtLibs = ui->lineEdit_QtLibs->text();
    const QString &tmp = QFileDialog::getExistingDirectory(this,
                         trUtf8("Выбор директории с библиотеками Qt"),
                         QtLibs,
                         QFileDialog::ShowDirsOnly);
    if(!tmp.isEmpty()) {
        setQtLibs(tmp);
    }
}

void MainWindow::on_toolButton_QtPlugins_clicked()
{
    const QString QtPlugins = ui->lineEdit_QtPlugins->text();
    const QString &tmp = QFileDialog::getExistingDirectory(this,
                         trUtf8("Выбор директории с плагинами Qt"),
                         QtPlugins,
                         QFileDialog::ShowDirsOnly);
    if(!tmp.isEmpty()) {
        setQtPlugins(tmp);
    }
}

void MainWindow::on_checkBox_Log_stateChanged(int arg1)
{
    ui->widget_Log->setVisible(arg1);
}

void MainWindow::on_pushButton_FindQt_clicked()
{
    const QString QT_BIN = "bin";
    const QString QT_PLUGINS = "plugins";
    
    addLog(trUtf8("Ищем Qt..."));
    const QString &qtDir = findPathQt();
    if(!qtDir.isEmpty()) {
        addLog(trUtf8("Qt найден!"));
        setQtLibs(qtDir + QDir::separator() + QT_BIN);
        setQtPlugins(qtDir + QDir::separator() + QT_PLUGINS);
    } else {
        addLogErr(trUtf8("Qt не найден."));
    }
}

void MainWindow::on_pushButton_UpdateTree_clicked()
{
    updateDependencyTree();
}

void MainWindow::on_pushButton_CleanLog_clicked()
{
    auto table = ui->tableWidget_Log;
    table->clearContents();
    table->setRowCount(0);
}

void MainWindow::on_pushButton_CopyLog_clicked()
{
    auto table = ui->tableWidget_Log;
    QClipboard *clipboard = QApplication::clipboard();
    
    QStringList list;
    QString tmpStr;
    int iColumnCount = table->columnCount();
    int iRowCount = table->rowCount();
    for(int i = 0; i < iRowCount; ++i) {
        for(int j = 0; j < iColumnCount; ++j) {
            tmpStr += table->item(i, j)->text() + ';';
        }
        list << tmpStr;
        tmpStr.clear();
    }
    
    clipboard->setText(list.join('\r'));
}

void MainWindow::_addLog(const QString &fun, const QString &mes, TypesMessage type)
{
    if(m_log != nullptr) {
        QColor color;
        if(type == c_general) {
            color = QColor("white");
        } else {
            color = QColor("PeachPuff");
        }
        
        QTableWidgetItem *function = new QTableWidgetItem(fun);
        function->setToolTip(fun);
        function->setBackgroundColor(color);
        
        QTableWidgetItem *message = new QTableWidgetItem(mes);
        message->setToolTip(mes);
        message->setBackgroundColor(color);
        
        int row = m_log->rowCount();
        m_log->setRowCount(row + 1);
        m_log->setItem(row, 0, function);
        m_log->setItem(row, 1, message);
        
        m_log->scrollToBottom();
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == ui->toolButton_HWnd) {
        if(event->type() == QEvent::MouseButtonRelease) {
            do_toolButton_HWnd_release();
        }
    }
    
    return QObject::eventFilter(obj, event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    QFileInfo file(QUrl(event->mimeData()->text()).toLocalFile());
    
    if(file.suffix().compare("exe", Qt::CaseInsensitive) == 0) {
        event->setAccepted(true);
        event->setDropAction(Qt::LinkAction);
    }
    
    QMainWindow::dragEnterEvent(event);
}

void MainWindow::dropEvent(QDropEvent *event)
{
    if(!event->isAccepted()) {
        setExe(QUrl(event->mimeData()->text()).toLocalFile());
    }
    
    QMainWindow::dropEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *)
{
    m_process.close();
    
    //Сохранение параметров
    saveSettings();
}
