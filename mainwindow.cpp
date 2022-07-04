#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include"selectdeviceform.h"
#include <QDebug>
#include <QPoint>
#include <QTime>
#include <QHeaderView>

QPoint p1, p2, pc;
const QColor red = Qt::red;
const QColor green = Qt::green;
const QColor blue = Qt::blue;
const QColor cyan = Qt::cyan;
const QColor magenta = Qt::magenta;
const QColor yellow = Qt::yellow;
QList<QColor> scolors;

double realtemp[6];
double old_coefs20[6];

double old_coefs90[6];
double new_coefs20[6];
double new_coefs90[6];
double average20[6];
double average90[6];
double inacc[6][2];
double h_inacc[6][2];
double table_temp20[6][6];
double table_temp90[6][6];
const int grafrange = 600;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowIcon(QIcon(":/Icons/Icons/MainIcon.png"));

    ui->progressBar->setStyleSheet("text-align:center;");
    ui->statusBar->addPermanentWidget(ui->versioLabel, 1);

    scolors << Qt::red << Qt::green << Qt::blue << Qt::cyan << Qt::magenta << Qt::yellow;


    t20 = new QTimer;
    t90 = new QTimer;
    tstatus = new QTimer;
    ina = 0;

    for(int i=0; i<6; i++)
    {
        inacc[i][0] = 0;
        inacc[i][1] = 0;
        for(int j=0; j<6; j++)
        {
            table_temp20[i][j] = 0;
            table_temp90[i][j] = 0;
        }
    }

    td = new ThermoblockDialog;
    QObject::connect(this, SIGNAL(sendNumberType(QString,QString)), td, SLOT(getNumberType(QString,QString)));
    QObject::connect(td, SIGNAL(writeNumberType(QString,QString, bool)), this, SLOT(onWriteNumberType(QString,QString, bool)));

    ui->Tlabel->setStyleSheet("border: 1px solid black");
    ui->hlistButton->setVisible(false);
    ui->pushButton_2->setVisible(false);

    rubberBand = 0;

    m3pressed = false;

    measures = 0;

    isScaled = false;

    measureRealTemp = false;

    stage2ison = false;

    warmingup = false;

    grafison = false;

    isWorking = false;

    measurefinished = false;

    ui->actionStop->setEnabled(false);

    ui->writeHistoryButton->setEnabled(false);
    ui->clearHistoryButton->setEnabled(false);

    SensorsIsEnabled[0] = 1;
    SensorsIsEnabled[1] = 1;
    SensorsIsEnabled[2] = 1;
    SensorsIsEnabled[3] = 1;
    SensorsIsEnabled[4] = 1;
    SensorsIsEnabled[5] = 1;

    ReadSettings();

    SetTempTable();
    SetCoefTable();
    SetPlot();
    SetMiniPlot();    

    OnPositionChanged(1);

    on_actionConnect_triggered();

    QObject::connect(ui->positionWidget, SIGNAL(positionChanged(int)), this, SLOT(OnPositionChanged(int)));

    QObject::connect(t20, SIGNAL(timeout()), this, SLOT(Calib20C()));
    QObject::connect(t90, SIGNAL(timeout()), this, SLOT(Calib90C()));
    QObject::connect(tstatus, SIGNAL(timeout()), this, SLOT(DTStatusCheck()));

    QObject::connect(ui->actionSettings, SIGNAL(triggered(bool)), this, SLOT(OpenSettingsDialog()));
    QObject::connect(ui->actionStart, SIGNAL(triggered(bool)), this, SLOT(StartDTCalib()));
    QObject::connect(ui->actionGraph, SIGNAL(triggered(bool)), this, SLOT(ClearGraph()));
    QObject::connect(ui->actionStop, SIGNAL(triggered(bool)), this, SLOT(StopDTCalib()));

    QObject::connect(ui->actionSave, SIGNAL(triggered(bool)), this, SLOT(SaveGraphToFile()));
    QObject::connect(ui->actionLoad, SIGNAL(triggered(bool)), this, SLOT(LoadGraphFromFile()));

    QObject::connect(ui->actionSaveExcel, SIGNAL(triggered(bool)), this, SLOT(ExportToExcel()));
    QObject::connect(ui->actionLoadExcel, SIGNAL(triggered(bool)), this, SLOT(ImportFromExcel()));

    QObject::connect(ui->plotwidget, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress(QMouseEvent*)));
    QObject::connect(ui->plotwidget, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMove(QMouseEvent*)));
    QObject::connect(ui->plotwidget, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(mouseRelease(QMouseEvent*)));

    QObject::connect(ui->checkBoxCurrent, SIGNAL(clicked(bool)), this, SLOT(UseCurrentSensorsInaccuracy()));
    QObject::connect(ui->checkBoxHistory, SIGNAL(clicked(bool)), this, SLOT(UseHistorySensorsInaccuracy()));

    QObject::connect(ui->TempTable, SIGNAL(cellClicked(int,int)), this, SLOT(DisableCell(int, int)));
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::DTStatusCheck()
{
    if(DTconnected)
    {
        QStringList sl;
        QString outs;
        Temp_uC = new CANChannels(usb, 3);
        USBCy_RW("XGS", outs, usb, Temp_uC);
        sl = outs.split(' ');
        outs = sl[0];
        if(outs == "4")
        {
            isWorking = false;
            ui->actionSettings->setEnabled(false);
            ui->actionStart->setEnabled(false);
            ui->actionGraph->setEnabled(false);
            ui->actionStop->setEnabled(true);
            ui->positionWidget->setEnabled(false);
            ui->checkBoxCurrent->setEnabled(false);
            ui->checkBoxHistory->setEnabled(false);
            warmingup = true;
        }
        if(outs == "1")
        {
            isWorking = true;
            if(warmingup == true)
            {
                warmingup = false;
            }           
            ui->actionSettings->setEnabled(false);
            ui->actionStart->setEnabled(false);
            ui->actionGraph->setEnabled(false);
            ui->actionStop->setEnabled(true);
            ui->positionWidget->setEnabled(false);
            ui->checkBoxCurrent->setEnabled(false);
            ui->checkBoxHistory->setEnabled(false);

        }
        if(outs == "0")
        {
            isWorking = false;
            ui->actionStop->setEnabled(false);
            ui->actionGraph->setEnabled(true);
            ui->actionSettings->setEnabled(true);
            ui->actionStart->setEnabled(true);
            ui->positionWidget->setEnabled(true);
            ui->checkBoxCurrent->setEnabled(true);
            ui->checkBoxHistory->setEnabled(true);
        }
    }
}

void MainWindow::FindT8K()
{
    foreach(const QSerialPortInfo &pinfo, QSerialPortInfo::availablePorts())
    {
        if(pinfo.hasProductIdentifier())
        {
            if((QByteArray::number(pinfo.productIdentifier())) == "24577")
            {
                T8KPortName = pinfo.portName();
                T8Kconnected = true;
                ui->actionLoad->setEnabled(false);
                ClearGraph();
                T8KThread *p = new T8KThread;
                QObject::connect(p, SIGNAL(SendTemperature(QStringList, double)), SLOT(ReceiveTemperature(QStringList, double)));
                QObject::connect(p, SIGNAL(PortError(QString)), SLOT(SerialPortErrorHandler(QString)));
                QObject::connect(p, SIGNAL(finished()), p, SLOT(deleteLater()));
                p->start();                
            }
        }
    }
    if(T8KPortName == "")
        QMessageBox::information(this, tr("Внимание!"), tr("T8K не подключен"));
}

void MainWindow::Calib20C()
{
    if(!warmingup)
    {
        QTime lefttime;
        int it, min, sec;
        QString outs;

        USBCy_RW("TI", outs, usb, Temp_uC);
        it = outs.split(' ')[0].toInt();
        ui->progressBar->setValue(ui->progressBar->maximum()-(60+(Polka+MeasureTime))-it);
        this->setWindowTitle("["+QString::number(100*ui->progressBar->value()/ui->progressBar->maximum())+
                             "%]"+"Calibdater-"+DTSN);
        int to20time = it-Polka-MeasureTime;
        int hold20time = it-MeasureTime;
        if(to20time <= 0)
        {
            if(hold20time <= 0)
            {
                if(it <= 0)
                {
                    measureRealTemp = false;
                    AddLine(measureendpoint);
                    AddPlotTextMark((measureendpoint+measurestartpoint)/2,30);
                    t20->stop();
                    WriteRealTemp();
                    StartDTCalibStage2();
                }
                else
                {
                    measureRealTemp = true;
                    sec = (it)%60;
                    min = (it-sec)/60;
                    lefttime.setHMS(0, min, sec);
                    ui->phaselabel->setText("   Измерение 20°C : "+lefttime.toString("mm:ss"));
                }
            }
            else
            {
                sec = (hold20time)%60;
                min =(hold20time-sec)/60;
                lefttime.setHMS(0, min, sec);
                ui->phaselabel->setText("   Выдержка 20°C : "+lefttime.toString("mm:ss"));
            }
        }
        else
        {
            sec = (to20time)%60;
            min = (to20time-sec)/60;
            lefttime.setHMS(0, min, sec);
            ui->phaselabel->setText("   Установка 20°C : "+lefttime.toString("mm:ss"));
        }
    }
    else
    {
        ui->phaselabel->setText("   Прогрев!");
    }
}

void MainWindow::Calib90C()
{
    QTime lefttime;
    int it, min, sec;
    QString outs;

    USBCy_RW("TI", outs, usb, Temp_uC);
    it = outs.split(' ')[0].toInt();
    ui->progressBar->setValue(ui->progressBar->maximum()-it);
    this->setWindowTitle("["+QString::number(100*ui->progressBar->value()/ui->progressBar->maximum())+
                         "%]"+"Calibdater-"+DTSN);
    int to90time = it-Polka-MeasureTime;
    int hold90time = it-MeasureTime;
    if(to90time <= 0)
    {
        if(hold90time <= 0)
        {
            if(it <= 0)
            {
                measureRealTemp = false;
                AddLine(measureendpoint);
                AddPlotTextMark((measureendpoint+measurestartpoint)/2,75);
                t90->stop();
                WriteRealTemp();
                StopDTCalib();
                GetCoefs();
                CalcAverageTemp();
                CalcNewCoefs();
                CalcCurrentSensorsInacuracy();
                PlayBell();
            }
            else
            {
                measureRealTemp = true;
                sec = (it)%60;
                min = (it-sec)/60;
                lefttime.setHMS(0, min, sec);
                ui->phaselabel->setText("   Измерение 90°C : "+lefttime.toString("mm:ss"));
            }
        }
        else
        {
            sec = (hold90time)%60;
            min =(hold90time-sec)/60;
            lefttime.setHMS(0, min, sec);
            ui->phaselabel->setText("   Выдержка 90°C : "+lefttime.toString("mm:ss"));
        }
    }
    else
    {
        sec = (to90time)%60;
        min = (to90time-sec)/60;
        lefttime.setHMS(0, min, sec);
        ui->phaselabel->setText("   Установка 90°C : "+lefttime.toString("mm:ss"));
    }
}

void MainWindow::PlayBell()
{
    QMediaPlayer *player = new QMediaPlayer(this);
    player->setMedia(QUrl("qrc:/Icons/Icons/bell.mp3"));
    player->play();
}

void MainWindow::StartDTCalib()
{
    if(T8Kconnected)
    {
        if(DTconnected)
        {
            if(ui->TempTable->cellWidget(3 ,2*((position-1)%6)+1) != NULL)
            {
                QLabel *l = qobject_cast<QLabel*>(ui->TempTable->cellWidget(3, 2*((position-1)%6)+1));
                if(l->text() != "")
                {
                    QMessageBox m(QMessageBox::Question,
                                    "Внимание!",
                                    "Данные для этой позиции уже измерены. Провести измерения повторно?",
                                    QMessageBox::Yes|QMessageBox::No);
                    m.setButtonText(QMessageBox::Yes, "Да");
                    m.setButtonText(QMessageBox::No, "Нет");
                    if(m.exec() == QMessageBox::Yes)
                    {
                        StartDTCalibStage1();
                    }
                }
                else
                    StartDTCalibStage1();
            }
            else
            {
                StartDTCalibStage1();
            }
        }
    }
    else
    {
        QMessageBox::information(this, tr("Внимание!"), tr("Устройство T8K не подключено"));
    }
}

void MainWindow::StartDTCalibStage1()
{
    measurefinished = false;
    QString outs;
    ui->label_2->setText("Текущая операция : ");
    Temp_uC = new CANChannels(usb, 3);
    USBCy_RW("XPRG 0 35 0", outs, usb, Temp_uC);
    USBCy_RW("XLEV 2000 "+QString::number(Polka+MeasureTime)+" 0 0 0 0" , outs, usb, Temp_uC);
    USBCy_RW("XCYC 1", outs, usb, Temp_uC);
    USBCy_RW("XSAV Calib20", outs, usb, Temp_uC);
    USBCy_RW("RN", outs, usb, Temp_uC);
    QThread::msleep(100);
    USBCy_RW("TI", outs, usb, Temp_uC);
    ui->progressBar->setMaximum(outs.split(' ')[0].toInt()+60+(Polka+MeasureTime));
    t20->start(1000);
    ui->positionWidget->setEnabled(false);
}

void MainWindow::StartDTCalibStage2()
{
    measurefinished = false;
    for(int i=0; i<6; i++)
    {
        realtemp[i] = 0.0;
    }
    measures = 0;
    QString outs;
    Temp_uC = new CANChannels(usb, 3);
    USBCy_RW("ST", outs, usb, Temp_uC);
    USBCy_RW("XPRG 0 35 0", outs, usb, Temp_uC);
    USBCy_RW("XLEV 9000 "+QString::number(Polka+MeasureTime)+" 0 0 0 0" , outs, usb, Temp_uC);
    USBCy_RW("XCYC 1", outs, usb, Temp_uC);
    USBCy_RW("XSAV Calib90", outs, usb, Temp_uC);
    USBCy_RW("RN", outs, usb, Temp_uC);
    QThread::msleep(100);
    USBCy_RW("TI", outs, usb, Temp_uC);
    ui->progressBar->setMaximum(ui->progressBar->value()+outs.split(' ')[0].toInt());
    stage2ison = true;
    t90->start(1000);
}

void MainWindow::StopDTCalib()
{
    isWorking = false;
    if(!measurefinished)
    {
        for(int i=0; i<6; i++)
        {
            QLabel *l1 = new QLabel();
            QLabel *l2 = new QLabel();
            l1->setText("");
            l2->setText("");
            ui->TempTable->setCellWidget(i+3, 2*((position-1+i)%6)+1, l1);
            ui->TempTable->setCellWidget(i+3, 2*((position-1+i)%6)+2, l2);
        }
        measurefinished = true;
    }
    ui->label_2->setText("");
    Temp_uC = new CANChannels(usb, 3);
    QString outs;
    USBCy_RW("ST", outs, usb, Temp_uC);
    if(t20->isActive()) t20->stop();
    if(t90->isActive()) t90->stop();
    QThread::msleep(1000);
    for(int i=0; i<6; i++)
    {
        realtemp[i] = 0.0;
    }
    measures = 0;
    isScaled = false;
    measureRealTemp = false;
    measurestartpoint = 0;
    measureendpoint = 0;
    warmingup = false;
    stage2ison = false;
    ui->positionWidget->setEnabled(true);
    ui->progressBar->setValue(0);
    ui->phaselabel->setText("");
    this->setWindowTitle("Calibdater-"+DTSN);
}

void MainWindow::on_actionConnect_triggered()
{
    deviceForm = new selectdeviceform();
    deviceForm->show();

    QObject::connect(deviceForm,SIGNAL(getIndex(int)),this,SLOT(setIndex(int)));
}

void MainWindow::setIndex(int index)
{
    usb = new CyDev(NULL);
    if(usb->IsOpen()){
        QMessageBox::critical(this,"Внимание!","Устройство уде подключено.");
        return;
    }
    else{
        usb->Open(index);
        if(usb->IsOpen()){
            QString ttype;
            DTSN = QString::fromUtf16((ushort*)usb->SerNum);
            this->setWindowTitle("Calibdater-"+DTSN);
            DTconnected = true;
            QStringList sl;
            QString outs;
            Temp_uC = new CANChannels(usb, 3);
            USBCy_RW("HRTY", outs, usb, Temp_uC);
            TermNumber = outs;
            QThread::msleep(100);
            USBCy_RW("HRID", outs, usb, Temp_uC);
            ttype = outs;
            QThread::msleep(100);
            ui->coeflabel->setText("Коэффициенты Термоблока №"+TermNumber+" (Тип - "+ttype+")");
            USBCy_RW("XGS", outs, usb, Temp_uC);
            sl = outs.split(' ');
            outs = sl[0];
            if(outs == "1")
            {
                if(!grafison)
                {
                    FindT8K();
                }
            }
            QFile file("autosave.csv");
            if(file.exists())
            {
                QMessageBox m(QMessageBox::Question,
                                "Внимание!",
                                "Программа была завершена с ошибкой. Загрузить сохраненные данные?",
                                QMessageBox::Yes|QMessageBox::No);
                m.setButtonText(QMessageBox::Yes, "Да");
                m.setButtonText(QMessageBox::No, "Нет");
                if(m.exec() == QMessageBox::Yes)
                {
                    LoadAutoSavedTable();
                }
            }
        }
        else
            QMessageBox::critical(this,"Внимание!","Не удалось подключить устройство!");
    }
    if(!tstatus->isActive())tstatus->start(500);
}

void MainWindow::on_actionDisconnect_triggered()
{
    if(DTconnected == true){
            if(usb->IsOpen()){
                usb->Close();
                DTconnected = false;
                 this->setWindowTitle("Calibdater - Not Connected");
                   QFile file("autosave.csv");
                   file.remove();
            }
            else
                QMessageBox::critical(this,"Внимание!","USB не был открыт.");
        }
        else
            QMessageBox::critical(this,"Внимание!","Прибор не был подключен.");
}

void MainWindow::ReadSettings()
{
    int x, y, w, h;
    QFile file("settings.xml");
    if(file.open(QIODevice::ReadOnly))
    {
        QXmlStreamReader sr(&file);
        while(!sr.atEnd())
        {
            if(sr.name() == "Settings")
            {
                while(!sr.atEnd())
                {
                    if(sr.isEndElement())
                    {
                        sr.readNext();
                        break;
                    }
                    if(sr.name() == "MainWindow")
                    {
                        while(!sr.atEnd())
                        {
                            if(sr.isEndElement())
                            {
                                sr.readNext();
                                break;
                            }
                            if(sr.isStartElement())
                            {
                                if(sr.name() == "x")
                                {
                                    while(!sr.atEnd())
                                    {
                                        if(sr.isEndElement())
                                        {
                                            sr.readNext();
                                            break;
                                        }
                                        if(sr.isStartElement())
                                        {
                                            x = sr.readElementText().toInt();
                                            sr.readNext();
                                            break;
                                        }
                                        else
                                        {
                                            sr.readNext();
                                        }
                                    }
                                }
                                if(sr.name() == "y")
                                {
                                    while(!sr.atEnd())
                                    {
                                        if(sr.isEndElement())
                                        {
                                            sr.readNext();
                                            break;
                                        }
                                        if(sr.isStartElement())
                                        {
                                            y = sr.readElementText().toInt();
                                            sr.readNext();
                                            break;
                                        }
                                        else
                                        {
                                            sr.readNext();
                                        }
                                    }
                                }
                                if(sr.name() == "w")
                                {
                                    while(!sr.atEnd())
                                    {
                                        if(sr.isEndElement())
                                        {
                                            sr.readNext();
                                            break;
                                        }
                                        if(sr.isStartElement())
                                        {
                                            w = sr.readElementText().toInt();
                                            sr.readNext();
                                            break;
                                        }
                                        else
                                        {
                                            sr.readNext();
                                        }
                                    }
                                }
                                if(sr.name() == "h")
                                {
                                    while(!sr.atEnd())
                                    {
                                        if(sr.isEndElement())
                                        {
                                            sr.readNext();
                                            break;
                                        }
                                        if(sr.isStartElement())
                                        {
                                            h = sr.readElementText().toInt();
                                            sr.readNext();
                                            break;
                                        }
                                        else
                                        {
                                            sr.readNext();
                                        }
                                    }
                                }
                                else
                                    sr.readNext();
                            }
                            else
                                sr.readNext();
                        }
                    }
                    if(sr.name() == "Custom")
                    {
                        while(!sr.atEnd())
                        {
                            if(sr.isEndElement())
                            {
                                sr.readNext();
                                break;
                            }
                            if(sr.isStartElement())
                            {
                                if(sr.name() == "background")
                                {
                                    while(!sr.atEnd())
                                    {
                                        if(sr.isEndElement())
                                        {
                                            sr.readNext();
                                            break;
                                        }
                                        if(sr.isStartElement())
                                        {
                                            graphbackground = QColor(sr.readElementText());
                                            sr.readNext();
                                            break;
                                        }
                                        else
                                        {
                                            sr.readNext();
                                        }
                                    }
                                }
                                if(sr.name() == "fontsize")
                                {
                                    while(!sr.atEnd())
                                    {
                                        if(sr.isEndElement())
                                        {
                                            sr.readNext();
                                            break;
                                        }
                                        if(sr.isStartElement())
                                        {
                                            fontsize = sr.readElementText().toInt();
                                            sr.readNext();
                                            break;
                                        }
                                        else
                                        {
                                            sr.readNext();
                                        }
                                    }
                                }
                                if(sr.name() == "snumber")
                                {
                                    while(!sr.atEnd())
                                    {
                                        if(sr.isEndElement())
                                        {
                                            sr.readNext();
                                            break;
                                        }
                                        if(sr.isStartElement())
                                        {
                                            sensorgraphnumber = sr.readElementText().toInt()-1;
                                            sr.readNext();
                                            break;
                                        }
                                        else
                                        {
                                            sr.readNext();
                                        }
                                    }
                                }
                                else
                                    sr.readNext();
                            }
                            else
                                sr.readNext();
                        }
                    }
                    if(sr.name() == "Calib")
                    {
                        while(!sr.atEnd())
                        {
                            if(sr.isEndElement())
                            {
                                sr.readNext();
                                break;
                            }
                            if(sr.isStartElement())
                            {
                                if(sr.name() == "interval")
                                {
                                    while(!sr.atEnd())
                                    {
                                        if(sr.isEndElement())
                                        {
                                            sr.readNext();
                                            break;
                                        }
                                        if(sr.isStartElement())
                                        {
                                            Interval = sr.readElementText().toDouble();
                                            sr.readNext();
                                            break;
                                        }
                                        else
                                        {
                                            sr.readNext();
                                        }
                                    }
                                }
                                if(sr.name() == "filter")
                                {
                                    while(!sr.atEnd())
                                    {
                                        if(sr.isEndElement())
                                        {
                                            sr.readNext();
                                            break;
                                        }
                                        if(sr.isStartElement())
                                        {
                                            Filter = sr.readElementText().toInt();
                                            sr.readNext();
                                            break;
                                        }
                                        else
                                        {
                                            sr.readNext();
                                        }
                                    }
                                }
                                if(sr.name() == "polka")
                                {
                                    while(!sr.atEnd())
                                    {
                                        if(sr.isEndElement())
                                        {
                                            sr.readNext();
                                            break;
                                        }
                                        if(sr.isStartElement())
                                        {
                                            Polka = sr.readElementText().toInt();
                                            sr.readNext();
                                            break;
                                        }
                                        else
                                        {
                                            sr.readNext();
                                        }
                                    }
                                }
                                if(sr.name() == "measure")
                                {
                                    while(!sr.atEnd())
                                    {
                                        if(sr.isEndElement())
                                        {
                                            sr.readNext();
                                            break;
                                        }
                                        if(sr.isStartElement())
                                        {
                                            MeasureTime = sr.readElementText().toInt();
                                            sr.readNext();
                                            break;
                                        }
                                        else
                                        {
                                            sr.readNext();
                                        }
                                    }
                                }
                                else
                                    sr.readNext();
                            }
                            else
                                sr.readNext();
                        }
                    }
                    else
                        sr.readNext();
                }
            }
            else
                sr.readNext();
        }
        file.close();
        if(sr.hasError())
        {
            qDebug() << "Error: " << sr.errorString();
        }
    }
    if(x < 0)
        x = 0;
    if(y < 0)
        y = 0;
    this->move(x, y);
    this->resize(w, h);
}

void MainWindow::SaveSettings()
{
    QFile file("settings.xml");
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QXmlStreamWriter sw(&file);
        sw.setAutoFormatting(true);
        sw.writeStartDocument();
        sw.writeStartElement("Settings");
            sw.writeStartElement("MainWindow");
                sw.writeTextElement("x", QString::number(this->x()));
                sw.writeTextElement("y", QString::number(this->y()));
                sw.writeTextElement("w", QString::number(this->width()));
                sw.writeTextElement("h", QString::number(this->height()));
            sw.writeEndElement();
            sw.writeStartElement("Custom");
                sw.writeTextElement("background", graphbackground.name());
                sw.writeTextElement("fontsize", QString::number(fontsize));
                sw.writeTextElement("snumber", QString::number(sensorgraphnumber+1));
            sw.writeEndElement();
            sw.writeStartElement("Calib");
                sw.writeTextElement("interval", QString::number(Interval, 'f', 1));
                sw.writeTextElement("filter", QString::number(Filter));
                sw.writeTextElement("polka", QString::number(Polka));
                sw.writeTextElement("measure", QString::number(MeasureTime));
            sw.writeEndElement();
        sw.writeEndElement();
        sw.writeEndDocument();
        file.close();
    }
}

void MainWindow::SetTempTable()
{
    ui->TempTable->setColumnCount(13);
    ui->TempTable->setRowCount(10);
    ui->TempTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->TempTable->setSelectionMode(QAbstractItemView::NoSelection);
    ui->TempTable->setStyleSheet("border: 1px light gray");
    ui->TempTable->setSpan(0, 1, 1, 12);
    ui->TempTable->setSpan(0, 0, 3, 1);
    for(int i=1; i<13; i+=2)
    {
        ui->TempTable->setSpan(1, i, 1, 2);
    }

    QTableWidgetItem *qt = new QTableWidgetItem();
    qt->setText("Позиция датчика");
    qt->setTextAlignment(Qt::AlignCenter);
    qt->setBackground(Qt::gray);
    qt->setFont(QFont("MS Shell", 12));
    ui->TempTable->setItem(0, 1, qt);
    QHeaderView *hheaderView = new QHeaderView(Qt::Horizontal, ui->TempTable);
    QHeaderView *vheaderView = new QHeaderView(Qt::Vertical, ui->TempTable);
    ui->TempTable->setVerticalHeader(vheaderView);
    ui->TempTable->setHorizontalHeader(hheaderView);
    ui->TempTable->horizontalHeader()->hide();
    ui->TempTable->verticalHeader()->hide();

    for(int i=3; i<10; i++)
    {
        vheaderView->setSectionResizeMode(i, QHeaderView::Stretch);
    }
    for(int i=1; i<13; i++)
    {
        hheaderView->setSectionResizeMode(i, QHeaderView::Stretch);
    }
    vheaderView->resizeSection(0, 20);
    vheaderView->resizeSection(1, 20);
    vheaderView->resizeSection(2, 20);
    hheaderView->resizeSection(0, 60);
    for(int i=0; i<6; i++)
    {
        QTableWidgetItem *q = new QTableWidgetItem();
        q->setTextAlignment(Qt::AlignCenter);
        q->setText(QString::number(i+1));
        q->setBackground(Qt::gray);
        ui->TempTable->setItem(1, 2*i+1, q);
    }
    for(int i=0; i<6; i++)
    {
        QTableWidgetItem *q20 = new QTableWidgetItem();
        q20->setTextAlignment(Qt::AlignCenter);
        q20->setText("20°C");
        q20->setBackground(Qt::gray);
        ui->TempTable->setItem(2, 2*i+1, q20);
        QTableWidgetItem *q90 = new QTableWidgetItem();
        q90->setTextAlignment(Qt::AlignCenter);
        q90->setText("90°C");
        q90->setBackground(Qt::gray);
        ui->TempTable->setItem(2, 2*i+2, q90);
    }

    for(int i=0; i<6; i++)
    {
        QTableWidgetItem *q = new QTableWidgetItem();
        q->setTextAlignment(Qt::AlignCenter);
        q->setText(QString::number(i+1));
        q->setBackground(QBrush(QColor(scolors[i])));
        ui->TempTable->setItem(i+3, 0, q);
    }
    QTableWidgetItem *q = new QTableWidgetItem();
    q->setTextAlignment(Qt::AlignCenter);
    q->setText("Среднее");
    q->setBackground(QBrush(QColor("lightGray")));
    q->setFont(QFont("MS Shell", 10));
    ui->TempTable->setItem(9, 0, q);
    ui->TempTable->setItemDelegateForRow(9, new MyDelegate(ui->TempTable));

    ui->InaccTable->setColumnCount(4);
    ui->InaccTable->setRowCount(9);
    ui->InaccTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->InaccTable->setSelectionMode(QAbstractItemView::NoSelection);
    ui->InaccTable->setStyleSheet("border: 1px light gray");
    ui->InaccTable->setSpan(0, 0, 1, 4);
    ui->InaccTable->setSpan(1, 0, 1, 2);
    ui->InaccTable->setSpan(1, 2, 1, 2);

    QHeaderView *inachheaderView = new QHeaderView(Qt::Horizontal, ui->InaccTable);
    QHeaderView *inacvheaderView = new QHeaderView(Qt::Vertical, ui->InaccTable);
    ui->InaccTable->setHorizontalHeader(inachheaderView);
    ui->InaccTable->setVerticalHeader(inacvheaderView);
    ui->InaccTable->horizontalHeader()->hide();
    ui->InaccTable->verticalHeader()->hide();
    for(int i=0; i<4; i++)
    {
        inachheaderView->setSectionResizeMode(i, QHeaderView::Stretch);
    }
    for(int i=3; i<9; i++)
    {
        inacvheaderView->setSectionResizeMode(i, QHeaderView::Stretch);
    }
    inacvheaderView->resizeSection(0, 20);
    inacvheaderView->resizeSection(1, 20);
    inacvheaderView->resizeSection(2, 20);
    QTableWidgetItem *qt1 = new QTableWidgetItem();
    qt1->setText("Отклонения");
    qt1->setTextAlignment(Qt::AlignCenter);
    qt1->setBackground(Qt::gray);
    qt1->setFont(QFont("MS Shell", 12));
    ui->InaccTable->setItem(0, 0, qt1);
    QTableWidgetItem *q1 = new QTableWidgetItem();
    q1->setText("Текущие");
    q1->setTextAlignment(Qt::AlignCenter);
    q1->setBackground(Qt::gray);
    q1->setFont(QFont("MS Shell", 10));
    ui->InaccTable->setItem(1, 0, q1);
    QTableWidgetItem *q2 = new QTableWidgetItem();
    q2->setText("Накопленные");
    q2->setTextAlignment(Qt::AlignCenter);
    q2->setBackground(Qt::gray);
    q2->setFont(QFont("MS Shell", 10));
    ui->InaccTable->setItem(1, 2, q2);
    for(int i=0 ; i<2; i++)
    {
        QTableWidgetItem *q20 = new QTableWidgetItem();
        q20->setText("20°C");
        q20->setTextAlignment(Qt::AlignCenter);
        q20->setBackground(Qt::gray);
        q20->setFont(QFont("MS Shell", 10));
        ui->InaccTable->setItem(2, 2*i, q20);
        QTableWidgetItem *q90 = new QTableWidgetItem();
        q90->setText("90°C");
        q90->setTextAlignment(Qt::AlignCenter);
        q90->setBackground(Qt::gray);
        q90->setFont(QFont("MS Shell", 10));
        ui->InaccTable->setItem(2, 2*i+1, q90);
    }

    ui->TempTable->setContextMenuPolicy(Qt::ActionsContextMenu);
    QAction *a1 = new QAction("сделать все данные активными", this);
    QAction *a2 = new QAction("сделать все данные неактивными", this);
    QAction *a3 = new QAction("очистить таблицу", this);
    ui->TempTable->addAction(a1);
    ui->TempTable->addAction(a2);
    ui->TempTable->addAction(a3);
    ui->TempTable->connect(a1, SIGNAL(triggered()), this, SLOT(AllCellsEnabled()));
    ui->TempTable->connect(a2, SIGNAL(triggered()), this, SLOT(AllCellsDisabled()));
    ui->TempTable->connect(a3, SIGNAL(triggered()), this, SLOT(ClearTable()));
}

void MainWindow::SetCoefTable()
{
    ui->CoefTable->setColumnCount(13);
    ui->CoefTable->setRowCount(3);
    for(int i=1; i<13; i+=2)
    {
        ui->CoefTable->setSpan(0, i, 1, 2);
    }
    ui->CoefTable->setStyleSheet("border: 1px light gray");
    QHeaderView *vheaderView = new QHeaderView(Qt::Vertical, ui->TempTable);
    QHeaderView *hheaderView = new QHeaderView(Qt::Horizontal, ui->TempTable);
    ui->CoefTable->setVerticalHeader(vheaderView);
    ui->CoefTable->setHorizontalHeader(hheaderView);
    ui->CoefTable->verticalHeader()->hide();
    ui->CoefTable->horizontalHeader()->hide();
    for(int i=1; i<3; i++)
    {
        vheaderView->setSectionResizeMode(i, QHeaderView::Stretch);
    }
    for(int i=1; i<13; i++)
    {
        hheaderView->setSectionResizeMode(i, QHeaderView::Stretch);
    }
    vheaderView->resizeSection(0, 20);
    hheaderView->resizeSection(0, 60);
    QTableWidgetItem *q1 = new QTableWidgetItem();
    q1->setText("Текущие");
    q1->setBackground(QBrush(QColor(Qt::gray)));
    q1->setTextAlignment(Qt::AlignCenter);
    q1->setFont(QFont("MS Shell", 10));
    QTableWidgetItem *q2 = new QTableWidgetItem();
    q2->setText("Новые");
    q2->setBackground(QBrush(QColor(Qt::gray)));
    q2->setTextAlignment(Qt::AlignCenter);
    q2->setFont(QFont("MS Shell", 10));
    ui->CoefTable->setItem(1, 0, q1);
    ui->CoefTable->setItem(2, 0, q2);
    for(int i=0; i<6; i++)
    {
        QTableWidgetItem *q = new QTableWidgetItem();
        q->setText(QString::number(i+1));
        q->setTextAlignment(Qt::AlignCenter);
        q->setBackground(QBrush(QColor(Qt::gray)));
        q->setFont(QFont("MS Shell", 10));
        ui->CoefTable->setItem(0, 2*i+1, q);
    }
}

void MainWindow::AllCellsEnabled()
{
    int c = 0;
    for(int i=0; i<6; i++)
    {
        for(int j=0; j<6; j++)
        {
            if(ui->TempTable->cellWidget(i+3,2*j+1)!=NULL)
            {
                if(!ui->TempTable->cellWidget(i+3,2*j+1)->isEnabled())
                {
                    DisableCell(i+3,2*j+1);
                    c++;
                }
            }
        }
    }
    if(c!=0)
    {
        CalcAverageTemp();
        CalcNewCoefs();
    }
}

void MainWindow::AllCellsDisabled()
{
    int c = 0;
    for(int i=0; i<6; i++)
    {
        for(int j=0; j<6; j++)
        {
            if(ui->TempTable->cellWidget(i+3,2*j+1)!=NULL)
            {
                if(ui->TempTable->cellWidget(i+3,2*j+1)->isEnabled())
                {
                    DisableCell(i+3,2*j+1);
                    c++;
                }
            }
        }
    }
    if(c!=0)
    {
        CalcAverageTemp();
        CalcNewCoefs();
    }
}

void MainWindow::ClearTable()
{
    for(int i=3; i<9; i++)
    {
        for(int j=1; j<13; j++)
        {
            if(ui->TempTable->cellWidget(i, j)!=NULL)
                ui->TempTable->removeCellWidget(i, j);
        }
    }
    for(int j=1; j<13; j++)
    {
        if(ui->TempTable->item(9,j)!=NULL)
            delete ui->TempTable->item(9,j);
    }
    for(int i=3; i<9; i++)
    {
        for(int j=0; j<2; j++)
        {
            if(ui->InaccTable->item(i,j)!=NULL)
                delete ui->InaccTable->item(i,j);
        }
    }
    for(int i=1; i<3; i++)
    {
        for(int j=1; j<13; j++)
        {
            if(ui->CoefTable->item(i,j)!=NULL)
                delete ui->CoefTable->item(i,j);
        }
    }
    for(int i=0; i<6; i++)
    {
        inacc[i][0] = 0;
        inacc[i][1] = 0;
        for(int j=0; j<6; j++)
        {
            table_temp20[i][j] = 0;
            table_temp90[i][j] = 0;
        }
    }
    OnPositionChanged(position);
}

void MainWindow::DisableCell(int row, int column)
{
    if(!stage2ison)
    {
        if(row>2 && row<9 && column>0 && column<13)
        {
            if(ui->TempTable->cellWidget(row,column) != NULL)
            {
                if(column%2 == 1)
                {
                    if(ui->TempTable->cellWidget(row,column)->isEnabled())
                    {
                        ui->TempTable->cellWidget(row,column)->setEnabled(false);
                        ui->TempTable->cellWidget(row,column+1)->setEnabled(false);
                    }
                    else
                    {
                        ui->TempTable->cellWidget(row,column)->setEnabled(true);
                        ui->TempTable->cellWidget(row,column+1)->setEnabled(true);
                    }
                }
                if(column%2 == 0)
                {
                    if(ui->TempTable->cellWidget(row,column)->isEnabled())
                    {
                        ui->TempTable->cellWidget(row,column)->setEnabled(false);
                        ui->TempTable->cellWidget(row,column-1)->setEnabled(false);
                    }
                    else
                    {
                        ui->TempTable->cellWidget(row,column)->setEnabled(true);
                        ui->TempTable->cellWidget(row,column-1)->setEnabled(true);
                    }
                }
                CalcAverageTemp();
                CalcNewCoefs();
                CalcCurrentSensorsInacuracy();
            }
        }
    }
}

void MainWindow::ApplySettings(QColor c, int fs, int sensornumber)
{
    if(T8Kconnected)
    {
        if(!grafison)
        {
            ui->actionLoad->setEnabled(false);
            ClearGraph();
            T8KThread *p = new T8KThread;
            QObject::connect(p, SIGNAL(SendTemperature(QStringList, double)), SLOT(ReceiveTemperature(QStringList, double)));
            QObject::connect(p, SIGNAL(PortError(QString)), SLOT(SerialPortErrorHandler(QString)));
            QObject::connect(p, SIGNAL(PortConnected(QString)), SLOT(PortConnected(QString)));
            QObject::connect(p, SIGNAL(finished()), p, SLOT(deleteLater()));
            p->start();
        }
        for(int i=0; i<6; i++)
        {
            if(SensorsIsEnabled[i] == 0)
            {
                ui->TempTable->item(i+3, 0)->setBackground(QBrush(QColor(Qt::gray)));
                for(int j=0; j<6; j++)
                {
                    if(ui->TempTable->cellWidget(i+3, 2*j+1)!=NULL)
                    {
                        ui->TempTable->cellWidget(i+3,2*j+1)->setEnabled(false);
                        ui->TempTable->cellWidget(i+3,2*j+2)->setEnabled(false);
                    }
                }
            }
            else
            {
                ui->TempTable->item(i+3, 0)->setBackground(scolors[i]);
                for(int j=0; j<6; j++)
                {
                    if(ui->TempTable->cellWidget(i+3, 2*j+1)!=NULL)
                    {
                        ui->TempTable->cellWidget(i+3,2*j+1)->setEnabled(true);
                        ui->TempTable->cellWidget(i+3,2*j+2)->setEnabled(true);
                    }
                }
            }
        }        
        graphbackground = c;
        ui->plotwidget->setBackground(graphbackground);
        ui->miniplotwidget->setBackground(graphbackground);        
        fontsize = fs;
        if(sensorgraphnumber != sensornumber-1)
        {
            sensorgraphnumber = sensornumber-1;
            ui->miniplotwidget->graph(0)->setPen(QPen(scolors[sensorgraphnumber], 2));
        }
        ui->Tlabel->setText("  \r\n\r\n Датчик №"+QString::number(sensorgraphnumber+1));
        ReFontTables();
        if(!TableIsEmpty())
        {
            CalcAverageTemp();
            GetCoefs();
            CalcNewCoefs();
            CalcCurrentSensorsInacuracy();
        }
    }
    else
        QMessageBox::information(this, tr("Внимание!"), tr("T8K не подключен!"));
}

void MainWindow::ReFontTables()
{
    for(int i=3; i<9; i++)
    {
        for(int j=1; j<13; j++)
        {            
            if(ui->TempTable->cellWidget(i,j) != NULL)
            {
                QLabel *l = qobject_cast<QLabel*>(ui->TempTable->cellWidget(i, j));
                l->setFont(QFont("MS Shell", fontsize, 65));
            }
        }
    }
    for(int j=1; j<13; j++)
    {
        if(ui->TempTable->item(9,j) != NULL)
        {
            QTableWidgetItem *l = ui->TempTable->item(9, j);
            l->setFont(QFont("MS Shell", fontsize, 65));
        }
    }
    for(int i=3; i<9; i++)
    {
        for(int j=0; j<4; j++)
        {
            if(ui->InaccTable->item(i,j) != NULL)
            {
                QTableWidgetItem *l = ui->InaccTable->item(i,j);
                l->setFont(QFont("MS Shell", fontsize, 65));
            }
        }
    }
    for(int i=1; i<3; i++)
    {
        for(int j=1; j<13; j++)
        {
            if(ui->CoefTable->item(i,j) != NULL)
            {
                QTableWidgetItem *l = ui->CoefTable->item(i,j);
                l->setFont(QFont("MS Shell", fontsize, 65));
            }
        }
    }
}

void MainWindow::SerialPortErrorHandler(QString err)
{
    QMessageBox::information(this, tr("Serial Port Error!"), err);
    grafison = false;
}

void MainWindow::PortConnected(QString sNumber)
{
    T8KSN = sNumber.split(" ")[0];
    grafison = true;
    ui->groupBox->setTitle("Отклонения T8K "+T8KSN);
    ReadFromHistory();
}

void MainWindow::ReceiveTemperature(QStringList t, double time)
{
    ui->lcdRed->display(t[0]);
    ui->lcdGreen->display(t[1]);
    ui->lcdBlue->display(t[2]);
    ui->lcdCyan->display(t[3]);
    ui->lcdMagenta->display(t[4]);
    ui->lcdYellow->display(t[5]);
    if(isWorking)
    {
        for(int i=0; i<6; i++)
        {
            QLabel *l = new QLabel();
            l->setFont(QFont("MS Shell", fontsize, 65));
            l->setText("<font color='red'>"+QString::number(t[i].toDouble(), 'f', 2)+"</font>");
            l->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            if(!stage2ison)
                ui->TempTable->setCellWidget(i+3, 2*((position-1+i)%6)+1, l);
            else
                ui->TempTable->setCellWidget(i+3, 2*((position-1+i)%6)+2, l);
        }
    }
    for(int i=0; i<6; i++)
    {
        if(SensorsIsEnabled[i] == 1)
        {
            ui->plotwidget->graph(i)->addData(time, t[i].toDouble());
        }
    }

    if(measureRealTemp)
    {
        if(measures == 0)
        {
            measurestartpoint = time;
            AddLine(measurestartpoint);
        }
        for(int i=0; i<6; i++)
        {
            if(SensorsIsEnabled[i] == 1)
                realtemp[i] += t[i].toDouble();
            else
                realtemp[i] = -1;
        }
        measures++;
        measureendpoint = time;
    }

    ui->miniplotwidget->graph(0)->addData(time, t[sensorgraphnumber].toDouble());

    ui->Tlabel->setText("   "+QString::number(t[sensorgraphnumber].toDouble(), 'f', 2)+" °C"+"\r\n\r\n"+
                                                                                          " Датчик №"+
                        QString::number(sensorgraphnumber+1));
    if(time>=grafrange)
        ui->miniplotwidget->xAxis->setRange(time-grafrange, time);

    if(!isScaled) ui->plotwidget->rescaleAxes();
    ui->plotwidget->replot();
    ui->miniplotwidget->replot();
}

void MainWindow::ClearGraph()
{
    for(int i=0; i<6; i++)
    {
        ui->plotwidget->graph(i)->clearData();
    }
    ui->plotwidget->replot();
    ui->lcdRed->display("0");
    ui->lcdGreen->display("0");
    ui->lcdBlue->display("0");
    ui->lcdCyan->display("0");
    ui->lcdMagenta->display("0");
    ui->lcdYellow->display("0");
}

void MainWindow::SaveGraphToFile()
{
    QDate date = QDate::currentDate();
    QString filesave = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                    "График-"+DTSN+"-"+T8KSN+"-"+date.toString("dd.MM.yyyy"),
                                                    tr("Text File(*.txt)"));
    if(filesave != "")
    {
        QFile file(filesave);
        if(!file.open(QIODevice::WriteOnly))
        {
            QMessageBox::critical(this, tr("Error"), tr("Could not save file"));
            return;
        }
        else
        {
            QTextStream out(&file);
            QStringList slist;
            slist.append("#T8KRealTimeDTGraphData#|");
            for(int i=0; i<6; i++)
            {
                if(SensorsIsEnabled[i] == 1)
                {
                    QList<QCPData> d;
                    d = ui->plotwidget->graph(i)->data()->values();
                    for(int i=0; i<d.size(); i++)
                    {
                        slist.append(QString::number((d.value(i).key), 'f', 2)+",");
                        slist.append(QString::number((d.value(i).value), 'f', 2)+";");
                    }
                }
                else
                    slist.append("-");
                slist.append("|");
            }
            out << slist.join("");
            out.flush();
            file.close();
        }
    }
}

void MainWindow::LoadGraphFromFile()
{
    ClearGraph();
    QString fileopen = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    QDir::currentPath(),
                                                    tr("Text File(*.txt)"));
    if(fileopen != "")
    {
        QFile file(fileopen);
        if(!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return;
        }
        QTextStream in(&file);
        QString s;
        QStringList sensorslist, datalist, vklist;
        s = in.readAll();
        sensorslist = s.split("|");
        if(sensorslist[0] == "#T8KRealTimeDTGraphData#")
        {
            sensorslist.pop_back();
            for(int j=1; j<7; j++)
            {                
                if(sensorslist[j] != "-")
                {
                    datalist = sensorslist[j].split(";");                    
                    datalist.pop_back();
                    for(int i=0; i<datalist.size(); i++)
                    {
                        vklist = datalist[i].split(",");
                        ui->plotwidget->graph(j-1)->addData(vklist[0].toDouble(), vklist[1].toDouble());
                    }
                }
                else
                    ui->plotwidget->graph(j-1)->addData(0, -1);
            }
            if(ui->plotwidget->graph(0)->data()->values().value(0).value != -1)
                ui->lcdRed->display(ui->plotwidget->graph(0)->data()->values().value(datalist.size()-1).value);
            else
                ui->lcdRed->display("-");
            if(ui->plotwidget->graph(1)->data()->values().value(0).value != -1)
                ui->lcdGreen->display(ui->plotwidget->graph(1)->data()->values().value(datalist.size()-1).value);
            else
                ui->lcdGreen->display("-");
            if(ui->plotwidget->graph(2)->data()->values().value(0).value != -1)
                ui->lcdBlue->display(ui->plotwidget->graph(2)->data()->values().value(datalist.size()-1).value);
            else
                ui->lcdBlue->display("-");
            if(ui->plotwidget->graph(3)->data()->values().value(0).value != -1)
                ui->lcdCyan->display(ui->plotwidget->graph(3)->data()->values().value(datalist.size()-1).value);
            else
                ui->lcdCyan->display("-");
            if(ui->plotwidget->graph(4)->data()->values().value(0).value != -1)
                ui->lcdMagenta->display(ui->plotwidget->graph(4)->data()->values().value(datalist.size()-1).value);
            else
                ui->lcdMagenta->display("-");
            if(ui->plotwidget->graph(5)->data()->values().value(0).value != -1)
                ui->lcdYellow->display(ui->plotwidget->graph(5)->data()->values().value(datalist.size()-1).value);
            else
                ui->lcdYellow->display("-");

            ui->plotwidget->rescaleAxes();
            ui->plotwidget->replot();
        }
        else
            QMessageBox::information(this, tr("Error"), tr("Wrong file"));
        file.close();
    }
}

void MainWindow::ExportToExcel()
{
    QDate date = QDate::currentDate();
    QString filesave = QFileDialog::getSaveFileName(this, tr("Load File"),
                                                    "Таблица-"+DTSN+"-"+T8KSN+"-"+TermNumber+"-"+date.toString("dd.MM.yyyy"),
                                                    tr("(*.csv)"));
    if(filesave != "")
    {
        QFile file(filesave);
        if(!file.open(QIODevice::WriteOnly))
        {
            QMessageBox::critical(this, tr("Error"), tr("Could not save file"));
            return;
        }
        else
        {
            QTextStream out(&file);
            QStringList slist;
            for(int i=3; i<9; i++)
            {
                for(int j=0; j<6; j++)
                {
                    if(ui->TempTable->cellWidget(i, 2*j+1) != NULL)
                    {
                        QLabel *l20 = qobject_cast<QLabel*>(ui->TempTable->cellWidget(i,2*j+1));
                        slist.append(l20->text().replace(".", ","));
                    }                    
                    slist.append(";");
                }
                slist.append(" ;");
                for(int j=0; j<6; j++)
                {
                    if(ui->TempTable->cellWidget(i, 2*j+2) != NULL)
                    {
                        QLabel *l90 = qobject_cast<QLabel*>(ui->TempTable->cellWidget(i,2*j+2));
                        slist.append(l90->text().replace(".", ","));
                    }                    
                    slist.append(";");
                }
                slist.append("\r\n");
            }
            for(int j=0; j<6; j++)
            {
                if(ui->TempTable->item(9,2*j+1) != NULL)
                {
                    slist.append(ui->TempTable->item(9,2*j+1)->text().replace(".", ","));
                }
                else
                {
                    slist.append(" ");
                }
                slist.append(";");
            }
            slist.append(" ;");
            for(int j=0; j<6; j++)
            {
                if(ui->TempTable->item(9,2*j+2) != NULL)
                {
                    slist.append(ui->TempTable->item(9,2*j+2)->text().replace(".", ","));
                }
                else
                {
                    slist.append(" ");
                }
                slist.append(";");
            }
            slist.append("\r\n");
            out << slist.join("");
            file.close();
        }
    }
}

void MainWindow::ImportFromExcel()
{
    QString fileopen = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    QDir::currentPath(),
                                                    tr("(*.csv)"));
    if(fileopen != "")
    {
        QFile file(fileopen);
        if(!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return;
        }
        else
        {
            QTextStream in(&file);
            QString s;
            QStringList srows, row20item, row90item;
            s = in.readAll();
            srows = s.split("\r\n");
            for(int i=3; i<9; i++)
            {
                row20item = srows[i-3].split(" ")[0].split(";");
                row90item = srows[i-3].split(" ")[1].split(";");
                row90item.pop_front();
                for(int j=0; j<6; j++)
                {
                    QLabel *l20 = new QLabel();
                    QLabel *l90 = new QLabel();
                    l20->setFont(QFont("MS Shell", fontsize, 65));
                    l90->setFont(QFont("MS Shell", fontsize, 65));
                    l20->setText(row20item[j].replace(",", "."));
                    table_temp20[i-3][j] = row20item[j].replace(",", ".").toDouble();
                    l90->setText(row90item[j].replace(",", "."));
                    table_temp90[i-3][j] = row90item[j].replace(",", ".").toDouble();
                    l20->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                    l90->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                    ui->TempTable->setCellWidget(i, 2*j+1, l20);
                    ui->TempTable->setCellWidget(i, 2*j+2, l90);
                }
            }
            file.close();
            CalcAverageTemp();
            GetCoefs();
            CalcNewCoefs();
            CalcCurrentSensorsInacuracy();
        }
    }
}

void MainWindow::WriteRealTemp()
{
    for(int i=0; i<6; i++)
    {        
        if(stage2ison)
        {
            QLabel *l90 = new QLabel();
            l90->setFont(QFont("MS Shell", fontsize, 65));
            if(realtemp[i] != -1 && realtemp[i] != 0)
            {
                if(!ui->checkBoxCurrent->checkState() == Qt::CheckState())
                {
                    l90->setText(QString::number((realtemp[i]/measures)-inacc[i][1], 'f', 2));
                }
                if(!ui->checkBoxHistory->checkState() == Qt::CheckState())
                {
                    l90->setText(QString::number((realtemp[i]/measures)-h_inacc[i][1], 'f', 2));
                }
                if(ui->checkBoxCurrent->checkState() == Qt::CheckState() && ui->checkBoxHistory->checkState() == Qt::CheckState())
                {
                    l90->setText(QString::number(realtemp[i]/measures, 'f', 2));
                }
                table_temp90[i][(position-1+i)%6] = QString::number(realtemp[i]/measures, 'f', 2).toDouble();
            }
            else
            {
                l90->setText("-");
                table_temp90[i][(position-1+i)%6] = 0;
            }
            l90->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            ui->TempTable->setCellWidget(i+3, 2*((position-1+i)%6)+2, l90);
        }
        else
        {
            QLabel *l20 = new QLabel();
            l20->setFont(QFont("MS Shell", fontsize, 65));
            if(realtemp[i] != -1 && realtemp[i] != 0)
            {
                if(!ui->checkBoxCurrent->checkState() == Qt::CheckState())
                {
                    l20->setText(QString::number((realtemp[i]/measures)-inacc[i][0], 'f', 2));
                }
                if(!ui->checkBoxHistory->checkState() == Qt::CheckState())
                {
                    l20->setText(QString::number((realtemp[i]/measures)-h_inacc[i][0], 'f', 2));
                }
                if(ui->checkBoxCurrent->checkState() == Qt::CheckState() && ui->checkBoxHistory->checkState() == Qt::CheckState())
                {
                    l20->setText(QString::number(realtemp[i]/measures, 'f', 2));
                }
                table_temp20[i][(position-1+i)%6] = QString::number(realtemp[i]/measures, 'f', 2).toDouble();
            }
            else
            {
                l20->setText("-");
                table_temp20[i][(position-1+i)%6] = 0;
            }
            l20->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            ui->TempTable->setCellWidget(i+3, 2*((position-1+i)%6)+1, l20);
        }        
    }
    isWorking = false;
    measures = 0;
    measureRealTemp = false;
    measurefinished = true;
    OnPositionChanged(position);
    AutoSaveTable();
}

void MainWindow::CalcAverageTemp()
{
    if(!stage2ison)
    {
        double a20temp, a90temp;
        int m;
        for(int i=0; i<6; i++)
        {
            m = 0;
            a20temp = 0;
            a90temp = 0;
            for(int j=0; j<6; j++)
            {
                if(ui->TempTable->cellWidget(j+3,2*i+1) != NULL)
                {
                    if(ui->TempTable->cellWidget(j+3,2*i+1)->isEnabled())
                    {
                        QLabel *l20 = qobject_cast<QLabel*>(ui->TempTable->cellWidget(j+3, 2*i+1));
                        QLabel *l90 = qobject_cast<QLabel*>(ui->TempTable->cellWidget(j+3, 2*i+2));
                        if(l20->text() != "-")
                        {
                            a20temp += l20->text().toDouble();
                            a90temp += l90->text().toDouble();
                            m++;
                        }
                    }
                }
            }
            QTableWidgetItem *t20 = new QTableWidgetItem();
            QTableWidgetItem *t90 = new QTableWidgetItem();
            t20->setFont(QFont("MS Shell", fontsize, 65));
            t90->setFont(QFont("MS Shell", fontsize, 65));
            if(m == 0)
            {
                t20->setText("-");
                t90->setText("-");
                average20[i] = -1;
                average90[i] = -1;
            }
            else
            {
                average20[i] = QString::number(a20temp/m, 'f', 2).toDouble();
                average90[i] = QString::number(a90temp/m, 'f', 2).toDouble();
                t20->setText(QString::number(average20[i], 'f', 2));
                t90->setText(QString::number(average90[i], 'f', 2));
            }
            t20->setTextAlignment(Qt::AlignCenter);
            t90->setTextAlignment(Qt::AlignCenter);
            ui->TempTable->setItem(9, 2*i+1, t20);
            ui->TempTable->setItem(9, 2*i+2, t90);
        }        
    }
}

void MainWindow::UseCurrentSensorsInaccuracy()
{    
    if(!ui->checkBoxCurrent->checkState() == Qt::CheckState())  // Поставили галку "Текущие"
    {
        for(int i=0; i<6; i++)
        {
            for(int j=0; j<6; j++)
            {
               if(ui->TempTable->cellWidget(i+3,2*j+1) != NULL)
               {
                   QLabel *c20 = qobject_cast<QLabel*>(ui->TempTable->cellWidget(i+3,2*j+1));
                   QLabel *c90 = qobject_cast<QLabel*>(ui->TempTable->cellWidget(i+3,2*j+2));
                   if(c20->text() != "-")
                   {
                       c20->setText(QString::number(table_temp20[i][j]-inacc[i][0],'f',2));
                       c90->setText(QString::number(table_temp90[i][j]-inacc[i][1],'f',2));
                   }
               }
            }
        }
    }
    else                                                        // Не стоит галка "Текущие"
    {
        for(int i=0; i<6; i++)
        {
            for(int j=0; j<6; j++)
            {
                if(ui->TempTable->cellWidget(i+3,2*j+1) != NULL)
                {
                    QLabel *c20 = qobject_cast<QLabel*>(ui->TempTable->cellWidget(i+3,2*j+1));
                    QLabel *c90 = qobject_cast<QLabel*>(ui->TempTable->cellWidget(i+3,2*j+2));
                    if(c20->text() != "-")
                    {
                        c20->setText(QString::number(table_temp20[i][j],'f',2));
                        c90->setText(QString::number(table_temp90[i][j],'f',2));
                    }
                }
            }
        }
    }
    if(average20[0]!=0)
    {
        CalcAverageTemp();
        CalcNewCoefs();
    }
}

void MainWindow::UseHistorySensorsInaccuracy()
{
    if(!ui->checkBoxHistory->checkState() == Qt::CheckState())
    {
        for(int i=0; i<6; i++)
        {
            for(int j=0; j<6; j++)
            {
               if(ui->TempTable->cellWidget(i+3,2*j+1) != NULL)
               {
                   QLabel *c20 = qobject_cast<QLabel*>(ui->TempTable->cellWidget(i+3,2*j+1));
                   QLabel *c90 = qobject_cast<QLabel*>(ui->TempTable->cellWidget(i+3,2*j+2));
                   if(c20->text() != "-")
                   {
                       c20->setText(QString::number(table_temp20[i][j]-h_inacc[i][0],'f',2));
                       c90->setText(QString::number(table_temp90[i][j]-h_inacc[i][1],'f',2));
                   }
               }
            }
        }
    }
    else
    {
        for(int i=0; i<6; i++)
        {
            for(int j=0; j<6; j++)
            {
                if(ui->TempTable->cellWidget(i+3,2*j+1) != NULL)
                {
                    QLabel *c20 = qobject_cast<QLabel*>(ui->TempTable->cellWidget(i+3,2*j+1));
                    QLabel *c90 = qobject_cast<QLabel*>(ui->TempTable->cellWidget(i+3,2*j+2));
                    if(c20->text() != "-")
                    {
                        c20->setText(QString::number(table_temp20[i][j],'f',2));
                        c90->setText(QString::number(table_temp90[i][j],'f',2));
                    }
                }
            }
        }
    }
    if(average20[0]!=0)
    {
        CalcAverageTemp();
        CalcNewCoefs();
    }
}

void MainWindow::AutoSaveTable()
{
    QFile file("autosave.csv");
    if(!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this, tr("Error"), tr("Could not save file"));
        return;
    }
    else
    {
        QTextStream out(&file);
        QStringList slist;
        for(int i=3; i<9; i++)
        {
            for(int j=0; j<6; j++)
            {
                if(ui->TempTable->cellWidget(i, 2*j+1) != NULL)
                {
                    QLabel *l20 = qobject_cast<QLabel*>(ui->TempTable->cellWidget(i,2*j+1));
                    slist.append(l20->text());
                }
                slist.append(";");
            }
            slist.append(" ;");
            for(int j=0; j<6; j++)
            {
                if(ui->TempTable->cellWidget(i, 2*j+2) != NULL)
                {
                    QLabel *l90 = qobject_cast<QLabel*>(ui->TempTable->cellWidget(i,2*j+2));
                    slist.append(l90->text());
                }
                slist.append(";");
            }
            slist.append("\r\n");
        }
        out << slist.join("");
        file.close();
    }
}

void MainWindow::LoadAutoSavedTable()
{
    QFile file("autosave.csv");
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
        return;
    }
    else
    {
        QTextStream in(&file);
        QString s;
        QStringList srows, row20item, row90item;
        s = in.readAll();
        srows = s.split("\r\n");
        for(int i=3; i<9; i++)
        {
            row20item = srows[i-3].split(" ")[0].split(";");
            row90item = srows[i-3].split(" ")[1].split(";");
            row90item.pop_front();
            for(int j=0; j<6; j++)
            {
                QLabel *l20 = new QLabel();
                QLabel *l90 = new QLabel();
                l20->setFont(QFont("MS Shell", fontsize, 65));
                l90->setFont(QFont("MS Shell", fontsize, 65));
                l20->setText(row20item[j]);
                table_temp20[i-3][j] = row20item[j].toDouble();
                l90->setText(row90item[j]);
                table_temp90[i-3][j] = row90item[j].toDouble();
                l20->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                l90->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                ui->TempTable->setCellWidget(i, 2*j+1, l20);
                ui->TempTable->setCellWidget(i, 2*j+2, l90);
            }
        }
        file.close();
        CalcAverageTemp();
        GetCoefs();
        CalcNewCoefs();
        CalcCurrentSensorsInacuracy();
    }
}

bool MainWindow::TableIsFilled()
{
    int m = 0;
    for(int i=3; i<9; i++)
    {
        for(int j=1; j<13; j++)
        {
            if(ui->TempTable->cellWidget(i,j) == NULL)
                m++;
        }
    }
    if(m == 0)
        return true;
    else
        return false;
}

bool MainWindow::TableIsEmpty()
{
    int m =0;
    for(int i=3; i<9; i++)
    {
        for(int j=1; j<13; j++)
        {
            if(ui->TempTable->cellWidget(i,j) != NULL)
                m++;
        }
    }
    if(m == 0)
        return true;
    else
        return false;
}

void MainWindow::FillTable()
{
    for(int i=0; i<6; i++)
    {
        for(int j=0; j<6; j++)
        {
            double p20 = (qrand()%20)-10;
            double p90 = (qrand()%20)-10;
            double pd20 = p20/100;
            double pd90 = p90/100;
            QLabel *c20 = new QLabel(ui->TempTable);
            QLabel *c90 = new QLabel(ui->TempTable);
            c20->setFont(QFont("MS Shell", fontsize, 65));
            c90->setFont(QFont("MS Shell", fontsize, 65));
            c20->setText(QString::number(20+pd20,'f',2));
            table_temp20[i][j] = QString::number(20+pd20,'f',2).toDouble();
            c90->setText(QString::number(90+pd90,'f',2));
            table_temp90[i][j] = QString::number(90+pd90,'f',2).toDouble();
            c20->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            c90->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            ui->TempTable->setCellWidget(i+3,2*j+1,c20);
            ui->TempTable->setCellWidget(i+3,2*j+2,c90);
        }
    }    
    CalcAverageTemp();
    CalcCurrentSensorsInacuracy();
    CalcNewCoefs();
    CheckCalculatedInaccuracy();
}

void MainWindow::CalcCurrentSensorsInacuracy()
{
    for(int i=0; i<6; i++)
    {
        inacc[i][0] = 0;
        inacc[i][1] = 0;
    }
    int m;
    double table_average20[6], table_average90[6];
    for(int i=0; i<6; i++)
    {
        table_average20[i] = 0;
        table_average90[i] = 0;
    }
    for(int i=0; i<6; i++)
    {
        m = 0;
        for(int j=0; j<6; j++)
        {
            if(ui->TempTable->cellWidget(j+3, 2*i+1) != NULL)
            {
                if(ui->TempTable->cellWidget(j+3, 2*i+1)->isEnabled())
                {
                    if(table_temp20[j][i] != 0)
                    {
                        table_average20[i] += table_temp20[j][i];
                        table_average90[i] += table_temp90[j][i];
                        m++;
                    }
                }
            }
        }
        if(m != 0)
        {
            table_average20[i] /= m;
            table_average90[i] /= m;
        }
        else
        {
            table_average20[i] = 0;
            table_average90[i] = 0;
        }
    }
    for(int i=0; i<6; i++)
    {
        m = 0;
        for(int j=0; j<6; j++)
        {
            if(ui->TempTable->cellWidget(i+3, 2*j+1) != NULL)
            {
                    if(table_temp20[i][j] != 0)
                    {
                        inacc[i][0] += table_temp20[i][j]-table_average20[j];
                        inacc[i][1] += table_temp90[i][j]-table_average90[j];
                        m++;
                    }
            }
        }
        QTableWidgetItem *i20 = new QTableWidgetItem();
        QTableWidgetItem *i90 = new QTableWidgetItem();
        if(m == 0)
        {
            i20->setText("-");
            i90->setText("-");
        }
        else
        {
            inacc[i][0] = QString::number(floor(100*inacc[i][0]/m+0.5)/100, 'f', 2).toDouble();
            inacc[i][1] = QString::number(floor(100*inacc[i][1]/m+0.5)/100, 'f', 2).toDouble();
            i20->setText(QString::number(inacc[i][0],'f',2));
            i90->setText(QString::number(inacc[i][1],'f',2));
        }
        i20->setFont(QFont("MS Shell", fontsize, 65));
        i90->setFont(QFont("MS Shell", fontsize, 65));
        i20->setTextAlignment(Qt::AlignCenter);
        i90->setTextAlignment(Qt::AlignCenter);
        ui->InaccTable->setItem(i+3, 0, i20);
        ui->InaccTable->setItem(i+3, 1, i90);
    }
    ui->writeHistoryButton->setEnabled(true);
    CheckCalculatedInaccuracy();
}

void MainWindow::CheckCalculatedInaccuracy()
{
    if(ui->clearHistoryButton->isEnabled())
    {
        for(int i=0; i<6; i++)
        {
            if(ui->InaccTable->item(i+3, 0) != NULL)
            {
                if(ui->InaccTable->item(i+3, 0)->text() != "-")
                {
                    if(fabs(h_inacc[i][0]-inacc[i][0]) > 0.01)
                    {
                        if(fabs(h_inacc[i][0]-inacc[i][0]) > 0.02)
                        {
                            ui->InaccTable->item(i+3, 0)->setBackground(QBrush(QColor("#ff9424")));
                        }
                        if(fabs(h_inacc[i][0]-inacc[i][0]) > 0.05)
                        {
                            ui->InaccTable->item(i+3, 0)->setBackground(QBrush(QColor("#e40000")));
                        }
                    }
                    if(fabs(h_inacc[i][1]-inacc[i][1]) > 0.01)
                    {
                        if(fabs(h_inacc[i][1]-inacc[i][1]) > 0.02)
                        {
                            ui->InaccTable->item(i+3, 1)->setBackground(QBrush(QColor("#ff9424")));
                        }
                        if(fabs(h_inacc[i][1]-inacc[i][1]) > 0.05)
                        {
                            ui->InaccTable->item(i+3, 1)->setBackground(QBrush(QColor("#e40000")));
                        }
                    }
                }
            }
        }
    }
}

void MainWindow::ReadFromHistory()
{
    double in20[6], in90[6];
    int values20[6], values90[6];
    bool exists = false;
    for(int i=0; i<6; i++)
    {
        in20[i] = 0.0;
        in90[i] = 0.0;
        values20[i] = 0;
        values90[i] = 0;
    }
    int snumber = 0;
    QStringList tlist;
    QFile file("history " + T8KSN + ".xml");
    if(file.open(QIODevice::ReadOnly))
    {
        QXmlStreamReader sr(&file);
        while(!sr.atEnd())
        {
            if(sr.isStartElement())
            {
                if(sr.name() == "T8K")
                {
                    if(sr.attributes().value("number").toString() == T8KSN)
                    {
                        exists = true;
                        qDebug() << sr.attributes().value("number").toString(); // нашли нужный прибор
                        while(!sr.atEnd())
                        {
                            if(sr.isEndElement())
                            {
                                sr.readNext();
                                break;
                            }
                            if(sr.isStartElement())
                            {
                                if(sr.name() == "sensor")
                                {
                                    snumber = sr.attributes().value("id").toInt();
                                    while(!sr.atEnd())
                                    {
                                        if(sr.isEndElement())
                                        {
                                            sr.readNext();
                                            break;
                                        }
                                        if(sr.isStartElement())
                                        {
                                            if(sr.name() == "t20")
                                            {
                                                while(!sr.atEnd())
                                                {
                                                    if(sr.isEndElement())
                                                    {
                                                        sr.readNext();
                                                        break;
                                                    }
                                                    if(sr.isStartElement())
                                                    {
                                                        tlist =  sr.readElementText().split(";");
                                                        history_parametres << tlist;
                                                        for(int i=0; i<tlist.size(); i++)
                                                        {
                                                            in20[snumber] += tlist[i].toDouble();
                                                            values20[snumber]++;
                                                        }
                                                        sr.readNext();
                                                        break;
                                                    }
                                                    else
                                                    {
                                                        sr.readNext();
                                                    }
                                                }
                                            }
                                            if(sr.name() == "t90")
                                            {
                                                while(!sr.atEnd())
                                                {
                                                    if(sr.isEndElement())
                                                    {
                                                        sr.readNext();
                                                        break;
                                                    }
                                                    if(sr.isStartElement())
                                                    {
                                                        tlist =  sr.readElementText().split(";");
                                                        history_parametres << tlist;
                                                        for(int i=0; i<tlist.size(); i++)
                                                        {
                                                            in90[snumber] += tlist[i].toDouble();
                                                            values90[snumber]++;
                                                        }
                                                        sr.readNext();
                                                        break;
                                                    }
                                                    else
                                                    {
                                                        sr.readNext();
                                                    }
                                                }
                                            }

                                            sr.readNext();
                                        }
                                        else
                                            sr.readNext();
                                    }
                                }
                                if(sr.name() == "date")
                                {
                                    while(!sr.atEnd())
                                    {
                                        if(sr.isEndElement())
                                        {
                                            sr.readNext();
                                            break;
                                        }
                                        if(sr.isStartElement())
                                        {
                                            tlist =  sr.readElementText().split(";");
                                            history_parametres << tlist;
                                            sr.readNext();
                                            break;
                                        }
                                        else
                                        {
                                            sr.readNext();
                                        }
                                    }
                                }
                                sr.readNext();
                            }
                            else
                                sr.readNext();
                        }
                    }
                    else
                        sr.readNext();
                }
                else
                    sr.readNext();
            }
            else
                sr.readNext();
        }
        file.close();
        if(sr.hasError())
        {
            qDebug() << "Error: " << sr.errorString();
        }
    }

    for(int i=0; i<6; i++)
    {
        if(exists)
        {
            if(values20[i] == 0)
                h_inacc[i][0] = 0.0;
            else
                h_inacc[i][0] = QString::number(in20[i]/(values20[i]-1), 'f', 2).toDouble();
            if(values90[i] == 0)
                h_inacc[i][1] = 0.0;
            else
                h_inacc[i][1] = QString::number(in90[i]/(values90[i]-1), 'f', 2).toDouble();
            QTableWidgetItem *i20 = new QTableWidgetItem();
            QTableWidgetItem *i90 = new QTableWidgetItem();
            i20->setFont(QFont("MS Shell", fontsize, 65));
            i90->setFont(QFont("MS Shell", fontsize, 65));
            i20->setText(QString::number(h_inacc[i][0],'f',2));
            i90->setText(QString::number(h_inacc[i][1],'f',2));
            i20->setTextAlignment(Qt::AlignCenter);
            i90->setTextAlignment(Qt::AlignCenter);
            ui->InaccTable->setItem(i+3, 2, i20);
            ui->InaccTable->setItem(i+3, 3, i90);
        }
        else
        {
            h_inacc[i][0] = 0.0;
            h_inacc[i][1] = 0.0;
        }
    }
    if(exists)
    {
        ui->clearHistoryButton->setEnabled(true);
        ui->hlistButton->setVisible(true);
        CheckCalculatedInaccuracy();
    }
}

void MainWindow::WriteToHistory()
{
    bool exists = false;
    bool thisnumber;
    QString cursn;
    QDate date;
    QString sensorval;
    QVector<QString> sensordata;
    QVector<QString> devicename;
    QVector<QString> dates;
    QVector< QVector<QString> > devicedata;
    QVector< QVector< QVector<QString> > > indata;
    QFile file("history " + T8KSN +".xml");
    if(file.open(QIODevice::ReadOnly))
    {
        QXmlStreamReader sr(&file);
        while(!sr.atEnd())
        {
            if(sr.isStartElement())
            {
                if(sr.name() == "T8K")
                {
                    cursn = sr.attributes().value("number").toString();
                    if(cursn == T8KSN)
                    {
                        exists = true;
                        thisnumber = true;
                    }
                    else
                        thisnumber = false;
                    devicename << cursn;
                    devicedata << devicename;
                    devicename.clear();
                        while(!sr.atEnd())
                        {
                            if(sr.isEndElement())
                            {
                                indata << devicedata;
                                devicedata.clear();
                                sr.readNext();
                                break;
                            }
                            if(sr.isStartElement())
                            {
                                if(sr.name() == "sensor")
                                {
                                    sensordata << sr.attributes().value("id").toString();
                                    while(!sr.atEnd())
                                    {
                                        if(sr.isEndElement())
                                        {
                                            devicedata << sensordata;
                                            sensordata.clear();
                                            sr.readNext();
                                            break;
                                        }
                                        if(sr.isStartElement())
                                        {
                                            if(sr.name() == "t20")
                                            {
                                                while(!sr.atEnd())
                                                {
                                                    if(sr.isEndElement())
                                                    {
                                                        sr.readNext();
                                                        break;
                                                    }
                                                    if(sr.isStartElement())
                                                    {
                                                        sensorval = sr.readElementText();
                                                        if(thisnumber)
                                                        {
                                                            sensorval.append(QString::number(inacc[devicedata.size()-1][0], 'f', 2));
                                                            sensorval.append(";");
                                                        }
                                                        sensordata << sensorval;
                                                        sensorval = "";
                                                        sr.readNext();
                                                        break;
                                                    }
                                                    else
                                                    {
                                                        sr.readNext();
                                                    }
                                                }
                                            }
                                            if(sr.name() == "t90")
                                            {
                                                while(!sr.atEnd())
                                                {
                                                    if(sr.isEndElement())
                                                    {
                                                        sr.readNext();
                                                        break;
                                                    }
                                                    if(sr.isStartElement())
                                                    {
                                                        sensorval = sr.readElementText();
                                                        if(thisnumber)
                                                        {
                                                            sensorval.append(QString::number(inacc[devicedata.size()-1][1], 'f', 2));
                                                            sensorval.append(";");
                                                        }
                                                        sensordata << sensorval;
                                                        sensorval = "";
                                                        sr.readNext();
                                                        break;
                                                    }
                                                    else
                                                    {
                                                        sr.readNext();
                                                    }
                                                }
                                            }
                                            sr.readNext();
                                        }
                                        else
                                            sr.readNext();
                                    }
                                }
                                if(sr.name() == "date")
                                {
                                    while(!sr.atEnd())
                                    {
                                        if(sr.isEndElement())
                                        {
                                            sr.readNext();
                                            break;
                                        }
                                        if(sr.isStartElement())
                                        {
                                            sensorval = sr.readElementText();
                                            if(thisnumber)
                                            {
                                                sensorval.append(date.currentDate().toString("dd.MM.yyyy"));
                                                sensorval.append(";");
                                            }
                                            dates << sensorval;
                                            devicedata << dates;
                                            dates.clear();
                                            sensorval = "";
                                            sr.readNext();
                                            break;
                                        }
                                        else
                                        {
                                            sr.readNext();
                                        }
                                    }
                                }
                                sr.readNext();
                            }
                            else
                                sr.readNext();
                        }
                }
                else
                    sr.readNext();
            }
            else
                sr.readNext();
        }
        file.close();
        if(sr.hasError())
        {
            qDebug() << "Error: " << sr.errorString();
        }
    }    

    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QXmlStreamWriter sw(&file);
        sw.setAutoFormatting(true);
        sw.writeStartDocument();
            sw.writeStartElement("SensorsInaccuracy");
            for(int i=0; i<indata.size(); i++)
            {
                sw.writeStartElement("T8K");
                sw.writeAttribute("number", indata[i][0][0]);
                for(int j=1; j<7; j++)
                {
                    sw.writeStartElement("sensor");
                    sw.writeAttribute("id", indata[i][j][0]);
                        sw.writeTextElement("t20", indata[i][j][1]);
                        sw.writeTextElement("t90", indata[i][j][2]);
                    sw.writeEndElement();
                }
                sw.writeTextElement("date", indata[i][7][0]);
                sw.writeEndElement();
            }
            if(!exists)
            {
                sw.writeStartElement("T8K");
                sw.writeAttribute("number", T8KSN);
                for(int j=0; j<6; j++)
                {
                    sw.writeStartElement("sensor");
                    sw.writeAttribute("id", QString::number(j));
                        sw.writeTextElement("t20", QString::number(inacc[j][0], 'f', 2)+";");
                        sw.writeTextElement("t90", QString::number(inacc[j][1], 'f', 2)+";");
                    sw.writeEndElement();
                }
                sw.writeTextElement("date", date.currentDate().toString("dd.MM.yyyy")+";");
                sw.writeEndElement();
            }
            sw.writeEndElement();
        sw.writeEndDocument();
        file.close();
        QMessageBox::information(this, tr("Info"), tr("Данные добавлены в историю"));
    }
}

void MainWindow::ClearHistory()
{
    QString cursn;
    QVector<QString> sensordata;
    QVector<QString> devicename;
    QVector<QString> dates;
    QVector< QVector<QString> > devicedata;
    QVector< QVector< QVector<QString> > > indata;
    QFile file("history " + T8KSN +".xml");
    if(file.open(QIODevice::ReadOnly))
    {
        QXmlStreamReader sr(&file);
        while(!sr.atEnd())
        {
            if(sr.isStartElement())
            {
                if(sr.name() == "T8K")
                {
                    cursn = sr.attributes().value("number").toString();
                    if(cursn != T8KSN)
                    {
                        devicename << cursn;
                        devicedata << devicename;
                        devicename.clear();
                        while(!sr.atEnd())
                        {
                            if(sr.isEndElement())
                            {
                                indata << devicedata;
                                devicedata.clear();
                                sr.readNext();
                                break;
                            }
                            if(sr.isStartElement())
                            {
                                if(sr.name() == "sensor")
                                {
                                    sensordata << sr.attributes().value("id").toString();
                                    while(!sr.atEnd())
                                    {
                                        if(sr.isEndElement())
                                        {
                                            devicedata << sensordata;
                                            sensordata.clear();
                                            sr.readNext();
                                            break;
                                        }
                                        if(sr.isStartElement())
                                        {
                                            if(sr.name() == "t20")
                                            {
                                                while(!sr.atEnd())
                                                {
                                                    if(sr.isEndElement())
                                                    {
                                                        sr.readNext();
                                                        break;
                                                    }
                                                    if(sr.isStartElement())
                                                    {
                                                        sensordata << sr.readElementText();
                                                        sr.readNext();
                                                        break;
                                                    }
                                                    else
                                                    {
                                                        sr.readNext();
                                                    }
                                                }
                                            }
                                            if(sr.name() == "t90")
                                            {
                                                while(!sr.atEnd())
                                                {
                                                    if(sr.isEndElement())
                                                    {
                                                        sr.readNext();
                                                        break;
                                                    }
                                                    if(sr.isStartElement())
                                                    {
                                                        sensordata << sr.readElementText();
                                                        sr.readNext();
                                                        break;
                                                    }
                                                    else
                                                    {
                                                        sr.readNext();
                                                    }
                                                }
                                            }
                                            sr.readNext();
                                        }
                                        else
                                            sr.readNext();
                                    }
                                }
                                if(sr.name() == "date")
                                {
                                    while(!sr.atEnd())
                                    {
                                        if(sr.isEndElement())
                                        {
                                            sr.readNext();
                                            break;
                                        }
                                        if(sr.isStartElement())
                                        {
                                            dates << sr.readElementText();
                                            devicedata << dates;
                                            dates.clear();
                                            sr.readNext();
                                            break;
                                        }
                                        else
                                        {
                                            sr.readNext();
                                        }
                                    }
                                }
                                sr.readNext();
                            }
                            else
                                sr.readNext();
                        }
                    }
                    else
                        sr.readNext();
                }
                else
                    sr.readNext();
            }
            else
                sr.readNext();
        }
        file.close();
        if(sr.hasError())
        {
            qDebug() << "Error: " << sr.errorString();
        }
    }
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QXmlStreamWriter sw(&file);
        sw.setAutoFormatting(true);
        sw.writeStartDocument();
            sw.writeStartElement("SensorsInaccuracy");
            for(int i=0; i<indata.size(); i++)
            {
                sw.writeStartElement("T8K");
                sw.writeAttribute("number", indata[i][0][0]);
                for(int j=1; j<7; j++)
                {
                    sw.writeStartElement("sensor");
                    sw.writeAttribute("id", indata[i][j][0]);
                        sw.writeTextElement("t20", indata[i][j][1]);
                        sw.writeTextElement("t90", indata[i][j][2]);
                    sw.writeEndElement();
                }
                sw.writeTextElement("date", indata[i][7][0]);
                sw.writeEndElement();
            }
            sw.writeEndElement();
        sw.writeEndDocument();
        file.close();
        QMessageBox::information(this, tr("Info"), "История систематических отклонений для термометра "+
                             T8KSN+" очищена");
    }
}

void MainWindow::GetCoefs()
{
    if(DTconnected)
    {
        QStringList cofs;
        QString coef, cmd;
        Temp_uC = new CANChannels(usb, 3);
        for(int i=0; i<6; i++)
        {
            QTableWidgetItem *l1 = new QTableWidgetItem();
            QTableWidgetItem *l2 = new QTableWidgetItem();
            if(l1 != NULL && l2 != NULL)
            {
                l1->setFont(QFont("MS Shell", fontsize, 65));
                l2->setFont(QFont("MS Shell", fontsize, 65));
                cmd = "dlrs "+QString::number(i);
                USBCy_RW(cmd, coef, usb, Temp_uC);
                if(coef.split(' ').length() > 3)
                {
                    cofs = coef.split(' ');
                    old_coefs20[i] = cofs[2].toDouble();
                    old_coefs90[i] = cofs[3].toDouble();
                    l1->setText(QString::number(old_coefs20[i]));
                    l2->setText(QString::number(old_coefs90[i]));
                    l1->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                    l2->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                    ui->CoefTable->setItem(1, 2*i+1, l1);
                    ui->CoefTable->setItem(1, 2*i+2, l2);
               }
           }
        }
    }
    else
        QMessageBox::information(this, tr("Внимание!"), tr("Устройство не подключено."));
}

void MainWindow::CalcNewCoefs()
{
    for(int i=0; i<6; i++)
    {        
        QTableWidgetItem *l1 = new QTableWidgetItem();
        QTableWidgetItem *l2 = new QTableWidgetItem();
        l1->setFont(QFont("MS Shell", fontsize, 65));
        l2->setFont(QFont("MS Shell", fontsize, 65));
        if(average20[i] != -1 && average20[i] != 0)
        {
            double t = (90.0*average20[i]-20.0*average90[i])/(90.0-20.0);
            double t100 = (100.0-20.0)*(average90[i]-average20[i])/(90.0-20.0)+average20[i];

            new_coefs20[i] = old_coefs20[i]+t*256.0;
            new_coefs90[i] = old_coefs90[i]+(t100-t-100.0)*256.0;
            l1->setText(QString::number(floor(new_coefs20[i])));
            l2->setText(QString::number(floor(new_coefs90[i])));
        }
        else
        {
            l1->setText("-");
            l2->setText("-");
        }
        l1->setTextAlignment(Qt::AlignCenter);
        l2->setTextAlignment(Qt::AlignCenter);
        ui->CoefTable->setItem(2, 2*i+1, l1);
        ui->CoefTable->setItem(2, 2*i+2, l2);
    }
}

void MainWindow::SetPlot()
{
    QPalette redpal;
    redpal.setColor(QPalette::WindowText, Qt::red);
    QPalette greenpal;
    greenpal.setColor(QPalette::WindowText, Qt::green);
    QPalette bluepal;
    bluepal.setColor(QPalette::WindowText, Qt::blue);
    QPalette cyanpal;
    cyanpal.setColor(QPalette::WindowText, Qt::cyan);
    QPalette magentapal;
    magentapal.setColor(QPalette::WindowText, Qt::magenta);
    QPalette yellowpal;
    yellowpal.setColor(QPalette::WindowText, Qt::yellow);
    ui->lcdRed->setPalette(redpal);
    ui->lcdGreen->setPalette(greenpal);
    ui->lcdBlue->setPalette(bluepal);
    ui->lcdCyan->setPalette(cyanpal);
    ui->lcdMagenta->setPalette(magentapal);
    ui->lcdYellow->setPalette(yellowpal);
    ui->plotwidget->xAxis->setLabel("seconds");
    ui->plotwidget->axisRect()->setRangeZoom(Qt::Horizontal);
    ui->plotwidget->yAxis->setLabel("°C");
    ui->plotwidget->setBackground(graphbackground);
    QPen p;
    p.setColor(red);
    p.setWidth(2);
    ui->plotwidget->addGraph();
    ui->plotwidget->graph(0)->setPen(p);
    ui->plotwidget->graph(0)->setAntialiasedFill(false);
    ui->plotwidget->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->plotwidget->addGraph();
    p.setColor(green);
    ui->plotwidget->graph(1)->setPen(p);
    ui->plotwidget->graph(1)->setAntialiasedFill(false);
    ui->plotwidget->graph(1)->setLineStyle(QCPGraph::lsLine);
    ui->plotwidget->addGraph();
    p.setColor(blue);
    ui->plotwidget->graph(2)->setPen(p);
    ui->plotwidget->graph(2)->setAntialiasedFill(false);
    ui->plotwidget->graph(2)->setLineStyle(QCPGraph::lsLine);
    ui->plotwidget->addGraph();
    p.setColor(cyan);
    ui->plotwidget->graph(3)->setPen(p);
    ui->plotwidget->graph(3)->setAntialiasedFill(false);
    ui->plotwidget->graph(3)->setLineStyle(QCPGraph::lsLine);
    ui->plotwidget->addGraph();
    p.setColor(magenta);
    ui->plotwidget->graph(4)->setPen(p);
    ui->plotwidget->graph(4)->setAntialiasedFill(false);
    ui->plotwidget->graph(4)->setLineStyle(QCPGraph::lsLine);
    ui->plotwidget->addGraph();
    p.setColor(yellow);
    ui->plotwidget->graph(5)->setPen(p);
    ui->plotwidget->graph(5)->setAntialiasedFill(false);
    ui->plotwidget->graph(5)->setLineStyle(QCPGraph::lsLine);
}

void MainWindow::SetMiniPlot()
{
    QVector<double> ticks;
    ticks << 20;
    ticks << 90;

    ui->miniplotwidget->setBackground(graphbackground);
    ui->miniplotwidget->yAxis->setRange(0, 100);
    ui->miniplotwidget->yAxis->setAutoTicks(false);
    ui->miniplotwidget->yAxis->setTickVector(ticks);
    ui->miniplotwidget->xAxis->setRange(0, grafrange);

    ui->miniplotwidget->addGraph();
    ui->miniplotwidget->graph(0)->setPen(QPen(scolors[sensorgraphnumber], 2));
}

void MainWindow::AddLine(int x)
{
    QCPItemLine *l = new QCPItemLine(ui->plotwidget);
    ui->plotwidget->addItem(l);
    l->setPen(QPen(Qt::blue, 1, Qt::DashLine, Qt::SquareCap, Qt::BevelJoin));
    l->start->setCoords(x, 0);
    l->end->setCoords(x, 100);
}

void MainWindow::AddPlotTextMark(int x, int y)
{
    QCPItemText *t = new QCPItemText(ui->plotwidget);
    ui->plotwidget->addItem(t);
    t->position->setCoords(x, y);
    t->setText(" Расположение "+QString::number(position)+" ");
    t->setFont(QFont(font().family(), 12));
}

void MainWindow::mousePress(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {        
         p1 = e->pos();
         origin = e->pos();
         if (!rubberBand)
         {
             rubberBand = new QRubberBand(QRubberBand::Rectangle, ui->plotwidget);
         }
         rubberBand->setGeometry(QRect(origin, QSize()));
         rubberBand->show();
    }
    if(e->button() == Qt::RightButton)
    {
        QToolTip::showText(e->globalPos(),
                               QString::number(ui->plotwidget->yAxis->pixelToCoord(e->pos().y()), 'f', 2)+
                               " °C\r\n" + QString::number(ui->plotwidget->xAxis->pixelToCoord(e->pos().x()), 'f', 1)+" sec", this, rect() );
        QWidget::mousePressEvent(e);
    }
    if(e->button() == Qt::MidButton)
    {
        if(rubberBand)
        {
            if(rubberBand->isHidden())
            {
                p1 = e->pos();
                m3pressed = true;
            }
        }
    }
}

void MainWindow::mouseRelease(QMouseEvent *e)
{
    if(rubberBand)
    {
        if (e->button() == Qt::LeftButton)
        {
            p2 = e->pos();
            double w = (p2.x()-p1.x());
            double h = (p2.y()-p1.y());
            if(w>10 && h>10)
            {
                isScaled = true;
                double deltax = 0.0;
                double deltay = 0.0;
                deltax = w/ui->plotwidget->axisRect()->width();
                deltay = h/ui->plotwidget->axisRect()->height();
                ui->plotwidget->axisRect()->zoomEvent(deltax, deltay, QPoint((p1.x()+p2.x())/2.0,(p1.y()+p2.y())/2.0));
            }
            if(w<-10 && (h<-10 || h>10))
            {
                ui->plotwidget->rescaleAxes();
                ui->plotwidget->replot();
                isScaled = false;
            }
            rubberBand->hide();
        }
    }
    if(e->button() == Qt::MidButton)
        m3pressed = false;
}

void MainWindow::mouseMove(QMouseEvent *e)
{
    if(rubberBand)
        rubberBand->setGeometry(QRect(origin, e->pos()).normalized());
    if(isScaled)
        if(m3pressed)
        {
            pc = e->pos();
            ui->plotwidget->xAxis->moveRange(ui->plotwidget->xAxis->pixelToCoord(p1.x())-
                                             ui->plotwidget->xAxis->pixelToCoord(pc.x()));
            ui->plotwidget->yAxis->moveRange(ui->plotwidget->yAxis->pixelToCoord(p1.y())-
                                             ui->plotwidget->yAxis->pixelToCoord(pc.y()));
            ui->plotwidget->replot();
            p1 = e->pos();
        }
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    SaveSettings();
    if(isWorking)
    {
        QMessageBox m(QMessageBox::Question,
                        "Внимание!",
                        "Остановить текущую операцию?",
                        QMessageBox::Yes|QMessageBox::No);
        m.setButtonText(QMessageBox::Yes, "Да");
        m.setButtonText(QMessageBox::No, "Нет");
        if(m.exec() == QMessageBox::Yes)
        {
            StopDTCalib();
        }
        qDebug() << "Stop";
    }
    QFile file("autosave.csv");
    if(file.exists())
        file.remove();

    if(!td->isHidden())
        td->hide();
    e->accept();
}

void MainWindow::OpenSettingsDialog()
{
    QStringList custom;
    custom << graphbackground.name() << QString::number(fontsize) << QString::number(sensorgraphnumber);
    SettingsDialog *d = new SettingsDialog();
    QObject::connect(this, SIGNAL(sendSettings(QStringList)), d, SLOT(GetSettings(QStringList)));
    QObject::connect(d, SIGNAL(setupsettings(QColor, int, int)), this, SLOT(ApplySettings(QColor, int, int)));
    d->setModal(true);
    emit sendSettings(custom);
    d->exec();
}

void MainWindow::OnPositionChanged(int n)
{
    position = n;
    ui->positiongroupBox->setTitle("Расположение "+QString::number(n));
    for(int i=0; i<6; i++)
    {
        for(int j=0; j<6; j++)
        {
            if(ui->TempTable->cellWidget(i+3,2*j+1) == NULL)
            {
                QTableWidgetItem *q20 = new QTableWidgetItem();
                QTableWidgetItem *q90 = new QTableWidgetItem();
                if(j == (position-1+i)%6)
                {
                    q20->setBackground(QBrush(QColor("#ADD8E6")));
                    q90->setBackground(QBrush(QColor("#ADD8E6")));
                }
                else
                {
                    q20->setBackground(QBrush(QColor(Qt::white)));
                    q90->setBackground(QBrush(QColor(Qt::white)));
                }
                ui->TempTable->setItem(i+3, 2*j+1, q20);
                ui->TempTable->setItem(i+3, 2*j+2, q90);
            }
            else
            {
                QLabel *l20 = qobject_cast<QLabel*>(ui->TempTable->cellWidget(i+3,2*j+1));
                if(j == (position-1+i)%6)
                {
                    l20->setStyleSheet("background-color: #ADD8E6");
                }
                else
                {
                    l20->setStyleSheet("background-color: white");
                }
                if(ui->TempTable->cellWidget(i+3,2*j+2) != NULL)
                {
                    QLabel *l90 = qobject_cast<QLabel*>(ui->TempTable->cellWidget(i+3,2*j+2));
                    if(j == (position-1+i)%6)
                    {
                        l90->setStyleSheet("background-color: #ADD8E6");
                    }
                    else
                    {
                        l90->setStyleSheet("background-color: white");
                    }
                }
                else
                {
                    QTableWidgetItem *q90 = new QTableWidgetItem();
                    if(j == (position-1+i)%6)
                    {
                        q90->setBackground(QBrush(QColor("#ADD8E6")));
                    }
                    else
                    {
                        q90->setBackground(QBrush(QColor(Qt::white)));
                    }
                    ui->TempTable->setItem(i+3, 2*j+2, q90);
                }
            }
        }
    }
}

void MainWindow::on_write_default_clicked()
{
    QMessageBox m(QMessageBox::Question,
                    "Внимание! ",
                    "Вы уверены что хотите обнулить коэффициенты?",
                    QMessageBox::Yes|QMessageBox::No);
    m.setButtonText(QMessageBox::Yes, "Да");
    m.setButtonText(QMessageBox::No, "Нет");
    if(m.exec() == QMessageBox::Yes)
    {
        QString cmd, outs;
        Temp_uC = new CANChannels(usb, 3);
        for(int i = 0; i<6; i++)
        {
            cmd = "dlws BCAL " +QString::number(i)+ " " + "0" + " " + "0";
            USBCy_RW(cmd, outs, usb, Temp_uC);
        }
        QMessageBox::information(this, "", "Коэффициенты обнулены. Для того, чтобы новые коэффициенты стали действительны"
                                           " требуется перезапустить устройство ");
        ClearTable();
    }
}

void MainWindow::on_WriteButton_clicked()
{
    QMessageBox m(QMessageBox::Question,
                    "Внимание! ",
                    "Вы уверены что хотите записать коэффициенты?",
                    QMessageBox::Yes|QMessageBox::No);
    m.setButtonText(QMessageBox::Yes, "Да");
    m.setButtonText(QMessageBox::No, "Нет");
    if(m.exec() == QMessageBox::Yes)
    {
        QString cmd, out;
        Temp_uC = new CANChannels(usb, 3);
        for (int i=0; i<6; i++)
        {
            QTableWidgetItem *q20 = ui->CoefTable->item(2, 2*i+1);
            QTableWidgetItem *q90 = ui->CoefTable->item(2, 2*i+2);

            if(q20 != NULL && q90 != NULL)
            {
                if(q20->text() != "" && q90->text() != "")
                {
                    cmd = "dlws BCAL "+QString::number(i)+" "+q20->text()+" "+q90->text();
                    USBCy_RW(cmd, out, usb, Temp_uC);
                }
            }
        }

        if(out == "0")
        {
            QMessageBox::information(this, tr("Внимание"), tr("Для того, чтобы новые коэффициенты стали действительны"
                                                             " требуется перезапустить устройство"));
        }
        else
        {
            QMessageBox::information(this, tr("Error"), tr("Ошибка при записи коэффициентов"));
        }
        QMessageBox m(QMessageBox::Question,
                        " ",
                        "Сохранить данные таблицы?",
                        QMessageBox::Yes|QMessageBox::No);
        m.setButtonText(QMessageBox::Yes, "Да");
        m.setButtonText(QMessageBox::No, "Нет");
        if(m.exec() == QMessageBox::Yes)
        {
            ExportToExcel();
        }
        ClearTable();
    }
}

void MainWindow::on_pushButton_clicked()
{
    GetCoefs();
}

void MainWindow::on_clearHistoryButton_clicked()
{
    QMessageBox m(QMessageBox::Question,
                    "Внимание!",
                    "Вы уверены что хотит очистить историю систематических отклонений для термометра "+
                                  T8KSN+"?",
                    QMessageBox::Yes|QMessageBox::No);
    m.setButtonText(QMessageBox::Yes, "Да");
    m.setButtonText(QMessageBox::No, "Нет");
    if(m.exec() == QMessageBox::Yes)
    {
        ClearHistory();
    }
}
void MainWindow::on_writeHistoryButton_clicked()
{
    QMessageBox m(QMessageBox::Question,
                    "Внимание!",
                    "Если в таблице присутствуют исключённые измерения, систематические отклонения датчиков НЕ сохранять!",
                    QMessageBox::Yes|QMessageBox::No);
    m.setButtonText(QMessageBox::Yes, "Сохранить");
    m.setButtonText(QMessageBox::No, "Продолжить без сохранения");
    if(m.exec() == QMessageBox::Yes)
    {
       WriteToHistory() ;
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    FillTable();
}

void MainWindow::GetNewHistory(QVector<QStringList> newh)
{
    for(int i=0; i<newh.size(); i++)
    {
        qDebug() << newh[i];
    }
    history_parametres.clear();
    bool thisnumber;
    int snumber = 0;
    QString cursn;
    QString sensorval;
    QVector<QString> sensordata;
    QVector<QString> devicename;
    QVector<QString> dates;
    QVector< QVector<QString> > devicedata;
    QVector< QVector< QVector<QString> > > indata;
    QFile file("history " + T8KSN +".xml");
    if(file.open(QIODevice::ReadOnly))
    {
        QXmlStreamReader sr(&file);
        while(!sr.atEnd())
        {
            if(sr.isStartElement())
            {
                if(sr.name() == "T8K")
                {
                    cursn = sr.attributes().value("number").toString();
                    if(cursn == T8KSN)
                    {
                        thisnumber = true;
                    }
                    else
                        thisnumber = false;
                    devicename << cursn;
                    devicedata << devicename;
                    devicename.clear();
                        while(!sr.atEnd())
                        {
                            if(sr.isEndElement())
                            {
                                indata << devicedata;
                                devicedata.clear();
                                sr.readNext();
                                break;
                            }
                            if(sr.isStartElement())
                            {
                                if(sr.name() == "sensor")
                                {
                                    snumber = sr.attributes().value("id").toInt();
                                    sensordata << sr.attributes().value("id").toString();
                                    while(!sr.atEnd())
                                    {
                                        if(sr.isEndElement())
                                        {
                                            devicedata << sensordata;
                                            sensordata.clear();
                                            sr.readNext();
                                            break;
                                        }
                                        if(sr.isStartElement())
                                        {
                                            if(sr.name() == "t20")
                                            {
                                                while(!sr.atEnd())
                                                {
                                                    if(sr.isEndElement())
                                                    {
                                                        sr.readNext();
                                                        break;
                                                    }
                                                    if(sr.isStartElement())
                                                    {
                                                        sensorval = sr.readElementText();
                                                        if(thisnumber)
                                                        {
                                                            sensorval = "";
                                                            for(int i=0; i<newh[2*snumber].size(); i++)
                                                            {
                                                                sensorval.append(newh[2*snumber][i]);
                                                                sensorval.append(";");
                                                            }
                                                        }
                                                        sensordata << sensorval;
                                                        sensorval = "";
                                                        sr.readNext();
                                                        break;
                                                    }
                                                    else
                                                    {
                                                        sr.readNext();
                                                    }
                                                }
                                            }
                                            if(sr.name() == "t90")
                                            {
                                                while(!sr.atEnd())
                                                {
                                                    if(sr.isEndElement())
                                                    {
                                                        sr.readNext();
                                                        break;
                                                    }
                                                    if(sr.isStartElement())
                                                    {
                                                        sensorval = sr.readElementText();
                                                        if(thisnumber)
                                                        {
                                                            sensorval = "";
                                                            for(int i=0; i<newh[2*snumber+1].size(); i++)
                                                            {
                                                                sensorval.append(newh[2*snumber+1][i]);
                                                                sensorval.append(";");
                                                            }
                                                        }
                                                        sensordata << sensorval;
                                                        sensorval = "";
                                                        sr.readNext();
                                                        break;
                                                    }
                                                    else
                                                    {
                                                        sr.readNext();
                                                    }
                                                }
                                            }
                                            sr.readNext();
                                        }
                                        else
                                            sr.readNext();
                                    }
                                }
                                if(sr.name() == "date")
                                {
                                    while(!sr.atEnd())
                                    {
                                        if(sr.isEndElement())
                                        {
                                            sr.readNext();
                                            break;
                                        }
                                        if(sr.isStartElement())
                                        {
                                            sensorval = sr.readElementText();
                                            if(thisnumber)
                                            {
                                                sensorval = "";
                                                for(int i=0; i<newh[12].size(); i++)
                                                {
                                                    sensorval.append(newh[12][i]);
                                                    sensorval.append(";");
                                                }
                                            }
                                            dates << sensorval;
                                            devicedata << dates;
                                            dates.clear();
                                            sensorval = "";
                                            sr.readNext();
                                            break;
                                        }
                                        else
                                        {
                                            sr.readNext();
                                        }
                                    }
                                }
                                sr.readNext();
                            }
                            else
                                sr.readNext();
                        }
                }
                else
                    sr.readNext();
            }
            else
                sr.readNext();
        }
        file.close();
        if(sr.hasError())
        {
            qDebug() << "Error: " << sr.errorString();
        }
    }

    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QXmlStreamWriter sw(&file);
        sw.setAutoFormatting(true);
        sw.writeStartDocument();
            sw.writeStartElement("SensorsInaccuracy");
            for(int i=0; i<indata.size(); i++)
            {
                sw.writeStartElement("T8K");
                sw.writeAttribute("number", indata[i][0][0]);
                for(int j=1; j<7; j++)
                {
                    sw.writeStartElement("sensor");
                    sw.writeAttribute("id", indata[i][j][0]);
                        sw.writeTextElement("t20", indata[i][j][1]);
                        sw.writeTextElement("t90", indata[i][j][2]);
                    sw.writeEndElement();
                }
                sw.writeTextElement("date", indata[i][7][0]);
                sw.writeEndElement();
            }
            sw.writeEndElement();
        sw.writeEndDocument();
        file.close();
    }
    QThread::msleep(100);

    ReadFromHistory();
}

void MainWindow::on_hlistButton_clicked()
{
    InaccDialog *dd = new InaccDialog();
    QObject::connect(this, SIGNAL(SendHistoryList(QVector<QStringList>)), dd, SLOT(SetTable(QVector<QStringList>)));
    QObject::connect(dd, SIGNAL(sendNewParams(QVector<QStringList>)), this, SLOT(GetNewHistory(QVector<QStringList>)));
    emit SendHistoryList(history_parametres);
    dd->setModal(true);
    dd->exec();
}


void MainWindow::onWriteNumberType(QString number, QString type, bool cheked)
{
    Temp_uC = new CANChannels(usb, 3);
    if(cheked == true)
    {
            QString cmd, outs;
            QString temp = "0";
            for(int i = 0; i<6; i++)
            {
                cmd ="dlws BCAL "+QString::number(i)+" "+temp+" "+temp +" "+ QString::number(i);
                USBCy_RW(cmd, outs, usb, Temp_uC);
            }
            for(int i = 6; i<8; i++)
            {
                cmd ="dlws BCAL "+QString::number(i)+" "+temp+" "+temp +" "+ QString::number(i);
                USBCy_RW(cmd, outs, usb, Temp_uC);
            }
    }

    QString numberofterm,  typeofterm, outs;
    numberofterm = number;
    typeofterm = type;

    USBCy_RW(numberofterm, outs, usb, Temp_uC);
    QThread::msleep(100);
    if(outs != "0")
    {
        QMessageBox::critical(this, "Внимание!", "Номер термоблока не был прописан!");
    }
    USBCy_RW(typeofterm, outs, usb, Temp_uC);
    QThread::msleep(100);
    if(outs != "0")
    {
        QMessageBox::critical(this, "Внимание!", "Тип термоблока не был прописан!");
    }

    QString ttype, TermNumber;
    USBCy_RW("HRTY", outs, usb, Temp_uC);
    TermNumber = outs;
    QThread::msleep(100);
    USBCy_RW("HRID", outs, usb, Temp_uC);
    ttype = outs;
    QThread::msleep(100);

    ui->coeflabel->setText("Коэффициенты Термоблока №"+TermNumber+" (Тип - "+ttype+")");
}

void MainWindow::on_WriteButton_2_clicked()
{
    QString n, t, outs;
    USBCy_RW("HRTY", outs, usb, Temp_uC);
    n = outs;
    QThread::msleep(100);
    USBCy_RW("HRID", outs, usb, Temp_uC);
    t = outs;
    QThread::msleep(100);

    emit sendNumberType(n, t);
    td->setGeometry(this->x()+this->width()/2-td->width()/2, this->y()+this->height()/2-td->height()/2, td->width(), td->height());

    td->show();
}
