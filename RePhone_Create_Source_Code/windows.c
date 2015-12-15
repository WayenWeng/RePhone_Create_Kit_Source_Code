#include "ugui.h"
#include "vmgraphic.h"
#include "vmtimer.h"
#include "vmlog.h"
#include "lvoicecall.h"
#include "vmdatetime.h"
#include "sensor.h"
#include "actuator.h"
#include "laudio.h"
#include "battery.h"
#include "vmgsm_sim.h"
#include "vmfs.h"
#include "vmchset.h"
#include <stdlib.h>

char g_system_time_string[17] = {'2', '0', '0', '4', '-', '0', '1', '-', '0', '1', '\n', '0', '0', ':', '0', '0', 0 };
vm_date_time_t g_time_set;
char g_time_set_string[5][5] = {0,};
uint8_t g_time_set_focus = 4;

char *g_battery_info[] = { ";", "<", "=", ">", "?" }; // charging, empty, full, half, low
int g_battery_status = -1;

UG_WINDOW g_home_window;
//const char *g_entry_name[7] = { "sesnor", "action", "ifttt", "setting", "call", "sms", "12:00" };
const char *g_entry_name[7] = { "3", "6", "7", "2", "1", "5", "12:00" }; // icon font

UG_WINDOW g_time_window;



UG_WINDOW g_sensor_list_window;
char g_sensor_data_strings[4][32] = { 0, };
unsigned int g_sensor_first_visible = 0;

UG_WINDOW g_actuator_list_window;
char *g_actuator_name_strings[4][32] = { 0, };
unsigned int g_actuator_first_visible = 0;

extern UG_WINDOW g_music_window;
extern UG_WINDOW g_settings_window;
extern UG_WINDOW g_sms_inbox_window;
extern UG_WINDOW g_ifttt_list_window;

extern UG_WINDOW g_led_matrix_window;

extern void lcd_backlight_level(VMUINT32 ulValue);
extern char *itoa(int num, char* str, int radix);

extern void settings_window_create();
extern void call_window_create();
extern void sms_window_create();
extern void ifttt_window_create();
extern void input_window_create();
extern void led_matrix_window_create();
extern void input_window_show(int id);
extern void ws2812_window_create();
extern void vibrator_window_create(void);
extern void music_window_create(void);

extern void call_window_show();
extern void sms_window_show();

void time_update_callback(void)
{
    static vm_date_time_t last_time = { 0, };
    vm_date_time_t current_time;
    int battery_status;
    int level = 0;

    if (vm_time_get_date_time(&current_time) >= 0) {
        if (memcmp(&last_time, &current_time, sizeof(current_time)) != 0) {
            memcpy(&last_time, &current_time, sizeof(current_time));

            g_system_time_string[15] = '0' + (current_time.minute % 10);
            g_system_time_string[14] = '0' + ((current_time.minute / 10) % 6);

            g_system_time_string[12] = '0' + (current_time.hour % 10);
            g_system_time_string[11] = '0' + ((current_time.hour / 10) % 3);

            g_system_time_string[9] = '0' + (current_time.day % 10);
            g_system_time_string[8] = '0' + ((current_time.day / 10) % 10);

            g_system_time_string[6] = '0' + (current_time.month % 10);
            g_system_time_string[5] = '0' + ((current_time.month / 10) % 10);

            g_system_time_string[3] = '0' + (current_time.year % 10);
            g_system_time_string[2] = '0' + ((current_time.year / 10) % 10);
            g_system_time_string[1] = '0' + ((current_time.year /100) % 10);
            g_system_time_string[0] = '0' + ((current_time.year / 1000) % 10);

            UG_ButtonSetText(&g_home_window, 6, g_system_time_string);
        }
    }

    if (battery_is_charging()) {
        battery_status = 0;
    } else {
        level = battery_get_level();
        if (level < 25) {
            battery_status = 1;
        } else if (level > 75) {
            battery_status = 2;
        } else if (level >= 50) {
            battery_status = 3;
        } else {
            battery_status = 4;
        }
    }

    if (battery_status != g_battery_status) {
        g_battery_status = battery_status;
        UG_ButtonSetText(&g_home_window, 7, g_battery_info[battery_status]);
    }
}

static void home_window_callback(UG_MESSAGE *msg)
{
    /* output log to monitor or catcher */
    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            switch (msg->sub_id) {
                case 0: // sensor
                    UG_WindowShow(&g_sensor_list_window);
                    break;
                case 1: // actuator
                    UG_WindowShow(&g_actuator_list_window);
                    break;
                case 2: // ifttt
                    UG_WindowShow(&g_ifttt_list_window);
                    break;
                case 3:
                    UG_WindowShow(&g_settings_window);
                    break;
                case 4: // call
                    call_window_show();
                    break;
                case 5: // sms
                    sms_window_show();
                    break;
                case 6: // time
                    time_window_show();
                    break;
            }
        }
    }
}

static void home_window_create(void)
{
    static UG_BUTTON buttons[11];
    static UG_OBJECT objects[11];
    uint32_t colors[] = {0x00865a, 0x4edc4, 0xc7f464, 0xff6b6b, 0xc44d58, 0xf7cf10};
    int i, j;
    int index;

    UG_WindowCreate(&g_home_window, objects, sizeof(objects) / sizeof(*objects),
            home_window_callback);
    UG_WindowSetStyle(&g_home_window, WND_STYLE_2D);

    index = 0;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 3; j++) {
            UG_ButtonCreate(&g_home_window, buttons + index, index, 80 * j,
                    80 * i + 80, 80 * (j + 1) - 1, 80 * i + 160 - 1);
            UG_ButtonSetFont(&g_home_window, index, &FONT_ICON48);
            UG_ButtonSetText(&g_home_window, index, g_entry_name[index]);
            UG_ButtonSetBackColor(&g_home_window, index, colors[index]);
            UG_ButtonSetStyle(&g_home_window, index,
                    BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS
                            | BTN_STYLE_NO_BORDERS);

            index++;
        }
    }

    UG_ButtonCreate(&g_home_window, buttons + index, index, 40, 24, 199, 79);
    UG_ButtonSetFont(&g_home_window, index, &FONT_SIZE20);
    UG_ButtonSetStyle(&g_home_window, index,
            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_home_window, index, 0);

    index++;

    UG_ButtonCreate(&g_home_window, buttons + index, index, 200, 0, 239, 26);
    UG_ButtonSetFont(&g_home_window, index, &FONT_ICON24);
    UG_ButtonSetStyle(&g_home_window, index,
            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_home_window, index, 0);

    index++;

    UG_ButtonCreate(&g_home_window, buttons + index, index, 0, 0, 70, 26);
    UG_ButtonSetFont(&g_home_window, index, &FONT_SIZE20);
    UG_ButtonSetStyle(&g_home_window, index,
            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_home_window, index, 0);

    if (!vm_gsm_sim_has_card()) {
        UG_ButtonSetText(&g_home_window, index, "no sim");
    }

    index++;

    if (vm_gsm_sim_has_card()) {
        UG_ButtonCreate(&g_home_window, buttons + index, index, 0, 0, 32, 26);
        UG_ButtonSetFont(&g_home_window, index, &FONT_ICON24);
        UG_ButtonSetStyle(&g_home_window, index,
                BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
        UG_ButtonSetBackColor(&g_home_window, index, 0);

        UG_ButtonSetText(&g_home_window, index, NULL);
    }
    time_update_callback();

}

void time_window_increase()
{
    int id = g_time_set_focus;

    if (id == 4) {
        if (g_time_set.hour < 23) {
            g_time_set.hour++;
        } else {
            g_time_set.hour = 0;
        }
    } else if (id == 6) {
        if (g_time_set.minute < 59) {
            g_time_set.minute++;
        } else {
            g_time_set.minute = 0;
        }
    }
    else if(id == 7)
    {
        g_time_set.year++;
        if((g_time_set.year % 400 == 0)||(g_time_set.year % 4 == 0)&&(g_time_set.year % 100 != 0))
        {
        	if(g_time_set.month == 2 && g_time_set.day > 29)
        	g_time_set.day = 29;
        }
        else
        {
        	if(g_time_set.month == 2 && g_time_set.day > 28)
        	g_time_set.day = 28;
        }
    }
    else if (id == 8)
    {
        if(g_time_set.month < 12)
        {
            g_time_set.month++;
        }
        else
        {
            g_time_set.month = 1;
        }
        if(g_time_set.month == 4 || g_time_set.month == 6 || g_time_set.month == 9 || g_time_set.month == 11)
        {
        	if(g_time_set.day == 31)
        	g_time_set.day = 30;
        }
        else if(g_time_set.month == 2)
        {
        	if((g_time_set.year % 400 == 0)||(g_time_set.year % 4 == 0)&&(g_time_set.year % 100 != 0))
			{
				if(g_time_set.day > 29)g_time_set.day = 29;
			}
        	else if(g_time_set.day > 28)g_time_set.day = 28;
        }
    }
    else if (id == 9)
    {
    	if(g_time_set.month == 1 || g_time_set.month == 3 || g_time_set.month == 5 || g_time_set.month == 7 || g_time_set.month == 8 || g_time_set.month == 10 || g_time_set.month == 12)
    	{
			if(g_time_set.day < 31)
			{
				g_time_set.day++;
			}
			else
			{
				g_time_set.day = 1;
			}
    	}
    	else if(g_time_set.month == 4 || g_time_set.month == 6 || g_time_set.month == 9 || g_time_set.month == 11)
    	{
			if(g_time_set.day < 30)
			{
				g_time_set.day++;
			}
			else
			{
				g_time_set.day = 1;
			}
    	}
    	else
    	{
    		if((g_time_set.year % 400 == 0)||(g_time_set.year % 4 == 0)&&(g_time_set.year % 100 != 0))
    		{
    			if(g_time_set.day < 29)
    			{
    				g_time_set.day++;
    			}
    			else
    			{
    				g_time_set.day = 1;
    			}
    		}
    		else
    		{
    			if(g_time_set.day < 28)
    			{
    				g_time_set.day++;
    			}
    			else
    			{
    				g_time_set.day = 1;
    			}
    		}
    	}
    }
}

void time_window_descrease()
{
    int id = g_time_set_focus;
    if (id == 4) {
        if (g_time_set.hour > 0) {
            g_time_set.hour--;
        } else {
            g_time_set.hour = 23;
        }
    } else if (id == 6) {
        if (g_time_set.minute > 0) {
            g_time_set.minute--;
        } else {
            g_time_set.minute = 59;
        }
    }
    else if (id == 7)
    {
        g_time_set.year--;
        if(g_time_set.year < 2004)g_time_set.year = 2004;

        if((g_time_set.year % 400 == 0)||(g_time_set.year % 4 == 0)&&(g_time_set.year % 100 != 0))
        {
        	if(g_time_set.month == 2 && g_time_set.day > 29)
        	g_time_set.day = 29;
        }
        else
        {
        	if(g_time_set.month == 2 && g_time_set.day > 28)
        	g_time_set.day = 28;
        }
    }
    else if(id == 8)
    {
        if(g_time_set.month > 1)
        {
            g_time_set.month--;
        }
        else
        {
            g_time_set.month = 12;
        }
        if(g_time_set.month == 4 || g_time_set.month == 6 || g_time_set.month == 9 || g_time_set.month == 11)
		{
			if(g_time_set.day == 31)
			g_time_set.day = 30;
		}
		else if(g_time_set.month == 2)
		{
			if((g_time_set.year % 400 == 0)||(g_time_set.year % 4 == 0)&&(g_time_set.year % 100 != 0))
			{
				if(g_time_set.day > 29)g_time_set.day = 29;
			}
			else if(g_time_set.day > 28)g_time_set.day = 28;
		}
    }
    else if (id == 9)
    {
    	if(g_time_set.month == 1 || g_time_set.month == 3 || g_time_set.month == 5 || g_time_set.month == 7 || g_time_set.month == 8 || g_time_set.month == 10 || g_time_set.month == 12)
    	{
			if(g_time_set.day > 1)
			{
				g_time_set.day--;
			}
			else
			{
				g_time_set.day = 31;
			}
    	}
    	else if(g_time_set.month == 4 || g_time_set.month == 6 || g_time_set.month == 9 || g_time_set.month == 11)
    	{
			if(g_time_set.day > 1)
			{
				g_time_set.day--;
			}
			else
			{
				g_time_set.day = 30;
			}
    	}
    	else
    	{
    		if((g_time_set.year % 400 == 0)||(g_time_set.year % 4 == 0)&&(g_time_set.year % 100 != 0))
    		{
    			if(g_time_set.day > 1)
    			{
    				g_time_set.day--;
    			}
    			else
    			{
    				g_time_set.day = 29;
    			}
    		}
    		else
    		{
    			if(g_time_set.day > 1)
    			{
    				g_time_set.day--;
    			}
    			else
    			{
    				g_time_set.day = 28;
    			}
    		}
    	}
    }
}

static void time_window_callback(UG_MESSAGE *msg)
{
    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            switch (msg->sub_id) {
                case 0: // return
                    vm_time_set_date_time(&g_time_set);
                    UG_WindowShow(&g_home_window);
                    break;
                case 1: // +
                    time_window_increase();
                    time_window_update();
                    break;
                case 2: // -
                    time_window_descrease();
                    time_window_update();
                    break;

                case 4: // hour
                case 6: // minute
                case 7: // year
                case 8: // month
                case 9: // day
                    UG_ButtonSetForeColor(&g_time_window, g_time_set_focus, C_WHITE);
                    UG_ButtonSetBackColor(&g_time_window, g_time_set_focus, C_BUTTON_BC);

                    g_time_set_focus = msg->sub_id;
                    UG_ButtonSetForeColor(&g_time_window, g_time_set_focus, C_BUTTON_BC);
                    UG_ButtonSetBackColor(&g_time_window, g_time_set_focus, C_WHITE);
                    break;
                default:
                    break;
            }
        }
    }
}

static void time_window_create(void)
{
    static UG_BUTTON buttons[10];
    static UG_OBJECT objects[10];
    const char *button_icons[] = {"1", "5", "J"}; // <, +, -
    int id = 0;
    int i = 0;
    int j = 0;

    UG_WindowCreate(&g_time_window, objects, sizeof(objects) / sizeof(*objects), time_window_callback);
    UG_WindowSetStyle(&g_time_window, WND_STYLE_2D);

    for (i = 0; i < 3; i++) {
        id = i;
        UG_ButtonCreate(&g_time_window, buttons + id, id, 80 * i,
                200, 80 * i + 79, 239);
        UG_ButtonSetFont(&g_time_window, id, &FONT_ICON24);
        UG_ButtonSetText(&g_time_window, id,
                button_icons[id]);
        UG_ButtonSetStyle(&g_time_window, id,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS
                        | BTN_STYLE_NO_BORDERS);
    }

    id++;
    UG_ButtonCreate(&g_time_window, buttons + id, id, 0, 0, 239, 39);
    UG_ButtonSetFont(&g_time_window, id, &FONT_SIZE20);
    UG_ButtonSetText(&g_time_window, id, "Time & Date");
    UG_ButtonSetStyle(&g_time_window, id,
            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_time_window, id, 0);

    id++;
    UG_ButtonCreate(&g_time_window, buttons + id, id, 40, 40, 109, 79);
    UG_ButtonSetFont(&g_time_window, id, &FONT_SIZE20);
    UG_ButtonSetText(&g_time_window, id, "00");
    UG_ButtonSetStyle(&g_time_window, id,
            BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS
                        | BTN_STYLE_NO_BORDERS);

    id++;
    UG_ButtonCreate(&g_time_window, buttons + id, id, 110, 40, 129, 79);
    UG_ButtonSetFont(&g_time_window, id, &FONT_SIZE20);
    UG_ButtonSetText(&g_time_window, id, ":");
    UG_ButtonSetStyle(&g_time_window, id,
            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_time_window, id, 0);

    id++;
    UG_ButtonCreate(&g_time_window, buttons + id, id, 130, 40, 199, 79);
    UG_ButtonSetFont(&g_time_window, id, &FONT_SIZE20);
    UG_ButtonSetText(&g_time_window, id, "00");
    UG_ButtonSetStyle(&g_time_window, id,
            BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS
                        | BTN_STYLE_NO_BORDERS);

    id++;
    UG_ButtonCreate(&g_time_window, buttons + id, id, 10, 80, 109, 119);
    UG_ButtonSetFont(&g_time_window, id, &FONT_SIZE20);
    UG_ButtonSetText(&g_time_window, id, "2015");
    UG_ButtonSetStyle(&g_time_window, id,
            BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS
                        | BTN_STYLE_NO_BORDERS);

    id++;
    UG_ButtonCreate(&g_time_window, buttons + id, id, 120, 80, 169, 119);
    UG_ButtonSetFont(&g_time_window, id, &FONT_SIZE20);
    UG_ButtonSetText(&g_time_window, id, "06");
    UG_ButtonSetStyle(&g_time_window, id,
            BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS
                        | BTN_STYLE_NO_BORDERS);

    id++;
    UG_ButtonCreate(&g_time_window, buttons + id, id, 180, 80, 229, 119);
    UG_ButtonSetFont(&g_time_window, id, &FONT_SIZE20);
    UG_ButtonSetText(&g_time_window, id, "16");
    UG_ButtonSetStyle(&g_time_window, id,
            BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS
                        | BTN_STYLE_NO_BORDERS);

    UG_ButtonSetForeColor(&g_time_window, g_time_set_focus, 0x263238);
    UG_ButtonSetBackColor(&g_time_window, g_time_set_focus, C_WHITE);
}

void time_window_update()
{
    g_time_set_string[0][0] = '0' + (g_time_set.hour / 10) % 3;
    g_time_set_string[0][1] = '0' + (g_time_set.hour % 10);
    UG_ButtonSetText(&g_time_window, 4, g_time_set_string[0]);

    g_time_set_string[1][0] = '0' + (g_time_set.minute / 10) % 6;
    g_time_set_string[1][1] = '0' + (g_time_set.minute % 10);
    UG_ButtonSetText(&g_time_window, 6, g_time_set_string[1]);

    g_time_set_string[2][0] = '0' + (g_time_set.year / 1000) % 10;
    g_time_set_string[2][1] = '0' + (g_time_set.year / 100) % 10;
    g_time_set_string[2][2] = '0' + (g_time_set.year / 10) % 10;
    g_time_set_string[2][3] = '0' + (g_time_set.year % 10);
    UG_ButtonSetText(&g_time_window, 7, g_time_set_string[2]);

    g_time_set_string[3][0] = '0' + (g_time_set.month / 10) % 2;
    g_time_set_string[3][1] = '0' + (g_time_set.month % 10);
    UG_ButtonSetText(&g_time_window, 8, g_time_set_string[3]);

    g_time_set_string[4][0] = '0' + (g_time_set.day / 10) % 4;
    g_time_set_string[4][1] = '0' + (g_time_set.day % 10);
    UG_ButtonSetText(&g_time_window, 9, g_time_set_string[4]);
}

void time_window_show()
{
    vm_time_get_date_time(&g_time_set);

    time_window_update();

    UG_WindowShow(&g_time_window);
}



void sensor_list_window_update()
{
    int i;
    int sensor_number;

    if (UG_GetActiveWindow() != &g_sensor_list_window) {
        return;
    }

    sensor_number = sensor_get_number() - SENSOR_HIDDEN_NUMBER;
    
    for (i = 0; i < 4; i++) {
        int id = i + 4;
        int offset = i + g_sensor_first_visible;
        if (offset < sensor_number) {
            sensor_to_string(offset, g_sensor_data_strings[i],
                sizeof(g_sensor_data_strings[0]));
        } else {
            g_sensor_data_strings[i][0] = '\0';
        }
        UG_ButtonSetText(&g_sensor_list_window, id, g_sensor_data_strings[i]);
    }
}

void sensor_list_window_callback(UG_MESSAGE *msg)
{
    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            switch (msg->sub_id) {
                case 0:
                    break;
                case 1: // back
                    UG_WindowShow(&g_home_window);
                    break;
                case 2: // down
                {
                    int sensor_number = sensor_get_number() - SENSOR_HIDDEN_NUMBER;
                    if (g_sensor_first_visible < (sensor_number - 1)) {
                        g_sensor_first_visible++;
                        sensor_list_window_update();
                    }
                    break;
                }
                case 3: // up
                    if (g_sensor_first_visible > 0) {
                        g_sensor_first_visible--;
                        sensor_list_window_update();
                    }
                    break;
            }

        }
    }
}

void sensor_list_window_create(void)
{
    static UG_BUTTON buttons[13];
    static UG_OBJECT objects[13];
    char *actions[] = { "1", "4", "3" };
    int i = 0;

    UG_WindowCreate(&g_sensor_list_window, objects,
            sizeof(objects) / sizeof(*objects), sensor_list_window_callback);
    UG_WindowSetStyle(&g_sensor_list_window, WND_STYLE_2D);

    UG_ButtonCreate(&g_sensor_list_window, buttons, BTN_ID_0, 0, 0, 239, 39);
    UG_ButtonSetFont(&g_sensor_list_window, BTN_ID_0, &FONT_SIZE20);
    UG_ButtonSetText(&g_sensor_list_window, BTN_ID_0, "Sensors");
    UG_ButtonSetStyle(&g_sensor_list_window, BTN_ID_0,
            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_sensor_list_window, BTN_ID_0, 0x000000);

    for (i = 0; i < 3; i++) {
        UG_ButtonCreate(&g_sensor_list_window, buttons + 1 + i, i + 1, 80 * i,
                200, 80 * i + 80 - 1, 239);
        UG_ButtonSetFont(&g_sensor_list_window, i + 1, &FONT_ICON24);
        UG_ButtonSetText(&g_sensor_list_window, i + 1, actions[i]);
        UG_ButtonSetStyle(&g_sensor_list_window, i + 1,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

    }

    for (i = 0; i < 4; i++) {
        int offset = i + 4;
        UG_ButtonCreate(&g_sensor_list_window, buttons + offset, offset, 0,
                40 * i + 40, 239, 40 * (i + 1) + 40 - 1);
        UG_ButtonSetFont(&g_sensor_list_window, offset, &FONT_SIZE20);
        UG_ButtonSetStyle(&g_sensor_list_window, offset,
                BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);

        buttons[offset].align = ALIGN_CENTER_LEFT;

        UG_ButtonSetBackColor(&g_sensor_list_window, offset, 0);
        UG_ButtonSetHSpace(&g_sensor_list_window, offset, 8);
    }

    sensor_set_update_callback(sensor_list_window_update);
}

void actuator_list_window_update()
{
    int i;
    int actuator_number = actuator_get_number() - ACTUATOR_HIDDEN_NUMBER;
    for (i = 0; i < 4; i++) {
        int index = (i + g_actuator_first_visible);
        char *name = NULL;
        if (index < actuator_number) {
            name = actuator_get_name(index);
        }

        UG_ButtonSetText(&g_actuator_list_window, i + 4, name);
    }
}

void actuator_list_window_callback(UG_MESSAGE *msg)
{
    int actuator_index;
    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            int actuator_number = actuator_get_number() - ACTUATOR_HIDDEN_NUMBER;
            switch (msg->sub_id) {
                case 0:
                    break;
                case 1: // back
                    UG_WindowShow(&g_home_window);
                    break;
                case 2: // down
                    if (g_actuator_first_visible < (actuator_number - 1)) {
                        g_actuator_first_visible++;
                        actuator_list_window_update();
                    }
                    break;
                case 3: // up
                    if (g_actuator_first_visible > 0) {
                        g_actuator_first_visible--;
                        actuator_list_window_update();
                    }
                    break;
                default:
                    actuator_index = msg->sub_id - 4 + g_actuator_first_visible;
                    if (actuator_index < actuator_number) {
                        actuator_get_action_data(actuator_index, 0);
                    }
                    break;
            }

        }
    }
}

void actuator_list_window_create(void)
{
    static UG_BUTTON buttons[13];
    static UG_OBJECT objects[13];
    char *actions[] = { "1", "4", "3" };
    int i = 0;

    UG_WindowCreate(&g_actuator_list_window, objects,
            sizeof(objects) / sizeof(*objects), actuator_list_window_callback);
    UG_WindowSetStyle(&g_actuator_list_window, WND_STYLE_2D);

    UG_ButtonCreate(&g_actuator_list_window, buttons, BTN_ID_0, 0, 0, 239, 39);
    UG_ButtonSetFont(&g_actuator_list_window, BTN_ID_0, &FONT_SIZE20);
    UG_ButtonSetText(&g_actuator_list_window, BTN_ID_0, "Actions");
    UG_ButtonSetStyle(&g_actuator_list_window, BTN_ID_0,
            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_actuator_list_window, BTN_ID_0, 0x000000);

    for (i = 0; i < 3; i++) {
        UG_ButtonCreate(&g_actuator_list_window, buttons + 1 + i, i + 1, 80 * i,
                200, 80 * i + 80 - 1, 239);
        UG_ButtonSetFont(&g_actuator_list_window, i + 1, &FONT_ICON24);
        UG_ButtonSetText(&g_actuator_list_window, i + 1, actions[i]);
        UG_ButtonSetStyle(&g_actuator_list_window, i + 1,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

    }

    for (i = 0; i < 4; i++) {
        int offset = i + 4;
        UG_ButtonCreate(&g_actuator_list_window, buttons + offset, offset, 0,
                40 * i + 40, 239, 40 * (i + 1) + 40 - 1);
        UG_ButtonSetFont(&g_actuator_list_window, offset, &FONT_SIZE20);
        UG_ButtonSetStyle(&g_actuator_list_window, offset,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

        buttons[offset].align = ALIGN_CENTER_LEFT;

        // UG_ButtonSetBackColor(&g_actuator_list_window, offset, 0);

        UG_ButtonSetText(&g_actuator_list_window, offset,
                g_actuator_name_strings[i]);
        UG_ButtonSetHSpace(&g_actuator_list_window, offset, 8);
    }

    actuator_list_window_update();
}

void windows_create(void)
{
    home_window_create();
    time_window_create();
    call_window_create();
    sms_window_create();
    ifttt_window_create();
    sensor_list_window_create();
    actuator_list_window_create();
    settings_window_create();

    input_window_create();

    ws2812_window_create();
    led_matrix_window_create();
    vibrator_window_create();
    music_window_create();



    UG_WindowShow(&g_home_window);
}
