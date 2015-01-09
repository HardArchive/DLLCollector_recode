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
    ui->processList->horizontalHeader()->resizeSection(1, 70);
    
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
        for (ProcessInfo i : list) {
            QTableWidgetItem* name = new QTableWidgetItem(i.name);
            QTableWidgetItem* pid = new QTableWidgetItem(QString::number(i.pid));

            int row = table->rowCount();
            table->setRowCount(row + 1);
            table->setItem(row, 0, name);
            table->setItem(row, 1, pid);
        }
    } else {
        addLog(trUtf8("Function getProcessList return 0."));
    }
}

void SelectProcess::on_pushButton_Update_clicked()
{
    updateProcessList();
}

void SelectProcess::on_processList_itemSelectionChanged()
{
    auto table = ui->processList;
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
        addLog(trUtf8("Please first select process!"));
        QMessageBox::information(this, tr("Information"), trUtf8("Please first select the process!"));
    }
}
