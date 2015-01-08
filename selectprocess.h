#ifndef SELECTPROCESS_H
#define SELECTPROCESS_H

//STL

//Native

//Qt
#include <QDialog>

//Project

namespace Ui {
class SelectProcess;
}

class SelectProcess : public QDialog {
    Q_OBJECT

public:
    explicit SelectProcess(QWidget* parent = 0);
    ~SelectProcess();
    qint32 getPID() const;

signals:
    void processSelected(qint32 PID);

private slots:
    void on_pushButton_Update_clicked();
    void on_processList_itemSelectionChanged();
    void on_pushButton_Ok_clicked();

private:
    Ui::SelectProcess* ui;
    qint32 m_PID = -1;

private:
    void updateProcessList();
};

#endif // SELECTPROCESS_H
