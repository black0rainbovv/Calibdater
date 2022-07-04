#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "global.h"

#include <QMainWindow>
#include <QTimer>
#include <QLCDnumber>
#include <QVector>
#include <QPalette>
#include <QRubberBand>
#include <QThread>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QTableWidget>
#include <QList>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QCheckBox>
#include <QMediaPlayer>
#include <QDir>
#include <QUrl>
#include <QXmlStreamReader>

#include "CypressUsb.h"
#include "CANChannels.h"
#include "Utility.h"
#include "sensorsposition.h"
#include "celllabel.h"
#include "t8kthread.h"
#include "inaccdialog.h"
#include "thermoblockdialog.h"
#include"selectdeviceform.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT   

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    SelectedArea(QWidget * parent = 0);

public:
    int x0, y0, x1, y1;    
    QRect rectSelectedArea;

public slots:
    void OpenSettingsDialog();
    void SetPlot();
    void SetMiniPlot();
    void AddPlotTextMark(int x, int y);
    void AddLine(int x);    
    void SetTempTable();
    void SetCoefTable();
    void ReceiveTemperature(QStringList t, double time);
    void ApplySettings(QColor c, int fontsize, int sensornumber);
    void SaveGraphToFile();
    void LoadGraphFromFile();
    void ExportToExcel();
    void ImportFromExcel();
    void ClearTable();
    void WriteRealTemp();
    void StartDTCalibStage1();
    void StartDTCalibStage2();
    void StartDTCalib();
    void StopDTCalib();
    void GetNewHistory(QVector<QStringList>);

    void FillTable();

private slots:
    void ReadSettings();
    void SaveSettings();
    void GetCoefs();
    void CalcNewCoefs();
    void CalcAverageTemp();
    void UseCurrentSensorsInaccuracy();
    void UseHistorySensorsInaccuracy();
    void CalcCurrentSensorsInacuracy();
    void CheckCalculatedInaccuracy();
    void ReadFromHistory();
    void WriteToHistory();
    void ClearHistory();
    void mousePress(QMouseEvent *e);
    void mouseMove(QMouseEvent *e);
    void mouseRelease(QMouseEvent *e);
    void closeEvent(QCloseEvent *e);
    void Calib20C();
    void Calib90C();
    void DTStatusCheck();
    void OnPositionChanged(int n);
    void on_WriteButton_clicked();
    void DisableCell(int row, int column);
    void ClearGraph();
    void SerialPortErrorHandler(QString err);
    void PortConnected(QString sNumber);
    void ReFontTables();
    void FindT8K();
    bool TableIsFilled();
    bool TableIsEmpty();
    void AllCellsEnabled();
    void AllCellsDisabled();
    void PlayBell();
    void AutoSaveTable();
    void LoadAutoSavedTable();
    void on_pushButton_clicked();
    void on_clearHistoryButton_clicked();
    void on_pushButton_2_clicked();

    void on_hlistButton_clicked();

    void onWriteNumberType(QString number, QString type, bool cheked);
    void on_WriteButton_2_clicked();


    void on_write_default_clicked();

    void on_writeHistoryButton_clicked();

    void on_actionConnect_triggered();

    void on_actionDisconnect_triggered();

    void setIndex(int index);

signals:
    void areaIsSelected(void);
    void windowClose(void);
    void sendSettings(QStringList);
    void SendHistoryList(QVector<QStringList>);
    void sendNumberType(QString, QString);


private:
    Ui::MainWindow *ui;

    bool isScaled;
    selectdeviceform *deviceForm;
    QRubberBand *rubberBand;
    QPoint origin;    
    bool isWorking;
    CyDev *usb;
    CANChannels *Temp_uC;
    bool measureRealTemp;
    bool DTconnected;
    bool stage2ison;
    bool warmingup;
    bool grafison;
    bool m3pressed;
    bool measurefinished;
    QColor graphbackground;
    int fontsize;
    int sensorgraphnumber;

    QTimer *t20, *t90, *tstatus;
    int measures;
    int measurestartpoint;
    int measureendpoint;
    int position;
    int ina;
    QString T8KSN, DTSN, TermNumber;
    QVector<QStringList> history_parametres;

    ThermoblockDialog *td;
};

#endif // MAINWINDOW_H
