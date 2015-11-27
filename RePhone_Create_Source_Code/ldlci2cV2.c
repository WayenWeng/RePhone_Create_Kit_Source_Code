
#include "vmdcl.h"
#include "vmlog.h"
#include "vmdcl_gpio.h"
#include "vmdcl_i2c.h"
#include "vmdatetime.h"
#include "ldlci2cV2.h"
#include "string.h"


VMUINT8 dlc_i2c_configure_done = FALSE;

VM_DCL_HANDLE dlc_i2c_handle;



void dlc_i2c_configure(VMUINT32 slave_addr, VMUINT32 speed)
{
	vm_dcl_i2c_control_config_t conf_data;

	if(!dlc_i2c_configure_done)
	{
		dlc_i2c_handle = vm_dcl_open(VM_DCL_I2C, 0);
	}

	conf_data.reserved_0 =  (VM_DCL_I2C_OWNER)0;
	conf_data.transaction_mode = VM_DCL_I2C_TRANSACTION_FAST_MODE;
	conf_data.get_handle_wait = TRUE;
	conf_data.reserved_1 = 0;
	conf_data.delay_length = 0;
	conf_data.slave_address = (slave_addr << 1);

	conf_data.fast_mode_speed = speed;
	conf_data.high_mode_speed = 0;
	vm_dcl_control(dlc_i2c_handle,VM_DCL_I2C_CMD_CONFIG,(void *)&conf_data);

	dlc_i2c_configure_done = TRUE;
}

VMUINT8 dlc_i2c_send_byte(VMUINT8 ucData)
{
	VMINT16 i;
	VMINT8 ret;
	VMUINT8 ucMask;
	VM_DCL_STATUS status = VM_DCL_STATUS_FAIL;
	vm_dcl_gpio_control_level_status_t  sda_read;
	vm_dcl_i2c_control_continue_write_t write;

	if(dlc_i2c_configure_done)
	{
		write.data_ptr = &ucData;
		write.data_length = 1;
		write.transfer_number = 1;
		status = vm_dcl_control(dlc_i2c_handle, VM_DCL_I2C_CMD_CONT_WRITE, (void *)&write);
	}

	ret = (status == VM_DCL_STATUS_OK)?TRUE:FALSE;

	return ret;
}

VMUINT8 dlc_i2c_receive_byte(VMUINT8 bAck)
{
	VMUINT8 ucRet = 0;
	VMINT16 i;
	vm_dcl_i2c_control_continue_read_t read;
	vm_dcl_gpio_control_level_status_t sda_read;
	VM_DCL_STATUS status;

	if(dlc_i2c_configure_done)
	{
		read.data_ptr = &ucRet;
		read.data_length = 1;
		read.transfer_number = 1;
		status = vm_dcl_control(dlc_i2c_handle, VM_DCL_I2C_CMD_CONT_READ, (void *)&read);
		if(status != VM_DCL_STATUS_OK)return FALSE;
	}

	return ucRet;
}

VMUINT8 dlc_i2c_send(VMUINT8 ucBufferIndex, VMUINT8* pucData, VMUINT32 unDataLength)
{
	VMUINT32 i;
	VMUINT8 write_buf[9];
	VMUINT8 bRet = TRUE;
	vm_dcl_i2c_control_continue_write_t write;
	VM_DCL_STATUS status;

	if(dlc_i2c_configure_done)
	{
		write_buf[0] = ucBufferIndex;
		for(i=0;i<unDataLength;i++)
		{
			write_buf[i+1] = *(pucData+i);
		}
		///*
		write.data_ptr = write_buf;
		write.data_length = unDataLength+1;
		write.transfer_number = 1;
		status = vm_dcl_control(dlc_i2c_handle, VM_DCL_I2C_CMD_CONT_WRITE, (void *)&write);
		if(status != VM_DCL_STATUS_OK)return FALSE;
		//*/
		/*
		for(i=0;i<(unDataLength + 1);i++)
		{
			status = dlc_i2c_send_byte(write_buf[i]);
		}
		*/
	}
	bRet = (status == VM_DCL_STATUS_OK)?TRUE:FALSE;
	return bRet;
}

VMUINT8 dlc_i2c_send_ext(VMUINT16 ucBufferIndex, VMUINT8* pucData, VMUINT32 unDataLength)
{
	VMUINT32 i;
	VMUINT8 write_buf[10];
	VMUINT8 bRet = TRUE;
	vm_dcl_i2c_control_continue_write_t write;
	VM_DCL_STATUS status;
	VMUINT8 addr_h = ( ucBufferIndex >> 8 )& 0xFF;
	VMUINT8 addr_l = ucBufferIndex&0xFF;
	VMUINT32 offset = 0;
	VMUINT8  pkt_len;

	if(dlc_i2c_configure_done)
	{
		while ( offset <= unDataLength )
		{
			write_buf[0] = ((ucBufferIndex + offset)>>8)&0xFF;
			write_buf[1] = (ucBufferIndex + offset)&0xFF;
		    if ( unDataLength - offset > 6 )
			{
				pkt_len = 6;
			}
			else
			{
				pkt_len = unDataLength - offset;
			}
			memcpy( &write_buf[2], &pucData[offset], pkt_len );
			offset += pkt_len;
			write.data_ptr = write_buf;
			write.data_length = pkt_len+2;
			write.transfer_number = 1;
			status = vm_dcl_control(dlc_i2c_handle, VM_DCL_I2C_CMD_CONT_WRITE, (void *)&write);

			if(status != VM_DCL_STATUS_OK)return FALSE;
			if ( offset == unDataLength ) break;
		}
	}

	return bRet;
}

VMUINT8 dlc_i2c_receive(VMUINT8 ucBufferIndex, VMUINT8* pucData, VMUINT32 unDataLength)
{
	VMUINT32 i;
	VM_DCL_STATUS dcl_i2c_ret;
	VMUINT8 bRet = TRUE;
	vm_dcl_i2c_control_write_and_read_t write_read;

	if(dlc_i2c_configure_done)
	{
		write_read.in_data_ptr = pucData;
		write_read.in_data_length = unDataLength;
		write_read.out_data_ptr = &ucBufferIndex;
		write_read.out_data_length = 1;
		dcl_i2c_ret = vm_dcl_control(dlc_i2c_handle, VM_DCL_I2C_CMD_WRITE_AND_READ, (void *)&write_read);
		if(dcl_i2c_ret != VM_DCL_STATUS_OK)
		{
			bRet = FALSE;
		}
	}

	return bRet;
}

VMUINT8 dlc_i2c_receive_ext(VMUINT16 ucBufferIndex, VMUINT8* pucData, VMUINT32 unDataLength)
{
	VMUINT32 i;
	VMUINT8 bRet = TRUE;
	vm_dcl_i2c_control_write_and_read_t write_read;
	VMUINT8 write_buf[2];
	VMUINT16 reg_addr = ucBufferIndex;
	VMUINT32 offset = 0;
	VMUINT8  pkt_len;
	VMUINT8 addr_h = ( ucBufferIndex >> 8 )& 0xFF;
	VMUINT8 addr_l = ucBufferIndex&0xFF;

	if(dlc_i2c_configure_done)
	{
		while ( offset < unDataLength )
		{
			write_buf[0] = ( reg_addr >> 8 )& 0xFF;
			write_buf[1] = reg_addr&0xFF;
			if ( unDataLength - offset > 8 )
			{
				pkt_len = 8;
			}
			else
			{
				pkt_len = unDataLength - offset;
			}
			write_read.in_data_ptr = pucData + offset;
			write_read.in_data_length = pkt_len;
			write_read.out_data_ptr = write_buf;
			write_read.out_data_length = 2;
			vm_dcl_control(dlc_i2c_handle, VM_DCL_I2C_CMD_WRITE_AND_READ, (void *)&write_read);
			offset += pkt_len;
			reg_addr = ucBufferIndex + offset;
		}
	}

	return bRet;
}

