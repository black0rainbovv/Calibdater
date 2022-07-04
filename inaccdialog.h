#ifndef INACCDIALOG_H
#define INACCDIALOG_H

#include <QDialog>
#include "QSignalMapper"

namespace Ui {
class InaccDialog;
}

class InaccDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InaccDialog(QWidget *parent = 0);
    ~InaccDialog();

private slots:
    void SetTable(QVector<QStringList>);
    void DeleteRow();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::InaccDialog *ui;
    QVector<QStringList> newparams;

signals:
    void sendNewParams(QVector<QStringList>);
};

#endif // INACCDIALOG_H
