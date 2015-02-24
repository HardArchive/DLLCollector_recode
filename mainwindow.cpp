//STL

//Native

//Qt
#include <QtWidgets>
#include <QProcess>
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QClipboard>

//Project
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "functions.h"
#include "info.h"

//Определение переменной, для вывода сообщений
QTableWidget* MainWindow::m_log;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //!!! Информация о программе !!!
    QApplication::setApplicationName(Info::ApplicationName);
    QApplication::setApplicationVersion(QString("%1.%2").arg(Info::MAJOR).arg(Info::MINOR));
    QApplication::setOrganizationName(Info::OrganizationName);

//!!! Титул приложения !!!

//Разрядность приложения
#ifdef Q_OS_WIN64
    static const QString& processType = trUtf8("64-разрядная версия");
#else
    static const QString& processType = trUtf8("32-разрядная версия");
#endif

//Версия для разработчиков
#ifdef DEV_PROJECT
    static const QString& projectType = tr("Developer Edition");
#else
    static const QString& projectType = tr("User Edition");
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

    //!!! Прицельный выбор окна

    //Свой обработчик событий, для кнопки "прицельного" выбора окна
    ui->toolButton_HWnd->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    m_process.close();
    m_log = nullptr;
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

void MainWindow::saveSettings()
{
    m_settings->clear();

    //Сохранение параметров
    m_settings->setValue(Settings::Profile::SelectedProfil, ui->comboBox_QtProfil->currentIndex());
    m_settings->setValue(Settings::GUI::EnvChecked, ui->envExec->isChecked());
    m_settings->setValue(Settings::GUI::VisibleLog, ui->checkBox_Log->isChecked());
    m_settings->setValue(Settings::GUI::WindowSize, size());

    m_settings->beginGroup(Settings::Profile::QtProfiles);
    const int profCount = ui->comboBox_QtProfil->count();
    for (int i = 0; i < profCount; ++i) {
        const QString& text = ui->comboBox_QtProfil->itemText(i);
        const QVariant& var = ui->comboBox_QtProfil->itemData(i);
        m_settings->setValue(text, var);
    }
    m_settings->endGroup();
}

void MainWindow::loadSettings()
{
    //!!! Загрузка параметров !!!

    //1
    {
        bool tmpBool = true;
        const QVariant& tmpVar = m_settings->value(Settings::GUI::EnvChecked);
        if (!tmpVar.isNull())
            tmpBool = tmpVar.toBool();
        ui->envExec->setChecked(tmpBool);
    }

    //2
    {
        bool tmpBool = true;
        const QVariant& tmpVar = m_settings->value(Settings::GUI::VisibleLog);
        if (!tmpVar.isNull())
            tmpBool = tmpVar.toBool();
        ui->checkBox_Log->setChecked(tmpBool);
    }

    //3
    resize(m_settings->value(Settings::GUI::WindowSize).toSize());

    //4
    m_settings->beginGroup(Settings::Profile::QtProfiles);
    for (const QString& str : m_settings->allKeys()) {
        const QVariant& var = m_settings->value(str);
        ui->comboBox_QtProfil->addItem(str, var);
    }
    m_settings->endGroup();

    //5
    const QVariant& profile = m_settings->value(Settings::Profile::SelectedProfil);
    if (!profile.isNull()) {
        const int indexProfil = profile.toInt();
        ui->comboBox_QtProfil->setCurrentIndex(indexProfil);
        on_comboBox_QtProfil_activated(indexProfil);
    }
}

void MainWindow::setHWnd(qintptr hWnd)
{
    if (hWnd > 0) {
        addLog(trUtf8("Дескриптор окна - задан."));
        m_hWnd = hWnd;
        ui->lineEdit_HWnd->setText(QString::number(m_hWnd, 16));
    } else {
        addLogErr(trUtf8("Дескриптор окна - не задан."));
    }
}

void MainWindow::setPID(qint64 PID)
{
    if (PID > 0) {
        addLog(trUtf8("Идентификатор процесса - задан."));

        m_PID = PID;
        ui->lineEdit_PID->setText(QString::number(PID));

        updateDependencyTree();
    } else {
        addLogErr(trUtf8("Идентификатор процесса - не задан."));
    }
}

void MainWindow::setExe(const QString& path)
{
    if (!path.isEmpty()) {
        addLog(trUtf8("Путь к исполняемому файлу - задан."));

        m_exePath = QDir::toNativeSeparators(path);
        ui->lineEdit_Exe->setText(m_exePath);
        setCopyTo(QFileInfo(m_exePath).absolutePath());
    } else {
        addLogErr(trUtf8("Путь к исполняемому файлу - не задан."));
    }
}

void MainWindow::setCopyTo(const QString& path)
{
    if (!path.isEmpty()) {
        addLog(trUtf8("Путь назначения для копирования библиотек - задан!"));

        m_copyTo = QDir::toNativeSeparators(path);
        ui->lineEdit_CopyTo->setText(m_copyTo);
    } else {
        addLogErr(trUtf8("Путь назначения для копирования библиотек - не задан!"));
    }
}

void MainWindow::setQtLibs(const QString& path)
{
    if (!path.isEmpty()) {
        addLog(trUtf8("Путь к библиотекам Qt - задан."));

        m_QtLibs = QDir::toNativeSeparators(path);
        ui->lineEdit_QtLibs->setText(m_QtLibs);
    } else {
        addLogErr(trUtf8("Путь к библиотекам Qt - не задан."));
    }
}

void MainWindow::setQtPlugins(const QString& path)
{
    if (!path.isEmpty()) {
        addLog(trUtf8("Путь к плагинам Qt - задан."));

        m_QtPlugins = QDir::toNativeSeparators(path);
        ui->lineEdit_QtPlugins->setText(m_QtPlugins);
    } else {
        addLogErr(trUtf8("Путь к плагинам Qt - не задан."));
    }
}

void MainWindow::processSelected(qint64 PID)
{
    clearFields();
    setPID(PID);
    setExe(getFilePathFromPID(PID));
}

void MainWindow::processStarted()
{
    addLog(trUtf8("Процесс запущен!"));

    Sleep(500); //Даём системе время, для обновления информации о процессе

    setPID(m_process.processId());
}

void MainWindow::processFinished(int exitStatus)
{
    Q_UNUSED(exitStatus)

    addLog(trUtf8("Процесс завершён!"));

    m_process.close();

    ui->lineEdit_HWnd->clear();
    ui->lineEdit_PID->clear();
    m_hWnd = 0;
    m_PID = 0;
}

void MainWindow::processError()
{
    addLogErr(trUtf8("Ошибка запуска процесса: ") + m_process.errorString());
}

void MainWindow::updateDependencyTree()
{
    ui->treeWidget->clear();

    if (m_PID <= 0) {
        addLogErr(trUtf8("Сначала выберите процесс или запустите его!"));
        return;
    }

    QList<QString> tmpModuleList;
    getModulesListFromProcessID(m_PID, tmpModuleList);

    if (tmpModuleList.isEmpty()) {
        addLogErr(trUtf8("Не удалось получить список библиотек."));
        return;
    }

    auto makeItem = [](const QString& text, Qt::CheckState state) {
        const int column = 0;
        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setText(column, text);
        item->setToolTip(column, text);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(column, state);
        
        return item;
    };

    QTreeWidgetItem* mainLibrary = makeItem(trUtf8("Основные библиотеки Qt"), Qt::Checked);
    QTreeWidgetItem* pluginsLibrary = makeItem(trUtf8("Плагины Qt"), Qt::Checked);
    QTreeWidgetItem* systemLibrary = makeItem(trUtf8("Системные библиотеки"), Qt::Unchecked);
    QTreeWidgetItem* otherLibrary = makeItem(trUtf8("Остальные библиотеки"), Qt::Unchecked);

    const QString& LIBS = m_QtLibs;
    const QString& PLUGINS = m_QtPlugins;
    const QString& SYSTEM = getWinDir();

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

    addLog(trUtf8("Дерево зависимостей обновлено."));
}

// Реализация выбора элементов в дереве зависимостей
void MainWindow::on_treeWidget_itemChanged(QTreeWidgetItem* item, int column)
{
    // Блокируем сигналы дерева зависимостей (treeWidget),
    // дабы избежать рекурсии при ручном управлении состоянием элементов.
    ui->treeWidget->blockSignals(true);

    // Изменяем состояние всех дочерних элементов
    const int childCount = item->childCount();
    for (int i = 0; i < childCount; i++)
        item->child(i)->setCheckState(column, item->checkState(column));

    // Установка статусов Qt::Checked, Qt::Unchecked, Qt::PartiallyChecked

    QTreeWidgetItem* root = item->parent();
    if (root && root != item) {
        const int childCount = root->childCount();
        int checkedCount = 0;

        // Считаем общее к-во элементов с установленным статусом - Qt::Checked
        for (int i = 0; i < childCount; i++)
            if (root->child(i)->checkState(column) == Qt::Checked)
                ++checkedCount;

        // В зависимости от checkedCount,
        // устанавливаем статус главному элементу.
        if (checkedCount == childCount)
            root->setCheckState(column, Qt::Checked);
        else if (checkedCount == 0)
            root->setCheckState(column, Qt::Unchecked);
        else
            root->setCheckState(column, Qt::PartiallyChecked);
    }

    ui->treeWidget->blockSignals(false);
}

void MainWindow::on_treeWidget_itemDoubleClicked(QTreeWidgetItem* item, int column)
{
    if (item->parent()) {
        const int role = 0;
        const QString& filePath = item->data(column, role).toString();
        QProcess::execute(QString("explorer.exe /select,\"%1\"").arg(filePath));
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
    connect(&sp, SIGNAL(processSelected(qint64)), this, SLOT(processSelected(qint64)));
    sp.show();
    sp.exec();
}

void MainWindow::on_toolButton_Exe_clicked()
{
    const QString& tmpPath = QFileDialog::getOpenFileName(this,
                                                          trUtf8("Выбор исполняемого файла"),
                                                          m_exePath,
                                                          trUtf8("Исполняемый файл (*.exe)"));
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
        addLogErr(trUtf8("Сначала выберите исполняемый файл!"));
    }
}

void MainWindow::on_toolButton_Kill_clicked()
{
    if (m_process.isOpen()) {
        addLog(trUtf8("Завершаем процесс."));
        m_process.terminate();
    } else {
        addLogErr(trUtf8("Сначала запустите приложение!"));
    }
}

void MainWindow::on_toolButton_SelectDirCopyTo_clicked()
{
    const QString& tmpCopy = QFileDialog::getExistingDirectory(this,
                                                               trUtf8("Выбор директории"),
                                                               m_copyTo,
                                                               QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!tmpCopy.isEmpty()) {
        setCopyTo(tmpCopy);
    }
}

void MainWindow::on_toolButton_CopyTo_clicked()
{
    auto copyFile = [this](const QString& filePath, const QString& outDir, ItemTypes types) {
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

    QDir tmpDir(m_copyTo);
    const int column = 0;
    if (!m_copyTo.isEmpty() && tmpDir.exists()) {
        auto tree = ui->treeWidget;

        QTreeWidgetItemIterator it(tree);
        bool copyError = false;
        while (*it) {
            QTreeWidgetItem* item = *it;
            auto topItem = item->parent();

            if (item->checkState(column) == Qt::Checked && topItem) {
                int indexTopItem = tree->indexOfTopLevelItem(topItem);
                const QString& filePath = item->text(column);
                QFileInfo fileInfo(filePath);

                bool copyFileStatus = copyFile(filePath, m_copyTo, ItemTypes(indexTopItem));

                if (copyFileStatus)
                    addLog(trUtf8("Файл \"%1\" скопирован!").arg(fileInfo.fileName()));
                else {
                    copyError = true;
                    addLogErr(trUtf8("Ошибка копирования файла - \"%1\"!").arg(fileInfo.fileName()));
                }
            }
            ++it;
        }
        if (!copyError)
            addLog(trUtf8("Выбранные файлы успешно скопированы!"));
        else
            addLogErr(trUtf8("Ошибка копирования файлов!"));

    } else
        addLogErr(trUtf8("Не указана директория в которую требуется скопировать библиотеки Qt!"));
}

//Выбор профиля
void MainWindow::on_comboBox_QtProfil_activated(int arg1)
{
    const QStringList& list = ui->comboBox_QtProfil->itemData(arg1).toStringList();

    const int minimumStrCount = 2;
    if (list.size() == minimumStrCount) {
        addLog(trUtf8("Выбран профиль: \"%1\"").arg(ui->comboBox_QtProfil->itemText(arg1)));

        setQtLibs(list[0]);
        setQtPlugins(list[1]);

        if (m_PID) {
            updateDependencyTree();
        }
    }
}

void MainWindow::on_toolButton_SaveProfil_clicked()
{
    Qt::WindowFlags flags = windowFlags();
    Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
    flags = flags & (~helpFlag);

    bool selection = false;
    QDir dir(m_QtLibs);
    dir.cdUp();
    const QString nameProfil = QInputDialog::getText(this,
                                                     trUtf8("Название профиля Qt"),
                                                     trUtf8("Ведите название для профиля Qt"),
                                                     QLineEdit::Normal,
                                                     dir.dirName(),
                                                     &selection,
                                                     flags);
    if (selection && !nameProfil.isEmpty()) {

        QVariant tmp(QStringList({ m_QtLibs, m_QtPlugins }));

        if (ui->comboBox_QtProfil->findText(nameProfil, Qt::MatchContains) == -1) {
            ui->comboBox_QtProfil->addItem(nameProfil, tmp);
            addLog(trUtf8("Профиль \"%1\" сохранён!").arg(nameProfil));
        } else {
            addLog(trUtf8("Профиль \"%1\" перезаписан!").arg(nameProfil));
        }
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
    if (reply == QMessageBox::Yes) {
        ui->comboBox_QtProfil->removeItem(currentIdx);
        addLog(trUtf8("Профиль \"%1\" удалён!").arg(currentText));
    }
}

void MainWindow::on_toolButton_QtLibs_clicked()
{
    const QString& tmp = QFileDialog::getExistingDirectory(this,
                                                           trUtf8("Выбор директории с библиотеками Qt"),
                                                           m_QtLibs,
                                                           QFileDialog::ShowDirsOnly);
    if (!tmp.isEmpty()) {
        setQtLibs(tmp);
    }
}

void MainWindow::on_toolButton_QtPlugins_clicked()
{
    const QString& tmp = QFileDialog::getExistingDirectory(this,
                                                           trUtf8("Выбор директории с плагинами Qt"),
                                                           m_QtPlugins,
                                                           QFileDialog::ShowDirsOnly);
    if (!tmp.isEmpty()) {
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
    const QString& qtDir = findPathQt();
    if (!qtDir.isEmpty()) {
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
    QClipboard* clipboard = QApplication::clipboard();

    QStringList list;
    QString tmpStr;
    int iColumnCount = table->columnCount();
    int iRowCount = table->rowCount();
    for (int i = 0; i < iRowCount; ++i) {
        for (int j = 0; j < iColumnCount; ++j) {
            tmpStr += table->item(i, j)->text() + ';';
        }
        list << tmpStr;
        tmpStr.clear();
    }

    clipboard->setText(list.join('\r'));
}

void MainWindow::_addLog(const QString& fun, const QString& mes, TypesMessage type)
{
    if (m_log != nullptr) {
        QColor color;
        if (type == c_general) {
            color = QColor("white");
        } else {
            color = QColor("PeachPuff");
        }

        QTableWidgetItem* function = new QTableWidgetItem(fun);
        function->setToolTip(fun);
        function->setBackgroundColor(color);

        QTableWidgetItem* message = new QTableWidgetItem(mes);
        message->setToolTip(mes);
        message->setBackgroundColor(color);

        int row = m_log->rowCount();
        m_log->setRowCount(row + 1);
        m_log->setItem(row, 0, function);
        m_log->setItem(row, 1, message);

        m_log->scrollToBottom();
    }
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

void MainWindow::closeEvent(QCloseEvent*)
{
    saveSettings();
}
