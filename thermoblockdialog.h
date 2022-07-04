#ifndef THERMOBLOCKDIALOG_H
#define THERMOBLOCKDIALOG_H

#include "global.h"
#include "CypressUsb.h"
#include "CANChannels.h"
#include "Utility.h"

#include <QDialog>

namespace Ui {
class ThermoblockDialog;
}

class ThermoblockDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ThermoblockDialog(QWidget *parent = 0);
    ~ThermoblockDialog();

private slots:
    void getNumberType(QString, QString);

    void on_cancel_pushButton_clicked();

    void on_ok_pushButton_clicked();

private:
    Ui::ThermoblockDialog *ui;
    CyDev *usb;
    CANChannels *Temp_uC;

signals:
    void writeNumberType(QString, QString,bool);
};

#endif // THERMOBLOCKDIALOG_H
