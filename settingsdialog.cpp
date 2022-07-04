#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "mainwindow.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    QObject::connect(ui->OkButton, SIGNAL(clicked()), this, SLOT(OkClicked()));
    QObject::connect(ui->CancelButton, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->colorwidget, SIGNAL(colorChanged(QColor)), this, SLOT(RecieveColor(QColor)));
    QObject::connect(this, SIGNAL(currentColor(QString)), ui->colorwidget, SLOT(SetCurrentColor(QString)));

    this->setWindowIcon(QIcon(":/Icons/Icons/Settings.png"));

    this->setFixedWidth(295);
    this->setFixedHeight(255);   

    AvailablePortsList();

    SensorsSwitchers();

    Diagnostic();

}

SettingsDialog::~SettingsDialog()
{
    delete ui;
    if(usb->IsOpen()) usb->Close();
    delete Optic_uC;
    delete Temp_uC;
    delete Motor_uC;
    delete Display_uC;
    delete usb;
}

void SettingsDialog::Diagnostic()
{
    usb = new CyDev(NULL);
    usb->Open(0);
    if(!usb->IsOpen())
    {
        ui->label_2->setEnabled(false);
        ui->label_3->setEnabled(false);
        ui->HTimeSpinBox->setEnabled(false);
        ui->MTimeSpinBox->setEnabled(false);
        ui->label_5->setEnabled(false);
        ui->label_6->setEnabled(false);

        ui->OpticLabel->setText("Device not found");
        ui->SerialLabel->setVisible(false);
        ui->MotorLabel->setVisible(false);
        ui->DisplayLabel->setVisible(false);
        ui->TempLabel->setVisible(false);

        Optic_uC = new CANChannels(usb,2);
        Temp_uC = new CANChannels(usb,3);
        Motor_uC = new CANChannels(usb,4);
        Display_uC = new CANChannels(usb,5);
    }
    else
    {
        Optic_uC = new CANChannels(usb,2);
        Temp_uC = new CANChannels(usb,3);
        Motor_uC = new CANChannels(usb,4);
        Display_uC = new CANChannels(usb,5);
        ControllersInfo();
    }
}

void SettingsDialog::ControllersInfo()
{
    ui->SerialLabel->setText(QString::fromUtf16((ushort*)usb->SerNum));
    CANChannels *uC;
    QString outs;
    int res;
    uC = Optic_uC;
    if(!USBCy_RW("fver", outs, usb, uC))
    {
        res = outs.toInt();
                switch(res)
                {
                case -1:    outs = "device don't answer command...(cydev)";     break;
                case -2:    outs = "device don't answer command...(candev: )";
                            break;
                case -3:    outs = "timeout";   break;
                default:    outs = "communication error...";    break;
                }
    }
    ui->OpticLabel->setText(outs);
    uC = Temp_uC;
    if(!USBCy_RW("fver", outs, usb, uC))
    {
        res = outs.toInt();
                switch(res)
                {
                case -1:    outs = "device don't answer command...(cydev)";     break;
                case -2:    outs = "device don't answer command...(candev: )";
                            break;
                case -3:    outs = "timeout";   break;
                default:    outs = "communication error...";    break;
                }
    }
    ui->TempLabel->setText(outs);
    uC = Motor_uC;
    if(!USBCy_RW("fver", outs, usb, uC))
    {
        res = outs.toInt();
                switch(res)
                {
                case -1:    outs = "device don't answer command...(cydev)";     break;
                case -2:    outs = "device don't answer command...(candev: )";
                            break;
                case -3:    outs = "timeout";   break;
                default:    outs = "communication error...";    break;
                }
    }
    ui->MotorLabel->setText(outs);
    uC = Display_uC;
    if(!USBCy_RW("fver", outs, usb, uC))
    {
        res = outs.toInt();
                switch(res)
                {
                case -1:    outs = "device don't answer command...(cydev)";     break;
                case -2:    outs = "device don't answer command...(candev: )";
                            break;
                case -3:    outs = "timeout";   break;
                default:    outs = "communication error...";    break;
                }
    }
    ui->DisplayLabel->setText(outs);
}

void SettingsDialog::SensorsSwitchers()
{
    if(SensorsIsEnabled[0] == 1)
    {
        s1on = true;
        ui->p1->setStyleSheet("background-color : red; border-style: outset;"
                                   "border-width: 2px; border-color: grey;"
                                   "border-radius: 10px; min-width: 2em");
    }
    else
    {
        ui->p1->setStyleSheet("background-color : grey; border-style: outset;"
                                   "border-width: 2px; border-color: grey;"
                                   "border-radius: 10px; min-width: 2em");
        s1on = false;
    }
    if(SensorsIsEnabled[1] == 1)
    {
        s2on = true;
        ui->p2->setStyleSheet("background-color : green; border-style: outset;"
                                   "border-width: 2px; border-color: grey;"
                                   "border-radius: 10px; min-width: 2em");
    }
    else
    {
        s2on = false;
        ui->p2->setStyleSheet("background-color : grey; border-style: outset;"
                                   "border-width: 2px; border-color: grey;"
                                   "border-radius: 10px; min-width: 2em");
    }
    if(SensorsIsEnabled[2] == 1)
    {
        s3on = true;
        ui->p3->setStyleSheet("background-color : blue; border-style: outset;"
                                   "border-width: 2px; border-color: grey;"
                                   "border-radius: 10px; min-width: 2em");
    }
    else
    {
        s3on = false;
        ui->p3->setStyleSheet("background-color : grey; border-style: outset;"
                                   "border-width: 2px; border-color: grey;"
                                   "border-radius: 10px; min-width: 2em");
    }
    if(SensorsIsEnabled[3] == 1)
    {
        s4on = true;
        ui->p4->setStyleSheet("background-color : cyan; border-style: outset;"
                                   "border-width: 2px; border-color: grey;"
                                   "border-radius: 10px; min-width: 2em");
    }
    else
    {
        s4on = false;
        ui->p4->setStyleSheet("background-color : grey; border-style: outset;"
                                   "border-width: 2px; border-color: grey;"
                                   "border-radius: 10px; min-width: 2em");
    }
    if(SensorsIsEnabled[4] == 1)
    {
        s5on = true;
        ui->p5->setStyleSheet("background-color : magenta; border-style: outset;"
                                   "border-width: 2px; border-color: grey;"
                                   "border-radius: 10px; min-width: 2em");
    }
    else
    {
        s5on = false;
        ui->p5->setStyleSheet("background-color : grey; border-style: outset;"
                                   "border-width: 2px; border-color: grey;"
                                   "border-radius: 10px; min-width: 2em");
    }
    if(SensorsIsEnabled[5] == 1)
    {
        s6on = true;
        ui->p6->setStyleSheet("background-color : yellow; border-style: outset;"
                                   "border-width: 2px; border-color: grey;"
                                   "border-radius: 10px; min-width: 2em");
    }
    else
    {
        s6on = false;
        ui->p6->setStyleSheet("background-color : grey; border-style: outset;"
                                   "border-width: 2px; border-color: grey;"
                                   "border-radius: 10px; min-width: 2em");
    }
}

void SettingsDialog::OkClicked()
{
    if (ui->comboBox->currentText() != NULL)
    {
        T8KPortName = ui->comboBox->currentText();
        T8Kconnected = true;
        Interval = ui->IntervalSpinBox->value();
        Filter = ui->FilterSpinBox->value();
        Polka = ui->HTimeSpinBox->value();
        MeasureTime = ui->MTimeSpinBox->value();
        emit setupsettings(graphbackgroundcolor, ui->fontSpinBox->value(), ui->SspinBox->value());
    }
    this->close();
}

void SettingsDialog::SendPortName()
{
    emit SendT8KSettings(ui->comboBox->currentText());
}

void SettingsDialog::AvailablePortsList()
{
    foreach(const QSerialPortInfo &pinfo, QSerialPortInfo::availablePorts())
    {
        ui->comboBox->addItem(pinfo.portName());
    }

    if(ui->comboBox->currentText() != NULL){
        QString current = ui->comboBox->currentText();
        if(current == "COM 1"){
            ui->comboBox->setCurrentIndex(1);
        }
    }
}

void SettingsDialog::on_p1_clicked()
{
    if(s1on)
    {
        ui->p1->setStyleSheet("background-color : grey; border-style: outset;"
                                   "border-width: 2px; border-color: grey;"
                                   "border-radius: 10px; min-width: 2em");
        s1on = false;
        SensorsIsEnabled[0] = 0;
    }
    else
    {
        ui->p1->setStyleSheet("background-color : red; border-style: outset;"
                               "border-width: 2px; border-color: grey;"
                               "border-radius: 10px; min-width: 2em");
        s1on = true;
        SensorsIsEnabled[0] = 1;
    }
}

void SettingsDialog::on_p2_clicked()
{
    if(s2on)
    {
        ui->p2->setStyleSheet("background-color : grey; border-style: outset;"
                                   "border-width: 2px; border-color: grey;"
                                   "border-radius: 10px; min-width: 2em");
        s2on = false;
        SensorsIsEnabled[1] = 0;
    }
    else
    {
        ui->p2->setStyleSheet("background-color : green; border-style: outset;"
                               "border-width: 2px; border-color: grey;"
                               "border-radius: 10px; min-width: 2em");
        s2on = true;
        SensorsIsEnabled[1] = 1;
    }

}

void SettingsDialog::on_p3_clicked()
{
    if(s3on)
    {
        ui->p3->setStyleSheet("background-color : grey; border-style: outset;"
                                   "border-width: 2px; border-color: grey;"
                                   "border-radius: 10px; min-width: 2em");
        s3on = false;
        SensorsIsEnabled[2] = 0;
    }
    else
    {
        ui->p3->setStyleSheet("background-color : blue; border-style: outset;"
                               "border-width: 2px; border-color: grey;"
                               "border-radius: 10px; min-width: 2em");
        s3on = true;
        SensorsIsEnabled[2] = 1;
    }
}

void SettingsDialog::on_p4_clicked()
{
    if(s4on)
    {
        ui->p4->setStyleSheet("background-color : grey; border-style: outset;"
                                   "border-width: 2px; border-color: grey;"
                                   "border-radius: 10px; min-width: 2em");
        s4on = false;
        SensorsIsEnabled[3] = 0;
    }
    else
    {
        ui->p4->setStyleSheet("background-color : cyan; border-style: outset;"
                               "border-width: 2px; border-color: grey;"
                               "border-radius: 10px; min-width: 2em");
        s4on = true;
        SensorsIsEnabled[3] = 1;
    }
}

void SettingsDialog::on_p5_clicked()
{
    if(s5on)
    {
        ui->p5->setStyleSheet("background-color : grey; border-style: outset;"
                                   "border-width: 2px; border-color: grey;"
                                   "border-radius: 10px; min-width: 2em");
        s5on = false;
        SensorsIsEnabled[4] = 0;
    }
    else
    {
        ui->p5->setStyleSheet("background-color : magenta; border-style: outset;"
                               "border-width: 2px; border-color: grey;"
                               "border-radius: 10px; min-width: 2em");
        s5on = true;
        SensorsIsEnabled[4] = 1;
    }
}

void SettingsDialog::on_p6_clicked()
{
    if(s6on)
    {
        ui->p6->setStyleSheet("background-color : grey; border-style: outset;"
                                   "border-width: 2px; border-color: grey;"
                                   "border-radius: 10px; min-width: 2em");
        s6on = false;
        SensorsIsEnabled[5] = 0;
    }
    else
    {
        ui->p6->setStyleSheet("background-color : yellow; border-style: outset;"
                               "border-width: 2px; border-color: grey;"
                               "border-radius: 10px; min-width: 2em");
        s6on = true;
        SensorsIsEnabled[5] = 1;
    }
}

void SettingsDialog::GetSettings(QStringList Custom)
{    
    graphbackgroundcolor = QColor(Custom[0]);
    ui->fontSpinBox->setValue(Custom[1].toInt());
    ui->SspinBox->setValue(Custom[2].toInt());

    ui->IntervalSpinBox->setValue(Interval);
    ui->FilterSpinBox->setValue(Filter);
    ui->HTimeSpinBox->setValue(Polka);
    ui->MTimeSpinBox->setValue(MeasureTime);

    emit currentColor(graphbackgroundcolor.name());
}

void SettingsDialog::RecieveColor(QColor c)
{
    graphbackgroundcolor = c;
}

