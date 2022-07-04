#ifndef SELECTDEVICEFORM_H
#define SELECTDEVICEFORM_H

#include <QDialog>

#include "CypressUsb.h"
#include "CANChannels.h"
#include "Utility.h"

#include "qcustomplot.h"

namespace Ui {
class selectdeviceform;
}

class selectdeviceform : public QDialog
{
    Q_OBJECT

public:
    explicit selectdeviceform(QWidget *parent = 0);
    ~selectdeviceform();

private slots:
    void on_cancelButton_clicked();
    void ReadDeviceList();

    void on_connectButton_clicked();

private:
    Ui::selectdeviceform *ui;

    QString DTSN;
    CyDev *usbD;
    CANChannels *Optic_uC;
    CANChannels *Temp_uC;
    CANChannels *Motor_uC;
    CANChannels *Disp_uC;
    selectdeviceform *deviceForm;
    QString devicenow;
    bool DTConnected;

signals:
    void getNumber(QString DTSN, bool connection);
    void getIndex(int index);
};

#endif // SELECTDEVICEFORM_H
