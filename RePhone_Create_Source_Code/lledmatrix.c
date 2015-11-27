
#include "vmtype.h"
#include "vmlog.h"
//#include "ldlci2cV1.h"
#include "ldlci2cV2.h"
#include "lledmatrix.h"
#include "ldlcgpio.h"


static void delay_ms(VMUINT32 millisecs)
{
   VMUINT32 timeStop;
   VMUINT32 timeStart;
   VMUINT32 Freq = 0;
   volatile VMUINT32 i;
   millisecs = millisecs*1000;

    timeStart = vm_time_ust_get_count();
    while( Freq  < millisecs)
    {
    	for(i=0;i<5000;i++){}
        timeStop = vm_time_ust_get_count();
        Freq = timeStop - timeStart + 1;
    }
}

void led_matrix_power_down()
{
	dlc_i2c_configure(LEDAddress, 100);
	dlc_i2c_send_byte(POWER_DOWN);
}

void led_matrix_power_wake()
{
	pinMode(LED_MATRIX_WAKE_PIN,OUTPUT);
	digitalWrite(LED_MATRIX_WAKE_PIN,0);
	delay_ms(1);
	digitalWrite(LED_MATRIX_WAKE_PIN,1);
}

VMUINT8 led_matrix_check_on_line()
{
	VMUINT8 DataBuf[4] = {0};

	dlc_i2c_configure(LEDAddress, 100);

	DataBuf[0] = dlc_i2c_receive_byte(NULL);
	DataBuf[1] = dlc_i2c_receive_byte(NULL);
	DataBuf[2] = dlc_i2c_receive_byte(NULL);
	DataBuf[3] = dlc_i2c_receive_byte(NULL);

	//m_log_info("led matrix check data is %x %x %x %x",DataBuf[0],DataBuf[1],DataBuf[2],DataBuf[3]);

	if(DataBuf[3] == LEDAddress)
	{
		//vm_log_info("led matrix is on line.");
		return TRUE;
	}
	else
	{
		//vm_log_info("led matrix is not on line.");
		return FALSE;
	}
}

void led_matrix_disp_string(VMINT8 uData[],VMUINT8 uDataLength,VMUINT16 uTime)
{
	///*
	VMINT8 buffer[20] = {0};
	VMUINT8 i;
	if(uDataLength > 4)return;
	buffer[0] = uDataLength;
	for(i=0;i<uDataLength;i++)buffer[i + 1] = *(uData ++);
	buffer[i+1] = uTime >> 8;
	buffer[i+2] = uTime;

	dlc_i2c_configure(LEDAddress, 100);
	dlc_i2c_send(DISP_STRING, buffer, uDataLength + 3);
	//*/
	/*
	masterBegin();
	beginTransmission(LEDAddress);
	write(DISP_STRING);
	write(uDataLength);
	write_ext((VMINT8*)uData,uDataLength);
	write(uTime>>8); //high byte of time
	write(uTime);    //low byte of time
	endTransmission();
	*/
}

void led_matrix_disp_char(VMUINT8 uData,VMUINT16 uTime)
{
	///*
	VMUINT8 buffer[3] = {0};
	buffer[0] = uData;
	buffer[1] = uTime >> 8;
	buffer[2] = uTime;

	dlc_i2c_configure(LEDAddress, 100);
	dlc_i2c_send(DISP_CHAR_5X7, buffer, 3);
	//*/
	/*
	beginTransmission(LEDAddress);
	write(DISP_CHAR_5X7);
	write(uData);
	write(uTime>>8); //high byte of time
	write(uTime);    //low byte of time
	endTransmission();
	*/
}

void led_matrix_set_disp_orientation(VMUINT8 orientation)
{
	dlc_i2c_configure(LEDAddress, 100);
	dlc_i2c_send(SET_DISP_ORIENTATION, &orientation, 1);
}

void led_matrix_disp_pic(VMUINT8 uPicNum, VMUINT16 uTime)
{
	VMUINT8 buffer[3] = {0};
	buffer[0] = uPicNum;
	buffer[1] = uTime >> 8;
	buffer[2] = uTime;

	dlc_i2c_configure(LEDAddress, 100);
	dlc_i2c_send(DISP_PIC, buffer, 3);
}

void led_matrix_do_action(VMUINT32 *pdata)
{
    char ch = pdata[0];
    int  duration = pdata[1];
    led_matrix_disp_char(ch, duration);
}
