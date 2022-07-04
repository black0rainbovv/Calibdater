#include "Utility.h"


bool USBCy_RW(QString cmd, QString &answer, CyDev *usb, CANChannels *uC)
{
    int i;
    static unsigned char buf[64];
    QByteArray str;
    bool sts = false;
    int res;                //
                            // -1 usb isn't open
                            // -2 uC isn't open
                            // -3 timeout    

    if(usb->IsOpen())
    {        
        if(uC->Open())
        {
            for(i=0; i<sizeof(buf); i++) buf[i] = '\0';

            str = cmd.toLatin1();
            if(!uC->Cmd((unsigned char*)str.constData(), buf, sizeof(buf))) res = -3;   // error -> timeout,communication_error
            else
            {
                answer = QString::fromLatin1((char*)buf,-1);                                // ok
                if(answer.count() > 3 && answer[2] == '>') answer.remove(0,3);             //
                answer = answer.simplified();                            // whitespace removed: '\t','\n','\v','\f','\r',and ' '
                sts = true;
            }

        }
        else res = -2;              // uC isn't open

        if(!uC->Close())            // Close uC
        {
            uC->Reset();
            uC->Close();
        }
    }
    else res = -1;                  // usb isn't open

    if(!sts) answer = QString::number(res);

    return(sts);
}
