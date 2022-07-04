#ifndef T8KTHREAD
#define T8KTHREAD

#include "global.h"
#include "myport.h"
#include "settingsdialog.h"
#include "mainwindow.h"


class T8KThread : public QThread
{
    Q_OBJECT

    void run();

public slots:
    QString ConvertAnswer(QByteArray answer);   

signals:
    void SendTemperature(QStringList t, double time);
    void PortError(QString err);
    void PortConnected(QString sNumber);
    void DataLost();

private:   
    QStringList templist;
    bool running;
};



#endif // T8KTHREAD

