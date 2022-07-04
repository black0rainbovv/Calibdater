#include "selectdeviceform.h"
#include "ui_selectdeviceform.h"
#include "mainwindow.h"

selectdeviceform::selectdeviceform(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::selectdeviceform)
{
    ui->setupUi(this);
    this->setWindowTitle("Выбор устройства");
    ReadDeviceList();

    if(ui->deviceListWidget->item(0) != NULL){
        ui->deviceListWidget->setCurrentRow(0);
    }
}

selectdeviceform::~selectdeviceform()
{
    delete ui;
}

void selectdeviceform::ReadDeviceList()
{
    usbD = new CyDev(NULL);
    devicenow = "";
    for(int i = 0; i < 10; i++){
        usbD->Open(i);
        Optic_uC = new CANChannels(usbD, 2);
        DTSN = QString::fromUtf16((ushort*)usbD->SerNum);
        if(DTSN == devicenow){
            if(ui->deviceListWidget->item(i) == NULL){
                continue;
            }
            else
                ui->deviceListWidget->item(i)->font().setStrikeOut(true);

            return;
        }
        ui->deviceListWidget->addItem(DTSN);
        devicenow = DTSN;
    }
}

void selectdeviceform::on_cancelButton_clicked()
{
    this->close();
}

void selectdeviceform::on_connectButton_clicked()
{
    QModelIndex currentIndex= ui->deviceListWidget->currentIndex();
    int index = currentIndex.row();
    usbD = new CyDev(NULL);
    usbD->Open(index);
    emit getIndex(index);
    if(usbD->IsOpen()){
        DTSN = QString::fromUtf16((ushort*)usbD->SerNum);
        bool connection= true;
        emit getNumber(DTSN, connection);
        on_cancelButton_clicked();
    }
    else{
        QMessageBox::critical(this, "Внимание!!", "Устройство не было подключено.");
    }
}
