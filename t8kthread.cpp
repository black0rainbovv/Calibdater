#include "t8kthread.h"
#include "mainwindow.h"

#include <QTime>
#include <QDebug>


void T8KThread::run()
{
    running = true;
    QTime t;
    MyPort *p = new MyPort();
    QObject::connect(p, SIGNAL(error_(QString)), SIGNAL(PortError(QString)));
    double time = 0.0;
    QList<QString> fdatalist[6];
    QByteArray data;
    QString out;
    double fdata;
    int timeout = 0;
    int format = 0;
    p->SetSettings(T8KPortName, 9600, 8, 0, 2, 0);
    if(p->ConnectPort())
    {
        if(p->GetPid() != "24577")
        {
            emit PortError("Подключаемое устройство не является термометром T8K");
        }
        else
        {
            data.append("SN\r");
            p->WriteToPort(data);
            emit PortConnected(p->ReadFromPort());
            data.clear();

            data.append("t 0\r");
            while(timeout != 10)
            {
                p->WriteToPort(data);
                format = p->ReadFromPort().length();
                if(format != 0)
                    break;
                qDebug() << QString::number(format);
            }
            timeout = 0;
            while(running)
            {
                t.start();
                for(int i=0; i<6; i++)
                {
                    data.clear();
                    fdata = 0;
                    if(SensorsIsEnabled[i] == 1)
                    {
                        data.append("t "+QString::number(i)+"\r");
                        p->WriteToPort(data);
                        QByteArray t = p->ReadFromPort();
                        if(t.length() != format)
                        {
                            timeout = 0;
                            while(timeout != 10)
                            {
                                QThread::msleep(150);
                                p->WriteToPort(data);
                                t = p->ReadFromPort();
                                //qDebug() << "timeout..." << QString::number(timeout) << " " << t;
                                if(t.length() == 6)
                                    break;
                                timeout++;
                            }                        }
                        out = ConvertAnswer(t);
                        if(out == NULL)
                        {
                            running = false;
                            templist.clear();
                            templist.append("0");
                            templist.append("0");
                            templist.append("0");
                            templist.append("0");
                            templist.append("0");
                            templist.append("0");
                            break;
                        }
                        double td = (out.toDouble()/100.0);
                        fdatalist[i] << QString::number(td, 'f', 2);
                        if(fdatalist[i].size() < Filter)
                        {
                            templist.append(QString::number(td, 'f', 2));
                        }
                        else
                        {
                            while(fdatalist[i].size()>Filter)
                            {
                                fdatalist[i].pop_front();
                            }
                            for(int j=0; j<fdatalist[i].size(); j++)
                            {
                                fdata += fdatalist[i].value(j).toDouble();
                            }
                            templist.append(QString::number(fdata/(double)Filter, 'f', 2));
                            fdatalist[i].pop_front();
                        }
                    }
                    else
                        templist.append("-");
                }
                emit SendTemperature(templist, time);
                templist.clear();
                time+=Interval;
                //qDebug() << (QString::number(Interval*1000-t.elapsed()));
                //qDebug() << "---------";
                if(Interval*1000-t.elapsed() > 0)
                    QThread::msleep(Interval*1000-t.elapsed());
            }
        }
        emit PortError("Ошибка при получения данных с термометра");
        p->DisconnectPort();
    }
}

QString T8KThread::ConvertAnswer(QByteArray answer)
{
   QRegExp rx("(\\d+)");
   QString canswer;
   int pos = 0;
   answer.simplified();
   while ((pos = rx.indexIn(answer, pos)) != -1) {
       canswer = rx.cap(1);
       pos += rx.matchedLength();
   }
   return canswer;
}






