
#include <stdlib.h>
#include "vmdcl.h"
#include "vmdcl_pwm.h"
#include "laudio.h"
#include "lstorage.h"
#include "vmmemory.h"
#include "cjson.h"
#include "ugui.h"
#include "vmpwr.h"

#define SCREEN_OFF_TIME    600

UG_WINDOW g_settings_window;
uint8_t g_settings_brightness = 3;
uint8_t g_settings_sound = 3;
char g_settings_brightness_str[4] = {'3', 0, 0 };
char g_settings_sound_str[2] = { '3', 0 };
uint8_t g_settings_is_changed = 0;

uint8_t g_brightness_table[] = {2, 20, 40, 50, 60, 80, 100};
int g_screen_off_timeout = SCREEN_OFF_TIME;

extern UG_WINDOW g_home_window;


int screen_suspend(void)
{
    if (g_screen_off_timeout > 0) {
        g_screen_off_timeout--;
        if (g_screen_off_timeout == 0) {
            lcd_backlight_level(0);
            vm_pwr_lcd_sleep_in();
        } else {
        	return 0;
        }
    }

    return 1;
}

void screen_resume(void)
{
    if (g_screen_off_timeout == 0) {
    	vm_pwr_lcd_sleep_out();
        lcd_backlight_level(g_brightness_table[g_settings_brightness]);
    }
    
    g_screen_off_timeout = SCREEN_OFF_TIME;
}

void lcd_backlight_level(VMUINT32 ulValue)
{
    VM_DCL_HANDLE pwm_handle;
    vm_dcl_pwm_set_clock_t pwm_clock;
    vm_dcl_pwm_set_counter_threshold_t pwm_config_adv;
    vm_dcl_config_pin_mode(3, VM_DCL_PIN_MODE_PWM);
    pwm_handle = vm_dcl_open(PIN2PWM(3), vm_dcl_get_owner_id());
    vm_dcl_control(pwm_handle, VM_PWM_CMD_START, 0);
    pwm_config_adv.counter = 100;
    pwm_config_adv.threshold = ulValue;
    pwm_clock.source_clock = 0;
    pwm_clock.source_clock_division = 3;
    vm_dcl_control(pwm_handle, VM_PWM_CMD_SET_CLOCK, (void *) (&pwm_clock));
    vm_dcl_control(pwm_handle, VM_PWM_CMD_SET_COUNTER_AND_THRESHOLD,
            (void *) (&pwm_config_adv));
    vm_dcl_close(pwm_handle);
}

void settings_load()
{
    unsigned long size = 0;
    file_size("settings.txt", &size);
    if (size < 24) {
        return;
    }

    char *ptr = vm_malloc(size);
    if (NULL == ptr) {
        return;
    }

    file_read("settings.txt", ptr, size, 0);

    cJSON *root = cJSON_Parse(ptr);
    if (NULL == root) {
        vm_free(ptr);
        return;
    }

    g_settings_brightness = cJSON_GetObjectItem(root, "brightness")->valueint;
    g_settings_sound = cJSON_GetObjectItem(root, "sound")->valueint;

    if (g_settings_brightness > 6) {
        g_settings_brightness = 6;
    }
    snprintf(g_settings_brightness_str, sizeof(g_settings_brightness_str), "%d", g_settings_brightness);

    if (g_settings_sound > 6) {
        g_settings_sound = 6;
    }
    g_settings_sound_str[0] = '0' + g_settings_sound;

    lcd_backlight_level(g_brightness_table[g_settings_brightness]);
    audioSetVolume(g_settings_sound);

    UG_ButtonSetText(&g_settings_window, 7, g_settings_brightness_str);
    UG_ButtonSetText(&g_settings_window, 4, g_settings_sound_str);

    vm_free(ptr);
}

void settings_save()
{
    char buf[40];
    snprintf(buf, sizeof(buf), "{\"brightness\":%d, \"sound\":%d}", g_settings_brightness, g_settings_sound);

    file_delete("settings.txt");
    file_open("settings.txt");
    file_write("settings.txt", buf, 0);
}

static void settings_window_callback(UG_MESSAGE *msg)
{
    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            switch (msg->sub_id) {
                case 0: // back
                    if (g_settings_is_changed) {
                        g_settings_is_changed = 0;
                        settings_save();
                    }
                    UG_WindowShow(&g_home_window);
                    break;
                case 3: // -
                    if (g_settings_sound > 0) {
                        g_settings_is_changed = 1;
                        g_settings_sound--;
                        audioSetVolume(g_settings_sound);
                        g_settings_sound_str[0] = '0' + g_settings_sound;
                        UG_ButtonSetText(&g_settings_window, 4,
                                g_settings_sound_str);
                    }
                    break;
                case 5: // +
                    if (g_settings_sound < 6) {
                        g_settings_is_changed = 1;
                        g_settings_sound++;
                        audioSetVolume(g_settings_sound);
                        g_settings_sound_str[0] = '0' + g_settings_sound;
                        UG_ButtonSetText(&g_settings_window, 4,
                                g_settings_sound_str);
                    }
                    break;
                case 6: // -
                    if (g_settings_brightness > 0) {
                        g_settings_is_changed = 1;
                        g_settings_brightness--;
                        lcd_backlight_level(g_brightness_table[g_settings_brightness]);

                        itoa(g_settings_brightness, g_settings_brightness_str,
                                10);
                        UG_ButtonSetText(&g_settings_window, 7,
                                g_settings_brightness_str);
                    }
                    break;
                case 8: // +
                    if (g_settings_brightness < 6) {
                        g_settings_is_changed = 1;
                        g_settings_brightness++;
                        lcd_backlight_level(g_brightness_table[g_settings_brightness]);

                        itoa(g_settings_brightness, g_settings_brightness_str,
                                10);
                        UG_ButtonSetText(&g_settings_window, 7,
                                g_settings_brightness_str);
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

void settings_window_create(void)
{
    char *button_texts[] = { "1", //icon font
                "", "", "-", "3", "+", "-", "3", "+"};
    static UG_BUTTON buttons[16];
    static UG_OBJECT objects[16];
    int id = 0;
    int i = 0;
    int j = 0;

    UG_WindowCreate(&g_settings_window, objects, 16, settings_window_callback);
    UG_WindowSetStyle(&g_settings_window, WND_STYLE_2D);

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            id = i * 3 + j;
            UG_ButtonCreate(&g_settings_window, buttons + id, id, 80 * j,
                    200 - i * 80, 80 * j + 79, 239 - i * 80);
            UG_ButtonSetFont(&g_settings_window, id, &FONT_SIZE20);
            UG_ButtonSetText(&g_settings_window, id,
                    button_texts[id]);
            UG_ButtonSetStyle(&g_settings_window, id,
                    BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS
                            | BTN_STYLE_NO_BORDERS);
        }
    }

    UG_ButtonSetFont(&g_settings_window, 0, &FONT_ICON24);

    id++;
    UG_ButtonCreate(&g_settings_window, buttons + id, id, 0, 0, 239, 39);
    UG_ButtonSetFont(&g_settings_window, id, &FONT_SIZE20);
    UG_ButtonSetText(&g_settings_window, id, "Brightness");
    UG_ButtonSetStyle(&g_settings_window, id,
            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_settings_window, id, 0);

    id++;
    UG_ButtonCreate(&g_settings_window, buttons + id, id, 0, 80, 239, 119);
    UG_ButtonSetFont(&g_settings_window, id, &FONT_SIZE20);
    UG_ButtonSetText(&g_settings_window, id, "Sound");
    UG_ButtonSetStyle(&g_settings_window, id,
            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_settings_window, id, 0);

    settings_load();
}
