

#include "ugui.h"
#include "lmotor.h"

UG_WINDOW g_vibrator_window;
UG_WINDOW *g_vibrator_window_prev;
static uint32_t *g_vibrator_parameters = 0;
static uint8_t g_vibrator_mode = 0;
static uint32_t g_vibrator_time = 0;
static char g_vibrator_time_str[8] = {0,};

extern void input_window_show(char *buf, int len, int id);


void vibrator_window_show(uint32_t *pdata)
{
    g_vibrator_parameters = pdata;

    g_vibrator_window_prev = UG_GetActiveWindow();
    UG_WindowShow(&g_vibrator_window);
}

void vibrator_window_callback(UG_MESSAGE *msg)
{
    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            switch (msg->sub_id) {
                case 1:
                    input_window_show(g_vibrator_time_str, sizeof(g_vibrator_time_str), 1);
                    break;
                case 2:
                    UG_WindowShow(g_vibrator_window_prev);
                    break;
                case 3:
                    g_vibrator_time = atoi(g_vibrator_time_str);

                    if (g_vibrator_parameters) {
                        g_vibrator_parameters[0] = g_vibrator_time;
                    } else {
                        motor_do_action(&g_vibrator_time);
                    }

                    UG_WindowShow(g_vibrator_window_prev);
                    break;
            }

        }
    }
}

void vibrator_window_create(void)
{
    static UG_BUTTON buttons[6];
    static UG_OBJECT objects[6];
    char *button_texts[] = { "Run Time (s)", ""};
    int offset;

    UG_WindowCreate(&g_vibrator_window, objects,
            sizeof(objects) / sizeof(*objects), vibrator_window_callback);
    UG_WindowSetStyle(&g_vibrator_window, WND_STYLE_2D);


    for (offset = 0; offset < 2; offset++) {
        UG_ButtonCreate(&g_vibrator_window, buttons + offset, offset, 0,
                40 * offset + 40, 239, 40 * offset + 80 - 1);
        UG_ButtonSetFont(&g_vibrator_window, offset, &FONT_SIZE20);

        if (offset % 2) {
            UG_ButtonSetStyle(&g_vibrator_window, offset,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);
        } else {
            UG_ButtonSetStyle(&g_vibrator_window, offset,
                            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
            UG_ButtonSetBackColor(&g_vibrator_window, offset, 0);
        }

        UG_ButtonSetAlignment(&g_vibrator_window, offset, ALIGN_CENTER_LEFT);
        UG_ButtonSetHSpace(&g_vibrator_window, offset, 8);

        UG_ButtonSetText(&g_vibrator_window, offset,
                button_texts[offset]);
    }

    UG_ButtonCreate(&g_vibrator_window, buttons + offset, offset, 0,
            200, 119, 239);
    UG_ButtonSetFont(&g_vibrator_window, offset, &FONT_ICON24);
    UG_ButtonSetText(&g_vibrator_window, offset, "1");
    UG_ButtonSetStyle(&g_vibrator_window, offset,
            BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

    offset++;
    UG_ButtonCreate(&g_vibrator_window, buttons + offset, offset, 120,
            200, 239, 239);
    UG_ButtonSetFont(&g_vibrator_window, offset, &FONT_ICON24);
    UG_ButtonSetText(&g_vibrator_window, offset, ":");
    UG_ButtonSetStyle(&g_vibrator_window, offset,
            BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

    offset++;
    UG_ButtonCreate(&g_vibrator_window, buttons + offset, offset, 0, 0, 239, 39);
    UG_ButtonSetFont(&g_vibrator_window, offset, &FONT_SIZE20);
    UG_ButtonSetText(&g_vibrator_window, offset, "Vibrator");
    UG_ButtonSetStyle(&g_vibrator_window, offset,
            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_vibrator_window, offset, 0x000000);
}
