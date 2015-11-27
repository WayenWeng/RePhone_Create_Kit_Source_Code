
#include <stdint.h>
#include "ugui.h"


static int g_from_button_id = 0;
UG_WINDOW g_input_window;
static const char *g_input_button_text[15] = { "1", "2", "3", "4", "5", "6", "7", "8",
        "9", "-", "0", ".", 
        "1", ":", "7" // ICON FONT
        };
static char *g_input_string;
static uint8_t g_input_string_length = 0;
static uint8_t g_input_string_max_length = 0;

static void input_window_callback(UG_MESSAGE *msg)
{
    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            switch (msg->sub_id) {
                case 15: // text
                    break;
                case 12: // return
                    UG_WindowShow(UG_GetLastWindow());
                    break;
                case 13: // return string
                    UG_ButtonSetText(UG_GetLastWindow(), g_from_button_id, g_input_string);
                    UG_WindowShow(UG_GetLastWindow());
                    break;
                case 14: // delete
                    if (g_input_string_length > 0) {
                        g_input_string_length--;
                        g_input_string[g_input_string_length] =
                                '\0';
                        UG_ButtonSetText(&g_input_window, 15,
                                g_input_string);
                    }
                    break;
                default:
                    if (g_input_string_length
                            < (g_input_string_max_length - 1)) {
                        int index = msg->sub_id;
                        g_input_string[g_input_string_length] =
                                *g_input_button_text[index];
                        UG_ButtonSetText(&g_input_window, 15,
                                g_input_string);
                        g_input_string_length++;
                        g_input_string[g_input_string_length] =
                                '\0';
                    }
            }
        }
    }
}

void input_window_create(void)
{
    static UG_BUTTON buttons[16];
    static UG_OBJECT objects[16];
    int i = 0;
    int j = 0;

    UG_WindowCreate(&g_input_window, objects, 16, input_window_callback);
    UG_WindowSetStyle(&g_input_window, WND_STYLE_2D);

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 3; j++) {
            int index = i * 3 + j;
            UG_ButtonCreate(&g_input_window, buttons + index, index, 80 * j,
                    40 * (i + 1), 80 * (j + 1) - 1, 40 * (i + 2) - 1);
            UG_ButtonSetFont(&g_input_window, index, &FONT_SIZE20);
            UG_ButtonSetText(&g_input_window, index, g_input_button_text[index]);
            UG_ButtonSetStyle(&g_input_window, index,
                    BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS
                            | BTN_STYLE_NO_BORDERS);
        }
    }

    for (j = 0; j < 3; j++) {
        int index = 4 * 3 + j;
        UG_ButtonCreate(&g_input_window, buttons + index, index, 80 * j,
                40 * (i + 1), 80 * (j + 1) - 1, 40 * (i + 2) - 1);
        UG_ButtonSetFont(&g_input_window, index, &FONT_ICON24);
        UG_ButtonSetText(&g_input_window, index, g_input_button_text[index]);
        UG_ButtonSetStyle(&g_input_window, index,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);
    }

    UG_ButtonCreate(&g_input_window, buttons + 15, 15, 0, 0,
            239, 39);
    UG_ButtonSetFont(&g_input_window, 15, &FONT_SIZE20);
    UG_ButtonSetText(&g_input_window, 15,
            g_input_string);
    UG_ButtonSetStyle(&g_input_window, 15,
            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_input_window, 15, 0);

    // UG_ButtonSetBackColor(&g_input_window, 13, 0x4CAF50);
}

void input_window_show(char *buf, int len, int id)
{
    g_from_button_id = id;
    
    g_input_string = buf;
    g_input_string_max_length = len;
    g_input_string_length = 0;
    UG_ButtonSetText(&g_input_window, 15, "");
    UG_WindowShow(&g_input_window);
}
