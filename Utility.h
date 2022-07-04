#ifndef UTILITY_H
#define UTILITY_H

#include <QWidget>
#include "CypressUsb.h"
#include "CANChannels.h"

bool USBCy_RW(QString cmd, QString &answer, CyDev *usb, CANChannels *uC);

#endif // UTILITY_H
