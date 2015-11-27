
#ifndef _LDLCGPIO_H
#define _LDLCGPIO_H

#include "vmtype.h"

#define HIGH 0x1
#define LOW  0x0

#define INPUT 			0x0
#define OUTPUT 			0x1
#define INPUT_PULLUP 	0x2
#define INPUT_PULLDN 	0x3


VMINT8 pinMode(VMUINT8 ulPin,VMUINT8 ulMode);
VMINT8 digitalWrite(VMUINT8 ulPin,VMUINT8 ulData);
VMINT8 digitalRead(VMUINT8 ulPin);
VMINT32 analogRead(VMUINT8 ulPin);


#endif
