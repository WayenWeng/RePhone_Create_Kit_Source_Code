

#include "actuator.h"

extern void led_matrix_window_show(uint32_t *pdata);
extern void led_matrix_do_action(uint32_t *pdata);
extern void ws2812_window_show(uint32_t *pdata);
extern void rgb_ws2812_do_action(uint32_t *);
extern void music_window_show(uint32_t *pdata);
extern void music_do_action(uint32_t *pdata);
extern void call_action_window_show(uint32_t *pdata);
extern void call_do_action(uint32_t *pdata);
extern void sms_action_window_show(uint32_t *pdata);
extern void sms_do_action(uint32_t *pdata);

char *g_actuator_name_list[] = {"LED matrix", "color pixels", "music", "call", "SMS"};
actuator_pfunc_t g_actuator_get_data_function_list[] = {
    led_matrix_window_show,
    ws2812_window_show,
    music_window_show,
    call_action_window_show,
    sms_action_window_show
};

actuator_pfunc_t g_actuator_do_action_function_list[] = {
    led_matrix_do_action,
    rgb_ws2812_do_action,
    music_do_action,
    call_do_action,
    sms_do_action,
};


int actuator_get_number()
{
    return sizeof(g_actuator_name_list) / sizeof(*g_actuator_name_list);
}

char *actuator_get_name(int index)
{
    return g_actuator_name_list[index];
}

action_pfunc_t actuator_get_action_function(int index)
{
    return g_actuator_do_action_function_list[index];
}

int actuator_get_action_data(int index, uint32_t *pdata)
{
    actuator_pfunc_t pfunc = g_actuator_get_data_function_list[index];
    if (pfunc) {
        pfunc(pdata);
    }

    return 0;
}

char *actuator_get_action_name(actuator_pfunc_t pfunc)
{
    int i;
    for (i = 0; i < sizeof(g_actuator_do_action_function_list); i++) {
        if (pfunc == g_actuator_do_action_function_list[i]) {
            return g_actuator_name_list[i];
        }
    }

    return "";
}
