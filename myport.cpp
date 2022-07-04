#include "myport.h"

void MyPort::SetSettings(QString name, int baudrate, int databits, int parity, int stopbits, int flowcontrol)
{
    PortSettings.name = name;
    PortSettings.baudrate = (QSerialPort::BaudRate)baudrate;
    PortSettings.databits = (QSerialPort::DataBits)databits;
    PortSettings.parity  = (QSerialPort::Parity)parity;
    PortSettings.stopbits = (QSerialPort::StopBits)stopbits;
    PortSettings.flowcontrol = (QSerialPort::FlowControl)flowcontrol;
}

bool MyPort::ConnectPort()
{
    sPort.setPortName(PortSettings.name);
    if(sPort.open(QIODevice::ReadWrite))
    {
        if(sPort.setBaudRate(PortSettings.baudrate)
                && sPort.setDataBits(PortSettings.databits)
                && sPort.setParity(PortSettings.parity)
                && sPort.setStopBits(PortSettings.stopbits)
                && sPort.setFlowControl(PortSettings.flowcontrol))
        {
            if(sPort.isOpen())
            {
                return 1;
//                Vse ok
            }
        }
        else
        {
            sPort.close();
            emit error_("Failed to apply port settings");
            return 0;
        }
    }
    else
    {
        sPort.close();
        emit error_("Failed to open port");
        return 0;
    }
}

void MyPort::WriteToPort(QByteArray data)
{
    if(sPort.isOpen())
    {
        sPort.clear();
        sPort.write(data);
        sPort.waitForBytesWritten(250);
    }
    else
    {
        emit error_("Failed to write Data");
    }
}

QByteArray MyPort::ReadFromPort()
{
    if(sPort.isOpen())
    {
        QByteArray data;
        while(sPort.waitForReadyRead(50))
            data += sPort.readAll();
        return data;
    }
    else
    {
        emit error_("Failed to read Data");
        return 0;
    }
}

QByteArray MyPort::GetPid()
{
    QSerialPortInfo *pInfo = new QSerialPortInfo(sPort);
    return QByteArray::number(pInfo->productIdentifier());
}

void MyPort::DisconnectPort()
{
    if(sPort.isOpen())
        sPort.close();
}







