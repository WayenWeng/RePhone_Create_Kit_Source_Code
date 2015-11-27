
#include "vmtype.h"
#include "vmlog.h"
#include "ldlci2cV2.h"
#include "lmotor.h"

uint8_t motor_check_on_line()
{
	uint8_t DataBuf[4] = {0};

	dlc_i2c_configure(MOTOR_Address, 100);
	dlc_i2c_receive(0, DataBuf, 4);

	vm_log_info("motor check data is %x %x %x %x",DataBuf[0],DataBuf[1],DataBuf[2],DataBuf[3]);

	if(DataBuf[3] == MOTOR_Address)
	{
		vm_log_info("motor is on line.");
		return TRUE;
	}
	else
	{
		vm_log_info("motor is not on line.");
		return FALSE;
	}
}

void motor_set_work(uint16_t uiTime)
{
	uint8_t DataBuf[2] = {0};

	DataBuf[0] = ((uiTime >> 8) & 0xff);
	DataBuf[1] = (uiTime & 0xff);

	dlc_i2c_configure(MOTOR_Address, 100);
	dlc_i2c_send(MOTOR_SET_WORK, DataBuf, 2);
}

void motor_set_mode(uint8_t ucMode)
{
	dlc_i2c_configure(MOTOR_Address, 100);
	dlc_i2c_send(MOTOR_SET_MODE, &ucMode, 1);
}

void motor_do_action(uint32_t *pdata)
{
    motor_set_work(pdata[0]);
}
