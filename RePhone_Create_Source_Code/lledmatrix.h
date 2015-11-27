

#ifndef _LLEDMATRIX_H
#define _LLEDMATRIX_H

#include "vmtype.h"

#define LEDAddress              33

#define DISP_CHAR_5X7	        0x80
#define DISP_STRING				0x81
#define SET_DISP_ORIENTATION    0x82
#define POWER_DOWN  			0x83
#define DISP_PIC				0x84

#define RIGHT_TO_LEFT 			0
#define LEFT_TO_RIGHT 			1

#define DIS_PIC_JOY_LVL0		0	// Joy level 0
#define DIS_PIC_JOY_LVL1		1	// Joy level 1
#define DIS_PIC_SAD_LVL0		2	// Sad level 0
#define DIS_PIC_SAD_LVL1		3	// Sad level 1
#define DIS_PIC_SAD_LVL2		4	// Sad level 2
#define DIS_PIC_SAD_LVL3		5	// Sad level 3
#define DIS_PIC_ALL				6	// Display all
#define DIS_PIN_NULL			7	// Display null

#define LED_MATRIX_WAKE_PIN 	13


VMUINT8 led_matrix_check_on_line();
void led_matrix_disp_string(VMINT8 uData[],VMUINT8 uDataLength,VMUINT16 uTime);
void led_matrix_disp_char(VMUINT8 uData,VMUINT16 uTime);
void led_matrix_set_disp_orientation(VMUINT8 orientation);
void led_matrix_disp_pic(VMUINT8 uPicNum, VMUINT16 uTime);
void led_matrix_power_down();
void led_matrix_power_wake();
void led_matrix_do_action(VMUINT32 *pdata);


#endif
