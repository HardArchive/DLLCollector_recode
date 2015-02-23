//STL

//Native

//Qt
#include <QMessageBox>

//Project
#include "selectprocess.h"
#include "ui_selectprocess.h"
#include "functions.h"
#include "debug.h"

SelectProcess::SelectProcess(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SelectProcess)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->processList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    updateProcessList();
}

SelectProcess::~SelectProcess()
{
    delete ui;
}

void SelectProcess::updateProcessList()
{
    QList<ProcessInfo> list;
    auto table = ui->processList;

    table->clearContents();
    table->setRowCount(0);

    if (getProcessList(list)) {
        for (const ProcessInfo& info : list) {
            QTableWidgetItem* name = new QTableWidgetItem(info.name);
            QTableWidgetItem* pid = new QTableWidgetItem(QString::number(info.pid));

            const int row = table->rowCount();
            table->setRowCount(row + 1);
            table->setItem(row, 0, name);
            table->setItem(row, 1, pid);
        }
    } else {
        addLogErr(trUtf8("Не удалось получить список процессов."));
    }
}

void SelectProcess::on_pushButton_Update_clicked()
{
    updateProcessList();
}

void SelectProcess::on_processList_itemSelectionChanged()
{
    const auto table = ui->processList;
    m_PID = table->item(table->currentRow(), 1)->text().toInt();
}

qint64 SelectProcess::getPID() const
{
    return m_PID;
}

void SelectProcess::on_pushButton_Ok_clicked()
{
    if (m_PID > 0) {
        emit processSelected(m_PID);
        close();
    } else {
        addLogErr(trUtf8("Сначала выберите процесс или отмените выбор!"));
        QMessageBox::information(this, trUtf8("Информация"), trUtf8("Сначала выберите процесс или отмените выбор!"));
    }
}
