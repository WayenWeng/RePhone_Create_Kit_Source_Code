
#include "ugui.h"
#include "vmgraphic.h"
#include "vmlog.h"
#include "lgsmsms.h"
#include "sms_book.h"
#include "sensor.h"

uint8_t g_sms_has_new_message = 0;

UG_WINDOW g_sms_inbox_window;
UG_WINDOW g_sms_info_window;
UG_WINDOW g_sms_new_window;
UG_WINDOW g_sms_sending_window;
UG_WINDOW *g_sms_new_window_prev = &g_sms_inbox_window;

uint8_t g_sms_inbox_first_visible = 0;
char g_sms_inbox_message[2][48] = {0,};
uint8_t g_sms_template_first_visible = 0;
uint8_t g_sms_select_message = 0;
char g_sms_new_message_info[16] = {0,};
char *g_sms_action_number = NULL;
char *g_sms_action_content = NULL;
extern UG_WINDOW g_home_window;

extern void contact_list_window_show();
extern void input_window_show(char *buf, int len, int id);
extern void screen_resume(void);

void sms_new_message_callback(char *number, char *content)
{
    screen_resume();
    
    g_sms_has_new_message = 1;
    UG_ButtonSetText(&g_home_window, 9, "K");


    strcpy(g_sms_new_message_info, number);
    sensor_t *sms_sensor = sensor_find(SMS_ID);
    sms_sensor->p = g_sms_new_message_info;
    ifttt_check();
    sms_sensor->p = NULL;

    ifttt_book_add(content);
}

void sms_send_message_callback(int result)
{
    char *msg;
    if (result == 1) {
        msg = "done";
    } else {
        msg = "failed";
    }

    UG_TextboxSetText(&g_sms_sending_window, 1, msg);
//    UG_ButtonShow(&g_sms_sending_window, 0);
}

void sms_inbox_window_update()
{
    int i;
    int number = gsm_sms_get_list_number();

    if (g_sms_has_new_message) {
        g_sms_has_new_message = 0;
        UG_ButtonSetText(&g_home_window, 9, NULL);
    }
    for (i = 0; i < 2; i++) {
        int index = i + g_sms_inbox_first_visible;
        if (index >= number) {
            g_sms_inbox_message[i][0] = '\0';
        } else {
            snprintf(g_sms_inbox_message[i],
                    sizeof(g_sms_inbox_message[i]),
                    "%s\n%s",
                    gsm_sms_remote_number(index),
                    gsm_sms_remote_content(index));
        }

        UG_ButtonSetText(&g_sms_inbox_window, i, g_sms_inbox_message[i]);
    }
}

void sms_inbox_window_callback(UG_MESSAGE *msg)
{
    int index;
    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            int index;
            int number = gsm_sms_get_list_number();
            switch (msg->sub_id) {
                case 0:
                case 1:
                    index = g_sms_inbox_first_visible + msg->sub_id;
                    if (index < number) {
                        sms_info_window_show(gsm_sms_remote_content(index));
                    }
                    break;
                case 2: // back
                    UG_WindowShow(&g_home_window);
                    break;
                case 3: // down
                    if (g_sms_inbox_first_visible < (number - 1)) {
                        g_sms_inbox_first_visible++;
                        sms_inbox_window_update();
                    }
                    break;
                case 4: // up
                    if (g_sms_inbox_first_visible > 0) {
                        g_sms_inbox_first_visible--;
                        sms_inbox_window_update();
                    }
                    break;
                case 5: // add
                    sms_new_window_show();
                    break;
                default:
                    break;
            }

        }
    }
}

void sms_inbox_window_create(void)
{
    static UG_BUTTON buttons[9];
    static UG_OBJECT objects[9];
    char *actions[] = { "1", "4", "3", "A" };
    int i = 0;
    int offset = 0;

    UG_WindowCreate(&g_sms_inbox_window, objects,
            sizeof(objects) / sizeof(*objects), sms_inbox_window_callback);
    UG_WindowSetStyle(&g_sms_inbox_window, WND_STYLE_2D);

    for (i = 0; i < 2; i++) {
        UG_ButtonCreate(&g_sms_inbox_window, buttons + i, i, 0, 80 * i + 40,
                239, 80 * (i + 1) + 40 - 1);
        UG_ButtonSetFont(&g_sms_inbox_window, i, &FONT_SIZE20);
        UG_ButtonSetStyle(&g_sms_inbox_window, i,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

        UG_ButtonSetAlignment(&g_sms_inbox_window, i, ALIGN_CENTER_LEFT);
        UG_ButtonSetHSpace(&g_sms_inbox_window, i, 8);
    }

    offset += 2;
    for (i = 0; i < 4; i++) {
        int index = offset + i;
        UG_ButtonCreate(&g_sms_inbox_window, buttons + index, index, 60 * i,
                200, 60 * i + 60 - 1, 239);
        UG_ButtonSetFont(&g_sms_inbox_window, index, &FONT_ICON24);
        UG_ButtonSetText(&g_sms_inbox_window, index, actions[i]);
        UG_ButtonSetStyle(&g_sms_inbox_window, index,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

    }

    offset += 4;
    UG_ButtonCreate(&g_sms_inbox_window, buttons + offset, offset, 0, 0, 239, 39);
    UG_ButtonSetFont(&g_sms_inbox_window, offset, &FONT_SIZE20);
    UG_ButtonSetText(&g_sms_inbox_window, offset, "Inbox");
    UG_ButtonSetStyle(&g_sms_inbox_window, offset,
            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_sms_inbox_window, offset, 0x000000);
}

void sms_info_window_callback(UG_MESSAGE *msg)
{
    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            switch (msg->sub_id) {
                case 0:
                    UG_WindowShow(&g_sms_inbox_window);
                    break;
                case 1:
                    sms_book_add(UG_ButtonGetText(&g_sms_info_window, 2));
                    UG_WindowShow(&g_sms_inbox_window);
                    break;
            }
        }
    }
}

void sms_info_window_create(void)
{
    static UG_BUTTON buttons[3];
    static UG_OBJECT objects[3];
    char *actions[] = { "1", "G" };
    int i = 0;

    UG_WindowCreate(&g_sms_info_window, objects,
            sizeof(objects) / sizeof(*objects), sms_info_window_callback);
    UG_WindowSetStyle(&g_sms_info_window, WND_STYLE_2D);

    for (i = 0; i < 2; i++) {
        int index = i;
        UG_ButtonCreate(&g_sms_info_window, buttons + index, index, 120 * i,
                200, 120 * i + 120 - 1, 239);
        UG_ButtonSetFont(&g_sms_info_window, index, &FONT_ICON24);
        UG_ButtonSetText(&g_sms_info_window, index, actions[i]);
        UG_ButtonSetStyle(&g_sms_info_window, index,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

    }

    UG_ButtonCreate(&g_sms_info_window, buttons + 2, 2, 10, 10, 229, 189);
    UG_ButtonSetFont(&g_sms_info_window, 2, &FONT_SIZE20);
    UG_ButtonSetStyle(&g_sms_info_window, 2,
            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_sms_info_window, 2, 0);
    UG_ButtonSetAlignment(&g_sms_info_window, 2, ALIGN_TOP_LEFT);
    UG_ButtonSetHSpace(&g_sms_info_window, 2, 8);
}

void sms_info_window_show(char *info)
{
    char *icon;
    if (UG_GetActiveWindow() == &g_sms_inbox_window) {
        icon = "G";
    } else {
        icon = "F";
    }
    UG_ButtonSetText(&g_sms_info_window, 1, icon);
    UG_ButtonSetText(&g_sms_info_window, 2, info);
    UG_WindowShow(&g_sms_info_window);
}

void sms_new_window_update()
{
    int i;
    int number = sms_get_max_item_num();
    for (i = 0; i < 4; i++) {
        char *message = NULL;
        char *icon = NULL;
        int index = i + g_sms_template_first_visible;
        if (index < number) {
            message = sms_get_item(index);
        }
        UG_ButtonSetText(&g_sms_new_window, 10 + i, message);

        if (index == g_sms_select_message) {
            icon = ":";
        }
        UG_ButtonSetText(&g_sms_new_window, 6 + i, icon);

    }
}

void sms_new_window_callback(UG_MESSAGE *msg)
{
    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            int number = sms_get_max_item_num();
            int select;
            switch (msg->sub_id) {
                case 0: // back
                    UG_WindowShow(g_sms_new_window_prev);
                    break;
                case 1: // down
                    if (g_sms_template_first_visible < (number - 1)) {
                        g_sms_template_first_visible++;
                        sms_new_window_update();
                    }
                    break;
                case 2: // up
                    if (g_sms_template_first_visible > 0) {
                        g_sms_template_first_visible--;
                        sms_new_window_update();
                    }
                    break;
                case 3: // send
                    if (g_sms_new_window_prev == &g_sms_inbox_window) {
                        gsm_sms_begin(UG_ButtonGetText(&g_sms_new_window, 5));
                        gsm_sms_send(sms_get_item(g_sms_select_message));

                        UG_TextboxSetText(&g_sms_sending_window, 1, NULL);
//                        UG_ButtonHide(&g_sms_sending_window, 0);
                        UG_WindowShow(&g_sms_sending_window);
                    } else {
                        strcpy(g_sms_action_number, UG_ButtonGetText(&g_sms_new_window, 5));
                        strcpy(g_sms_action_content, sms_get_item(g_sms_select_message));

                        UG_WindowShow(g_sms_new_window_prev);
                    }
                    break;
                case 4:
                case 5:
                    contact_list_window_show();
                    break;
                case 6:
                case 7:
                case 8:
                case 9:
                    select = msg->sub_id - 6 + g_sms_template_first_visible;
                    if (select < number) {
                        g_sms_select_message = select;
                        sms_new_window_update();
                    }
                    break;
                case 10:
                case 11:
                case 12:
                case 13:
                    select = msg->sub_id - 10 + g_sms_template_first_visible;
                    if (select < number) {
                        g_sms_select_message = select;
                        sms_new_window_update();
                    }
                    break;
            }
        }
    }
}

void sms_new_window_create(void)
{
    static UG_BUTTON buttons[14];
    static UG_OBJECT objects[14];
    char *actions[] = { "1", "4", "3", "F" };
    int i = 0;
    int index;

    UG_WindowCreate(&g_sms_new_window, objects,
            sizeof(objects) / sizeof(*objects), sms_new_window_callback);
    UG_WindowSetStyle(&g_sms_new_window, WND_STYLE_2D);

    for (i = 0; i < 4; i++) {
        index = i;
        UG_ButtonCreate(&g_sms_new_window, buttons + index, index, 60 * i,
                200, 60 * i + 60 - 1, 239);
        UG_ButtonSetFont(&g_sms_new_window, index, &FONT_ICON24);
        UG_ButtonSetText(&g_sms_new_window, index, actions[i]);
        UG_ButtonSetStyle(&g_sms_new_window, index,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

    }

    index = 4;
    UG_ButtonCreate(&g_sms_new_window, buttons + index, index, 0, 0, 39, 39);
    UG_ButtonSetFont(&g_sms_new_window, index, &FONT_SIZE20);
    UG_ButtonSetStyle(&g_sms_new_window, index,
            BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetText(&g_sms_new_window, index, "To");
    //UG_ButtonSetBackColor(&g_sms_new_window, index, 0);

    index = 5;
    UG_ButtonCreate(&g_sms_new_window, buttons + index, index, 40, 0, 239, 39);
    UG_ButtonSetFont(&g_sms_new_window, index, &FONT_SIZE20);
    UG_ButtonSetStyle(&g_sms_new_window, index,
            BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);
    //UG_ButtonSetBackColor(&g_sms_new_window, index, 0);



    for (i = 0; i < 4; i++) {
        index = 6 + i;
        UG_ButtonCreate(&g_sms_new_window, buttons + index, index, 0, 40 * i + 40, 39, 40 * i + 80 - 1);
        UG_ButtonSetFont(&g_sms_new_window, index, &FONT_ICON24);
        UG_ButtonSetStyle(&g_sms_new_window, index,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);
        UG_ButtonSetBackColor(&g_sms_new_window, index, 0xc44d58);

        index += 4;
        UG_ButtonCreate(&g_sms_new_window, buttons + index, index, 40, 40 * i + 40, 239, 40 * i + 80 - 1);
        UG_ButtonSetFont(&g_sms_new_window, index, &FONT_SIZE20);
        UG_ButtonSetStyle(&g_sms_new_window, index,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);
        UG_ButtonSetBackColor(&g_sms_new_window, index, 0xc44d58);
        UG_ButtonSetAlignment(&g_sms_new_window, index, ALIGN_CENTER_LEFT);
        UG_ButtonSetHSpace(&g_sms_new_window, index, 8);
    }
}

void sms_new_window_show()
{
    char *icon;

    g_sms_new_window_prev = UG_GetActiveWindow();
    if (g_sms_new_window_prev == &g_sms_inbox_window) {
        icon = "F";
    } else {
        icon = ":";
    }
    UG_ButtonSetText(&g_sms_new_window, 3, icon);

    g_sms_template_first_visible = 0;
    g_sms_select_message = 0;
    UG_ButtonSetText(&g_sms_new_window, 6, ":");
    sms_new_window_update();
    UG_WindowShow(&g_sms_new_window);
}

static void sms_sending_window_callback(UG_MESSAGE *msg)
{
    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            UG_WindowShow(UG_GetLastWindow());
        }
    }
}

void sms_sending_window_create(void)
{
    static UG_BUTTON button;
    static UG_TEXTBOX textboxs[2];
    static UG_OBJECT objects[3];

    UG_WindowCreate(&g_sms_sending_window, objects, 3, sms_sending_window_callback);

    UG_TextboxCreate(&g_sms_sending_window, textboxs, TXB_ID_0, 10, 10, 230, 80);
    UG_TextboxSetFont(&g_sms_sending_window, TXB_ID_0, &FONT_SIZE20);
    UG_TextboxSetText(&g_sms_sending_window, 0, "Sending   message");

    UG_TextboxCreate(&g_sms_sending_window, textboxs + 1, TXB_ID_1, 10, 90, 230,
            170);
    UG_TextboxSetFont(&g_sms_sending_window, TXB_ID_1, &FONT_SIZE20);

    UG_ButtonCreate(&g_sms_sending_window, &button, BTN_ID_0, 0, 180, 239, 239);
    UG_ButtonSetFont(&g_sms_sending_window, BTN_ID_0, &FONT_SIZE20);
    UG_ButtonSetText(&g_sms_sending_window, BTN_ID_0, "OK");
    UG_ButtonSetStyle(&g_sms_sending_window, BTN_ID_0,
            BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);
    //UG_ButtonSetBackColor(&g_sms_sending_window, BTN_ID_0, 0xf44336);
}

void sms_action_window_show(uint32_t *pdata)
{
    g_sms_action_number = (char *)pdata[0];
    g_sms_action_content = (char *)(pdata[0] + 16);

    sms_new_window_show();
}

void sms_do_action(uint32_t *pdata)
{
    char *number = (char *)pdata[0];
    char *content = (char *)(pdata[0] + 16);

    gsm_sms_begin(number);
    gsm_sms_send(content);
}



void sms_window_create()
{
    sms_inbox_window_create();
    sms_info_window_create();
    sms_new_window_create();
    sms_sending_window_create();

    sms_book_open();
    gsm_sms_set_new_message_callback(sms_new_message_callback);
    gsm_sms_set_send_message_callback(sms_send_message_callback);
}

void sms_window_show()
{
    sms_inbox_window_update();
    UG_WindowShow(&g_sms_inbox_window);
}
