

#include "ugui.h"
#include "lrgbws2812.h"

UG_WINDOW g_ws2812_mode_window;
UG_WINDOW *g_ws2812_mode_window_prev;
UG_WINDOW g_ws2812_nt_window;
UG_WINDOW g_ws2812_color_window;
static uint32_t *g_ws2812_parameters = 0;
static uint32_t g_ws2812_parameters_ex[3];
static uint8_t g_ws2812_mode = 0;
static uint8_t g_ws2812_number = 0;
static char g_ws2812_number_str[4] = {0,};
static uint16_t g_ws2812_time = 0;
static char g_ws2812_time_str[8] = {0,};
static uint8_t g_ws2812_color[4] = {0,};
static char g_ws2812_color_str[3][4] = {0,};

extern void input_window_show(char *buf, int len, int id);

void ws2812_window_create()
{
    ws2812_mode_window_create();
    ws2812_nt_window_create();
    ws2812_color_window_create();
}

void ws2812_window_show(uint32_t *pdata)
{
    g_ws2812_parameters = pdata;

    g_ws2812_mode_window_prev = UG_GetActiveWindow();
    UG_WindowShow(&g_ws2812_mode_window);
}

void ws2812_mode_window_callback(UG_MESSAGE *msg)
{
    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            switch (msg->sub_id) {
                case 0:
                case 1:
                case 2:
                    g_ws2812_mode = msg->sub_id;
                    ws2812_nt_window_show();
                    break;
                case 3:
                    break;
                case 4:
                    UG_WindowShow(g_ws2812_mode_window_prev);
                    break;
            }

        }
    }
}

void ws2812_mode_window_create(void)
{
    static UG_BUTTON buttons[5];
    static UG_OBJECT objects[5];
    char *button_texts[] = { "Monochrome", "Marquee", "Rainbow", "Color Pixels", "1"};
    int offset;

    UG_WindowCreate(&g_ws2812_mode_window, objects,
            sizeof(objects) / sizeof(*objects), ws2812_mode_window_callback);
    UG_WindowSetStyle(&g_ws2812_mode_window, WND_STYLE_2D);

    
    for (offset = 0; offset < 3; offset++) {
        UG_ButtonCreate(&g_ws2812_mode_window, buttons + offset, offset, 0,
                40 * offset + 40, 239, 40 * (offset + 1) + 40 - 1);
        UG_ButtonSetFont(&g_ws2812_mode_window, offset, &FONT_SIZE20);
        UG_ButtonSetStyle(&g_ws2812_mode_window, offset,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

        UG_ButtonSetAlignment(&g_ws2812_mode_window, offset, ALIGN_CENTER_LEFT);
        UG_ButtonSetHSpace(&g_ws2812_mode_window, offset, 8);

        // UG_ButtonSetBackColor(&g_ws2812_mode_window, offset, 0);

        UG_ButtonSetText(&g_ws2812_mode_window, offset,
                button_texts[offset]);
    }
    
    UG_ButtonCreate(&g_ws2812_mode_window, buttons + offset, offset, 0, 0, 239, 39);
    UG_ButtonSetFont(&g_ws2812_mode_window, offset, &FONT_SIZE20);
    UG_ButtonSetText(&g_ws2812_mode_window, offset, button_texts[offset]);
    UG_ButtonSetStyle(&g_ws2812_mode_window, offset,
            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_ws2812_mode_window, offset, 0x000000);

    offset++;
    UG_ButtonCreate(&g_ws2812_mode_window, buttons + offset, offset, 0,
            200, 239, 239);
    UG_ButtonSetFont(&g_ws2812_mode_window, offset, &FONT_ICON24);
    UG_ButtonSetText(&g_ws2812_mode_window, offset, button_texts[offset]);
    UG_ButtonSetStyle(&g_ws2812_mode_window, offset,
            BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

}


void ws2812_nt_window_callback(UG_MESSAGE *msg)
{
    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            switch (msg->sub_id) {
                case 1:
                    input_window_show(g_ws2812_number_str, sizeof(g_ws2812_number_str), 1);
                    break;
                case 3:
                    input_window_show(g_ws2812_time_str, sizeof(g_ws2812_time_str), 3);
                    break;
                case 4:
                    UG_WindowShow(&g_ws2812_mode_window);
                    break;
                case 5:
                    g_ws2812_number = atoi(g_ws2812_number_str);
                    g_ws2812_time = atoi(g_ws2812_time_str);

                    if (g_ws2812_mode != 2) {
                        UG_WindowShow(&g_ws2812_color_window);
                    } else {
                        if (!g_ws2812_parameters) {
                            g_ws2812_parameters = g_ws2812_parameters_ex;
                        }
                        g_ws2812_parameters[0] = (g_ws2812_mode << 16) | g_ws2812_number;
                        g_ws2812_parameters[1] = g_ws2812_time;
                        rgb_ws2812_do_action(g_ws2812_parameters);

                        UG_WindowShow(g_ws2812_mode_window_prev);
                    }
                    break;
            }

        }
    }
}

void ws2812_nt_window_create(void)
{
    static UG_BUTTON buttons[6];
    static UG_OBJECT objects[6];
    char *button_texts[] = { "Number of lights", "", "Run Time(s)", ""};
    int offset;

    UG_WindowCreate(&g_ws2812_nt_window, objects,
            sizeof(objects) / sizeof(*objects), ws2812_nt_window_callback);
    UG_WindowSetStyle(&g_ws2812_nt_window, WND_STYLE_2D);

    
    for (offset = 0; offset < 4; offset++) {
        UG_ButtonCreate(&g_ws2812_nt_window, buttons + offset, offset, 0,
                40 * offset, 239, 40 * offset + 40 - 1);
        UG_ButtonSetFont(&g_ws2812_nt_window, offset, &FONT_SIZE20);

        if (offset % 2) {
            UG_ButtonSetStyle(&g_ws2812_nt_window, offset,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);
        } else {
            UG_ButtonSetStyle(&g_ws2812_nt_window, offset,
                            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
            UG_ButtonSetBackColor(&g_ws2812_nt_window, offset, 0);
        }

        UG_ButtonSetAlignment(&g_ws2812_nt_window, offset, ALIGN_CENTER_LEFT);
        UG_ButtonSetHSpace(&g_ws2812_nt_window, offset, 8);

        UG_ButtonSetText(&g_ws2812_nt_window, offset,
                button_texts[offset]);
    }
    
    UG_ButtonCreate(&g_ws2812_nt_window, buttons + offset, offset, 0,
            200, 119, 239);
    UG_ButtonSetFont(&g_ws2812_nt_window, offset, &FONT_ICON24);
    UG_ButtonSetText(&g_ws2812_nt_window, offset, "1");
    UG_ButtonSetStyle(&g_ws2812_nt_window, offset,
            BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

    offset++;
    UG_ButtonCreate(&g_ws2812_nt_window, buttons + offset, offset, 120,
            200, 239, 239);
    UG_ButtonSetFont(&g_ws2812_nt_window, offset, &FONT_ICON24);
    UG_ButtonSetStyle(&g_ws2812_nt_window, offset,
            BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);
}

void ws2812_nt_window_show()
{
    char *action;
    if (g_ws2812_mode != 2) {
        action = "2";
    } else {
        action = ":";
    }
    
    UG_ButtonSetText(&g_ws2812_nt_window, 5, action);
    UG_WindowShow(&g_ws2812_nt_window);
}


void ws2812_color_window_callback(UG_MESSAGE *msg)
{
    int i;
    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            switch (msg->sub_id) {
                case 0:
                case 1:
                case 2:
                    input_window_show(g_ws2812_color_str[msg->sub_id], sizeof(*g_ws2812_color_str), msg->sub_id);
                    break;
                case 3:
                case 4:
                case 5:
                    break;
                case 6:
                    break;
                case 7:
                    ws2812_nt_window_show();
                    break;
                case 8:
                    for (i = 0; i < 3; i++) {
                        g_ws2812_color[i] = atoi(g_ws2812_color_str[2 - i]);
                    }

                    if (!g_ws2812_parameters) {
                        g_ws2812_parameters = g_ws2812_parameters_ex;
                    }
                    g_ws2812_parameters[0] = (g_ws2812_mode << 16) | g_ws2812_number;
                    g_ws2812_parameters[1] = g_ws2812_time;
                    g_ws2812_parameters[2] = *(uint32_t *)g_ws2812_color;
                    rgb_ws2812_do_action(g_ws2812_parameters);
                    UG_WindowShow(g_ws2812_mode_window_prev);
                    break;
            }

        }
    }
}

void ws2812_color_window_create(void)
{
    static UG_BUTTON buttons[9];
    static UG_OBJECT objects[9];
    char *button_texts[] = { "R(0~255)", "G(0~255)", "B(0~255)"};
    int i;
    int offset;

    UG_WindowCreate(&g_ws2812_color_window, objects,
            sizeof(objects) / sizeof(*objects), ws2812_color_window_callback);
    UG_WindowSetStyle(&g_ws2812_color_window, WND_STYLE_2D);

    for (i = 0; i < 3; i++) {
        UG_ButtonCreate(&g_ws2812_color_window, buttons + 3 + i, 3 + i, 0,
                40 * i + 40, 119, 40 * (i + 1) + 40 - 1);
        UG_ButtonSetFont(&g_ws2812_color_window, 3 + i, &FONT_SIZE20);
        UG_ButtonSetText(&g_ws2812_color_window, 3 + i, button_texts[i]);
        UG_ButtonSetStyle(&g_ws2812_color_window, 3 + i,
                BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
        UG_ButtonSetBackColor(&g_ws2812_color_window, 3 + i, 0x000000);

        UG_ButtonCreate(&g_ws2812_color_window, buttons + i, i, 120, 40 * i + 40,
                239, 40 * (i + 1) + 40 - 1);
        UG_ButtonSetFont(&g_ws2812_color_window, i, &FONT_SIZE20);
        UG_ButtonSetStyle(&g_ws2812_color_window, i,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

        // UG_ButtonSetAlignment(&g_ws2812_color_window, i, ALIGN_CENTER_LEFT);
        // UG_ButtonSetHSpace(&g_ws2812_color_window, i, 8);
    }



    offset = 6;
    UG_ButtonCreate(&g_ws2812_color_window, buttons + offset, offset, 0, 0, 239,
            39);
    UG_ButtonSetFont(&g_ws2812_color_window, offset, &FONT_SIZE20);
    UG_ButtonSetText(&g_ws2812_color_window, offset, "Set color");
    UG_ButtonSetStyle(&g_ws2812_color_window, offset,
            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_ws2812_color_window, offset, 0x000000);
    
    offset++;
    UG_ButtonCreate(&g_ws2812_color_window, buttons + offset, offset, 0,
            200, 119, 239);
    UG_ButtonSetFont(&g_ws2812_color_window, offset, &FONT_ICON24);
    UG_ButtonSetText(&g_ws2812_color_window, offset, "1");
    UG_ButtonSetStyle(&g_ws2812_color_window, offset,
            BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);
    
    offset++;
    UG_ButtonCreate(&g_ws2812_color_window, buttons + offset, offset, 120, 200, 239,
            239);
    UG_ButtonSetFont(&g_ws2812_color_window, offset, &FONT_ICON24);
    UG_ButtonSetText(&g_ws2812_color_window, offset, ":");
    UG_ButtonSetStyle(&g_ws2812_color_window, offset,
            BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);
    // UG_ButtonSetBackColor(&g_ws2812_color_window, offset, 0x4CAF50);
}






