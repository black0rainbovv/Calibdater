#include "thermoblockdialog.h"
#include "ui_thermoblockdialog.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "CypressUsb.h"
#include "settingsdialog.h"
#include <QDebug>
#include <QHeaderView>

ThermoblockDialog::ThermoblockDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ThermoblockDialog)
{
    ui->setupUi(this);

    this->setWindowTitle("Термоблок");
    this->setFixedSize(215, 212);
}

ThermoblockDialog::~ThermoblockDialog()
{
    delete ui;
}

void ThermoblockDialog::getNumberType(QString n, QString t)
{
    ui->number_lineEdit->setText(n);
    ui->type_lineEdit->currentText();
}

void ThermoblockDialog::on_cancel_pushButton_clicked()
{
    this->close();
}

void ThermoblockDialog::on_ok_pushButton_clicked()
{
    QString numberofterm = ui->number_lineEdit->text();
    QString typeofterm = ui->type_lineEdit->currentText();
    QString num = "ID " + numberofterm + " 8";
    QString type = "TY " + typeofterm + " 9";

    if(ui->coef_chek->isChecked())
    {
        bool cheked;
        cheked = true;
        emit writeNumberType(num, type, cheked);
    }
    else
    {
        bool cheked;
        cheked = false;
        emit writeNumberType(num, type, cheked);
    }
    this->close();
}
