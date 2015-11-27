

#include "ugui.h"
#include "lledmatrix.h"

UG_WINDOW *g_led_matrix_window_prev;
UG_WINDOW g_led_matrix_window;
UG_WINDOW g_led_matrix_time_window;
const char *g_led_matrix_button_text[15] = { "0", "1", "2", "3", "4", "5", "6",
        "7", "8", "9", "a", "b"};
static char g_led_matrix_display_char = ' ';
static uint32_t *g_led_matrix_parameters = 0;
char g_led_matrix_time_str[8] = {0,};

extern void input_window_show(char *buf, int len, int id);

void led_matrix_time_window_create();

static void led_matrix_window_callback(UG_MESSAGE *msg)
{
    char ch;
    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            switch (msg->sub_id) {
                case 15: // text
                    break;
                case 14:
                    break;
                case 13:
                    break;
                case 12: // return
                    UG_WindowShow(g_led_matrix_window_prev);
                    break;
                default: {
                    ch = g_led_matrix_button_text[msg->sub_id][0];
                    g_led_matrix_display_char = ch;
                    UG_WindowShow(&g_led_matrix_time_window);
                }
            }
        }
    }
}

void led_matrix_window_create(void)
{
    static UG_BUTTON buttons[16];
    static UG_OBJECT objects[16];
    char *icons[] = {"1", "3", "4"};
    int i = 0;
    int j = 0;

    UG_WindowCreate(&g_led_matrix_window, objects, 16,
            led_matrix_window_callback);
    UG_WindowSetStyle(&g_led_matrix_window, WND_STYLE_2D);

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 3; j++) {
            int index = i * 3 + j;
            UG_ButtonCreate(&g_led_matrix_window, buttons + index, index,
                    80 * j, 40 * (i + 1), 80 * (j + 1) - 1, 40 * (i + 2) - 1);
            UG_ButtonSetFont(&g_led_matrix_window, index, &FONT_SIZE20);
            UG_ButtonSetText(&g_led_matrix_window, index,
                    g_led_matrix_button_text[index]);
            UG_ButtonSetStyle(&g_led_matrix_window, index,
                    BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS
                            | BTN_STYLE_NO_BORDERS);
        }
    }

    for (j = 0; j < 3; j++) {
        int index = 4 * 3 + j;
        UG_ButtonCreate(&g_led_matrix_window, buttons + index, index, 80 * j,
                40 * (i + 1), 80 * (j + 1) - 1, 40 * (i + 2) - 1);
        UG_ButtonSetFont(&g_led_matrix_window, index, &FONT_ICON24);
        UG_ButtonSetText(&g_led_matrix_window, index,
                icons[j]);
        UG_ButtonSetStyle(&g_led_matrix_window, index,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);
    }

    UG_ButtonCreate(&g_led_matrix_window, buttons + 15, 15, 0, 0, 239, 39);
    UG_ButtonSetFont(&g_led_matrix_window, 15, &FONT_SIZE20);
    UG_ButtonSetText(&g_led_matrix_window, 15, "led matrix");
    UG_ButtonSetStyle(&g_led_matrix_window, 15,
            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_led_matrix_window, 15, 0);

    led_matrix_time_window_create();
}

void led_matrix_window_show(uint32_t *pdata)
{
    g_led_matrix_parameters = pdata;
    g_led_matrix_window_prev = UG_GetActiveWindow();

    UG_WindowShow(&g_led_matrix_window);
}

void led_matrix_time_window_callback(UG_MESSAGE *msg)
{
    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            int duration;
            switch (msg->sub_id) {
                case 1:
                    input_window_show(g_led_matrix_time_str, sizeof(g_led_matrix_time_str), 1);
                    break;
                case 2:
                    UG_WindowShow(&g_led_matrix_window);
                    break;
                case 3:
                    duration = atoi(g_led_matrix_time_str);

                    if (g_led_matrix_parameters) {
                        g_led_matrix_parameters[0] = (unsigned)g_led_matrix_display_char;
                        g_led_matrix_parameters[1] = duration;
                    } else {
                        led_matrix_disp_char(g_led_matrix_display_char, duration);
                    }

                    UG_WindowShow(g_led_matrix_window_prev);
                    g_led_matrix_parameters = 0;
                    break;
            }

        }
    }
}

void led_matrix_time_window_create(void)
{
    static UG_BUTTON buttons[6];
    static UG_OBJECT objects[6];
    char *button_texts[] = { "Run Time (s)", ""};
    int offset;

    UG_WindowCreate(&g_led_matrix_time_window, objects,
            sizeof(objects) / sizeof(*objects), led_matrix_time_window_callback);
    UG_WindowSetStyle(&g_led_matrix_time_window, WND_STYLE_2D);


    for (offset = 0; offset < 2; offset++) {
        UG_ButtonCreate(&g_led_matrix_time_window, buttons + offset, offset, 0,
                40 * offset + 40, 239, 40 * offset + 80 - 1);
        UG_ButtonSetFont(&g_led_matrix_time_window, offset, &FONT_SIZE20);

        if (offset % 2) {
            UG_ButtonSetStyle(&g_led_matrix_time_window, offset,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);
        } else {
            UG_ButtonSetStyle(&g_led_matrix_time_window, offset,
                            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
            UG_ButtonSetBackColor(&g_led_matrix_time_window, offset, 0);
        }

        UG_ButtonSetAlignment(&g_led_matrix_time_window, offset, ALIGN_CENTER_LEFT);
        UG_ButtonSetHSpace(&g_led_matrix_time_window, offset, 8);

        UG_ButtonSetText(&g_led_matrix_time_window, offset,
                button_texts[offset]);
    }

    UG_ButtonCreate(&g_led_matrix_time_window, buttons + offset, offset, 0,
            200, 119, 239);
    UG_ButtonSetFont(&g_led_matrix_time_window, offset, &FONT_ICON24);
    UG_ButtonSetText(&g_led_matrix_time_window, offset, "1");
    UG_ButtonSetStyle(&g_led_matrix_time_window, offset,
            BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

    offset++;
    UG_ButtonCreate(&g_led_matrix_time_window, buttons + offset, offset, 120,
            200, 239, 239);
    UG_ButtonSetFont(&g_led_matrix_time_window, offset, &FONT_ICON24);
    UG_ButtonSetText(&g_led_matrix_time_window, offset, ":");
    UG_ButtonSetStyle(&g_led_matrix_time_window, offset,
            BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

    offset++;
    UG_ButtonCreate(&g_led_matrix_time_window, buttons + offset, offset, 0, 0, 239, 39);
    UG_ButtonSetFont(&g_led_matrix_time_window, offset, &FONT_SIZE20);
    UG_ButtonSetText(&g_led_matrix_time_window, offset, "LED Matrix");
    UG_ButtonSetStyle(&g_led_matrix_time_window, offset,
            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_led_matrix_time_window, offset, 0x000000);
}
