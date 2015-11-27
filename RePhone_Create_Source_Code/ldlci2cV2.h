#ifndef _LDLCI2CV2_H
#define _LDLCI2CV2_H

#include "vmtype.h"


//configure HW I2C parameters
void dlc_i2c_configure(VMUINT32 slave_addr, VMUINT32 speed);

// Send one byte from host to client
VMUINT8 dlc_i2c_send_byte(VMUINT8 ucData);

// Receive one byte form client to host
VMUINT8 dlc_i2c_receive_byte(VMUINT8 bAck);

// I2C send data fuction
VMUINT8 dlc_i2c_send(VMUINT8 ucBufferIndex, VMUINT8* pucData, VMUINT32 unDataLength);

// I2C receive data function
VMUINT8 dlc_i2c_receive(VMUINT8 ucBufferIndex, VMUINT8* pucData, VMUINT32 unDataLength);

// I2C send data for 16 bits address fuction
VMUINT8 dlc_i2c_send_ext(VMUINT16 ucBufferIndex, VMUINT8* pucData, VMUINT32 unDataLength);

// I2C receive data for 16 bits address function
VMUINT8 dlc_i2c_receive_ext(VMUINT16 ucBufferIndex, VMUINT8* pucData, VMUINT32 unDataLength);


#endif
