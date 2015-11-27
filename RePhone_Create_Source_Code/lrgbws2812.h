

#ifndef _LRGBWS2812_H
#define _LRGBWS2812_H


#include "stdint.h"


#define RGB_WS2812_Address      35

#define RGB_SET_WORK 0x80
#define RGB_SET_MODE 0x81

#define RGB_POWER_OFF           0
#define RGB_POWER_ON            1
#define RGB_MONOCHROME          2
#define RGB_MARQUEE             3
#define RGB_RAINBOW             4


void rgb_ws2812_power_off();
void rgb_ws2812_power_on();
uint8_t rgb_ws2812_check_on_line();
void rgb_ws2812_set_pixel_color(uint32_t ulRGB, uint8_t ucNum);
void rgb_ws2812_monochrome(uint8_t ucNum, uint32_t ulRGBData, uint16_t uiTime);
void rgb_ws2812_marquee(uint8_t ucNum, uint32_t ulRGBData, uint16_t uiTime);
void rgb_ws2812_rainbow(uint8_t ucNum, uint16_t uiTime);
void rgb_ws2812_do_action(uint32_t *pdata);

#endif
