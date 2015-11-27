
#include "vmtype.h"
#include "vmlog.h"
#include "vmcmd.h"
#include "vmsystem.h"
#include "vmgraphic.h"
#include "vmgraphic_font.h"
#include "vmboard.h"
#include "ugui.h"
#include "lvoicecall.h"
#include "vmgsm_tel.h"
#include "vmgsm_sim.h"
#include "vmtimer.h"
#include "vmdatetime.h"
#include "sensor.h"
#include "vmtouch.h"
#include "ldlcgpio.h"
#include "ldlceint.h"
#include "lrgbws2812.h"
#include "lledmatrix.h"
#include "vmdcl_kbd.h"
#include "vmkeypad.h"
#include "sensor.h"
#include "ifttt_book.h"


#ifdef CUSTOM_TOUCHPAD
#include "tp_focaitech_ft6x06.h"
#else
#include "tp_goodix_gt9xx.h"
#endif

#define COMMAND_PORT  1000

#define RECORD_FLAG	1

#define RLED_GPIO 	17
#define GLED_GPIO 	15
#define BLED_GPIO 	12

VM_TIMER_ID_PRECISE sys_timer_id = 0;
static VMUINT8* g_font_pool;
VMUINT8 sys_record_time = 0;
unsigned char sys_blink = 0;


extern void gui_setup(void);
extern void windows_create(void);
extern void handle_touchevt(VM_TOUCH_EVENT event, VMINT x, VMINT y);
extern void screen_resume(void);
extern void lcd_backlight_level(VMUINT32 ulValue);


void sys_gpio_init(void){
	pinMode(RLED_GPIO,OUTPUT);
	pinMode(GLED_GPIO,OUTPUT);
	pinMode(BLED_GPIO,OUTPUT);

	pinMode(10, OUTPUT);
	pinMode(11, OUTPUT);

	digitalWrite(RLED_GPIO, 0);
	digitalWrite(GLED_GPIO, 1);
	digitalWrite(BLED_GPIO, 0);

	digitalWrite(10, 0);
	digitalWrite(11, 0);
}

void key_init(void){
    VM_DCL_HANDLE kbd_handle;
    vm_dcl_kbd_control_pin_t kbdmap;

    kbd_handle = vm_dcl_open(VM_DCL_KBD,0);
    kbdmap.col_map = 0x09;
    kbdmap.row_map =0x05;
    vm_dcl_control(kbd_handle,VM_DCL_KBD_COMMAND_CONFIG_PIN, (void *)(&kbdmap));

    vm_dcl_close(kbd_handle);
}

static void font_init(void){
    VM_RESULT result;
    VMUINT32 pool_size;

    result = vm_graphic_get_font_pool_size(0, 0, 0, &pool_size);
    if(VM_IS_SUCCEEDED(result))
    {
        g_font_pool = (VMUINT8* )vm_malloc(pool_size);
        if(NULL != g_font_pool)
        {
            vm_graphic_init_font_pool(g_font_pool, pool_size);
        } else {
            vm_log_info("allocate font pool memory failed, pool size: %d", pool_size);
        }
    }
    else
    {
        vm_log_info("get font pool size failed, result:%d", result);
    }
}


#ifdef CUSTOM_TOUCHPAD
void handle_touch()
{
    static uint8_t is_pressed = 0;

    vm_drv_tp_multiple_event_t tpdata = {0};
    ctp_focaitech_ft6x06_get_data(&tpdata);

    if (!(tpdata.points[0].x == 0 && tpdata.points[0].y == 0)) {
        handle_touchevt(VM_TOUCH_EVENT_TAP, tpdata.points[0].x, tpdata.points[0].y);
    } else {
        handle_touchevt(VM_TOUCH_EVENT_RELEASE, tpdata.points[0].x, tpdata.points[0].y);
    }

    is_pressed = 1 - is_pressed;
}
#endif

void command_callback(vm_cmd_command_t *param, void *user_data)
{
    vm_log_info("cmd = %s", (char*)param->command_buffer);

    if(strcmp("LOFF",(char*)param->command_buffer) == 0)
    {
    	lcd_backlight_level(2);
    }
    else if(strcmp("LON",(char*)param->command_buffer) == 0)
    {
    	lcd_backlight_level(50);
    }
    else if(strcmp("BUTTON",(char*)param->command_buffer) == 0)
    {
        sensor_t *button;
        screen_resume();
        button = sensor_find(BUTTON_ID);
        button->u32 = 1;
        ifttt_check();
        button->u32 = 0;
    }
}

void sys_timer_callback(VM_TIMER_ID_PRECISE sys_timer_id, void* user_data)
{
    sensor_update();

    vm_gsm_tel_set_volume(VM_AUDIO_VOLUME_6);

    if(RECORD_FLAG == 1)
    {
		if((sys_record_time ++) >= 60)
		{
			vm_date_time_t data_time;
			VMUINT8 str[25];
			sys_record_time = 0;
			vm_time_get_date_time(&data_time);
			sprintf(str, "%d-%d-%d, %d:%d:%d \r\n", data_time.year, data_time.month, data_time.day, data_time.hour, data_time.minute, data_time.second);
			file_write("system_record.txt", str, 0);
		}
    }

    if(sys_blink)digitalWrite(GLED_GPIO, 1);
    else digitalWrite(GLED_GPIO, 0);
    sys_blink = !sys_blink;
}

VMINT handle_keypad_event(VM_KEYPAD_EVENT event, VMINT code){
    /* output log to monitor or catcher */
    vm_log_info("key event=%d,key code=%d",event,code); /* event value refer to VM_KEYPAD_EVENT */

    if (code == 30) {
        if (event == 3) {   // long pressed
            // turn off peripheral
            led_matrix_disp_pic(7, 0);
        } else if (event == 2) { // down
            sensor_t *button;

            screen_resume();

            button = sensor_find(BUTTON_ID);
            button->u32 = 1;
            ifttt_check();
            button->u32 = 0;
        } else if (event == 1) { // up

        }
    }
    return 0;
}

void handle_sysevt(VMINT message, VMINT param)
{
    switch (message) {
        case VM_EVENT_CREATE:

        	vm_log_info("Open AT Port & Reg Callback");
        	vm_cmd_open_port(COMMAND_PORT, command_callback, NULL);

            lcd_backlight_level(50);
            gui_setup();

#ifdef CUSTOM_TOUCHPAD
            attachInterrupt(25, handle_touch, CHANGE);

#endif
            sensor_scan();
            //sys_timer_id = vm_timer_create_precise(1000, sys_timer_callback, NULL);
            sys_timer_id = vm_timer_create_non_precise(1000, sys_timer_callback, NULL);
            break;
        case VM_EVENT_PAINT:
            windows_create();
            digitalWrite(RLED_GPIO, 1);
            digitalWrite(BLED_GPIO, 1);
            break;
        case VM_EVENT_QUIT:
            break;
    }
}

/* Entry point */
void vm_main(void)
{
	sys_gpio_init();
    font_init();
    lcd_st7789s_init();
#ifdef CUSTOM_TOUCHPAD
    tp_ft6x06_init();
#else
    tp_gt9xx_init();
    vm_touch_register_event_callback(handle_touchevt);
#endif
    key_init();
    vm_keypad_register_event_callback(handle_keypad_event);
    //rgb_ws2812_power_on();

    ifttt_book_open();
    if(RECORD_FLAG == 1)file_open("system_record.txt");

    /* register system events handler */
    vm_pmng_register_system_event_callback(handle_sysevt);
}
