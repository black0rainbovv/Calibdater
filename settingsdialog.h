#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "global.h"
#include "t8kthread.h"

#include <QDialog>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QSignalMapper>
#include <QAction>
#include <QFile>
#include <QTextStream>

#include "CypressUsb.h"
#include "CANChannels.h"
#include "Utility.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();
public slots:
    void Diagnostic();
    void AvailablePortsList();
    void OkClicked();
    void SendPortName();
    void SensorsSwitchers();
    void ControllersInfo();
    void GetSettings(QStringList Custom);
    void RecieveColor(QColor c);

private slots:
    void on_p1_clicked();

    void on_p2_clicked();

    void on_p3_clicked();

    void on_p4_clicked();

    void on_p5_clicked();

    void on_p6_clicked();

    //void on_OkButton_clicked();

private:
    Ui::SettingsDialog *ui;
    QSignalMapper *signalMapper;
    bool s1on;
    bool s2on;
    bool s3on;
    bool s4on;
    bool s5on;
    bool s6on;
    CyDev *usb;
    CANChannels *Optic_uC;
    CANChannels *Temp_uC;
    CANChannels *Motor_uC;
    CANChannels *Display_uC;
    QColor graphbackgroundcolor;
    QFont tablefont;

signals:
    void SendT8KSettings(QString pName);
    void StartT8KThread();
    void setupsettings(QColor c, int s, int n);
    void currentColor(QString c);
};

#endif // SETTINGSDIALOG_H
