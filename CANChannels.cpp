#include <windows.h>

#include "CypressUsb.h"
#include "CANChannels.h"
#include <QDebug>
//#include <QtTest/QTest>



CANChannels::CANChannels(CyDev *Usb, int id, int tout)
{

	CANch = -1;
	FX2 = Usb;
	can_id = id;
	can_tout = tout;
}

CANChannels::~CANChannels(void)
{
	if (!Close()) {
		Reset();  // reset channel to close it
		Close();
	}
	return;
}

BOOL CANChannels::Open(void)
{
	int res;
	CHAR CANchan;
        res=FX2->VendRead((UCHAR*)&CANchan,1,0x42,0,can_id);
    if (CANchan<0 || res<0) { // can't open channel
		return false;
	}
	CANch = CANchan;
//        qDebug() << (uint)FX2->DeviceHandle();
	return true;
}

BOOL CANChannels::Close(void)
{
	int i;
	CHAR res;

	if (CANch<0) {
		return true; // channel is already closed
	}
    i=FX2->VendRead((UCHAR*)&res,1,0x43,0,CANch);
	if (res<0) { // cann't close channel
		return false;
	}
	CANch = -1;
	return true;
}

BOOL CANChannels::Reset(void)
{
	int i;
	UCHAR res;

	if (CANch<0) {
		return true; // channel is closed
	}
	i=FX2->VendRead(&res,1,0x44,0,CANch);
	return true;
}

BOOL CANChannels::Cmd(PUCHAR cmd,PUCHAR replybuf,int buflen)
{
	int i=can_tout;
	int tout=can_tout;

    FX2->VendWrite((UCHAR*)cmd,strlen((const char*)cmd)+1, 0x40, 0,CANch);
	do {
        Sleep(1); // data is not ready yet wait some time
//        QTest::qWait(5); //in ms
		i=FX2->VendRead(replybuf,buflen, 0x41, 0,CANch);
		if (i) break;
	} while (tout--);
	if (i<=0) {
		return false;
	}
	return true;
 }
