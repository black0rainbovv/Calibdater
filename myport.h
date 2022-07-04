#ifndef MYPORT
#define MYPORT

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>

struct Settings
{
    QString name;
    QSerialPort::BaudRate baudrate;
    QSerialPort::DataBits databits;
    QSerialPort::Parity parity;
    QSerialPort::StopBits stopbits;
    QSerialPort::FlowControl flowcontrol;
};

class MyPort : public QObject
{
    Q_OBJECT

public:
    QSerialPort sPort;
    Settings PortSettings;

public slots:
    void SetSettings(QString name, int baudrate, int databits, int parity, int stopbits, int flowcontrol);
    bool ConnectPort();
    void WriteToPort(QByteArray data);
    void DisconnectPort();
    QByteArray ReadFromPort();
    QByteArray GetPid();

signals:
    void error_(QString err);
};

#endif // MYPORT

