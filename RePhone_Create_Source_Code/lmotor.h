

#ifndef _LMOTOR_H
#define _LMOTOR_H


#include "stdint.h"


#define MOTOR_Address       34

#define MOTOR_SET_WORK   	0x80
#define MOTOR_SET_MODE  	0x81

#define MOTOR_MODE_SHORT        0x00
#define MOTOR_MODE_LONG         0x01
#define MOTOR_MODE_INTERVAL     0x02


uint8_t motor_check_on_line();
void motor_set_work(uint16_t uiTime);
void motor_set_mode(uint8_t ucMode);
void motor_do_action(uint32_t *pdata);


#endif
