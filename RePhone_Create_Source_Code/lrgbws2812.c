
#include "vmtype.h"
#include "vmlog.h"
#include "ldlci2cV2.h"
#include "lrgbws2812.h"


void rgb_ws2812_power_off()
{
	uint8_t ucData = RGB_POWER_OFF;
	dlc_i2c_configure(RGB_WS2812_Address, 100);
	dlc_i2c_send(RGB_SET_MODE, &ucData, 1);
}

void rgb_ws2812_power_on()
{
	uint8_t ucData = RGB_POWER_ON;
	dlc_i2c_configure(RGB_WS2812_Address, 100);
	dlc_i2c_send(RGB_SET_MODE, &ucData, 1);
}

uint8_t rgb_ws2812_check_on_line()
{
	uint8_t DataBuf[4] = {0};

	dlc_i2c_configure(RGB_WS2812_Address, 100);

	dlc_i2c_receive(0, DataBuf, 4);

	vm_log_info("rgb ws2812 check data is %x %x %x %x",DataBuf[0],DataBuf[1],DataBuf[2],DataBuf[3]);

	if(DataBuf[3] == RGB_WS2812_Address)
	{
		vm_log_info("rgb ws2812 is on line.");
		rgb_ws2812_power_on();
		return TRUE;
	}
	else
	{
		vm_log_info("rgb ws2812 is not on line.");
		return FALSE;
	}
}

void rgb_ws2812_set_pixel_color(uint32_t ulRGB, uint8_t ucNum)
{
	uint8_t DataBuf[4] = {0};

	dlc_i2c_configure(RGB_WS2812_Address, 100);

	DataBuf[0] = ucNum;
	DataBuf[1] = ((ulRGB >> 16) & 0xff);
	DataBuf[2] = ((ulRGB >> 8) & 0xff);
	DataBuf[3] = (ulRGB & 0xff);

	dlc_i2c_send(RGB_SET_WORK, DataBuf, 4);
}

void rgb_ws2812_monochrome(uint8_t ucNum, uint32_t ulRGBData, uint16_t uiTime)
{
	uint8_t DataBuf[7] = {0};

	DataBuf[0] = RGB_MONOCHROME;
	DataBuf[1] = ucNum;
	DataBuf[2] = (ulRGBData >> 16) & 0xff;
	DataBuf[3] = (ulRGBData >> 8) & 0xff;
	DataBuf[4] = ulRGBData & 0xff;
	DataBuf[5] = (uiTime >> 8) & 0xff;
	DataBuf[6] = uiTime & 0xff;

	dlc_i2c_configure(RGB_WS2812_Address, 100);
	dlc_i2c_send(RGB_SET_MODE, DataBuf, 7);
}

void rgb_ws2812_marquee(uint8_t ucNum, uint32_t ulRGBData, uint16_t uiTime)
{
	uint8_t DataBuf[7] = {0};

	DataBuf[0] = RGB_MARQUEE;
	DataBuf[1] = ucNum;
	DataBuf[2] = (ulRGBData >> 16) & 0xff;
	DataBuf[3] = (ulRGBData >> 8) & 0xff;
	DataBuf[4] = ulRGBData & 0xff;
	DataBuf[5] = (uiTime >> 8) & 0xff;
	DataBuf[6] = uiTime & 0xff;

	dlc_i2c_configure(RGB_WS2812_Address, 100);
	dlc_i2c_send(RGB_SET_MODE, DataBuf, 7);
}

void rgb_ws2812_rainbow(uint8_t ucNum, uint16_t uiTime)
{
	uint8_t DataBuf[4] = {0};

	DataBuf[0] = RGB_RAINBOW;
	DataBuf[1] = ucNum;
	DataBuf[2] = (uiTime >> 8) & 0xff;
	DataBuf[3] = uiTime & 0xff;

	dlc_i2c_configure(RGB_WS2812_Address, 100);
	dlc_i2c_send(RGB_SET_MODE, DataBuf, 4);
}

void rgb_ws2812_do_action(uint32_t *pdata)
{
    uint8_t mode = pdata[0] >> 16;
    uint8_t number = pdata[0] & 0xFF;
    uint16_t time = pdata[1];
    uint32_t color = pdata[2];

    vm_log_info("ws2812 - mode: %d, n: %d, time: %d (%d), color: 0x%X", mode, number, time, pdata[1], color);

    if (mode == 0) {
        rgb_ws2812_monochrome(number, color, time);
    } else if (mode == 1) {
        rgb_ws2812_marquee(number, color, time);
    } else {
        rgb_ws2812_rainbow(number, time);
    }
}

