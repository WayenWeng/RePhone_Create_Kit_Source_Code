
#include <string.h>
#include "ugui.h"
#include "vmlog.h"
#include "address_book.h"

UG_WINDOW* g_prev_contact_window_ptr;
UG_WINDOW g_contact_list_window;
UG_WINDOW g_dial_window;

uint8_t g_contact_first_visible = 0;
char g_contact_items[4][40];
const char* g_dial_button_text[15] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "*", "0", "#", "1", "0", "7" };
char g_outgoing_number[16] = {
    0,
};
int g_outgoing_number_length = 11;

extern UG_WINDOW g_sms_info_window;
extern UG_WINDOW g_sms_new_window;
extern UG_WINDOW g_call_action_window;
extern UG_WINDOW g_call_condition_window;
extern char* g_sms_message;

extern void calling_window_show(char* phone_number);
extern void sms_template_window_show();

void contact_list_window_update()
{
    int i;

    int number = book_get_number();
    for(i = 0; i < 4; i++) {
        char* contact_string = NULL;
        unsigned index = i + g_contact_first_visible;
        if(index < number) {
            book_get_item(index, g_contact_items[i], sizeof(g_contact_items[i]));
            contact_string = g_contact_items[i];
        }

        UG_ButtonSetText(&g_contact_list_window, i, contact_string);
    }
}

void contact_list_window_callback(UG_MESSAGE* msg)
{
    if(msg->type == MSG_TYPE_OBJECT) {
        if(msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            unsigned index;
            int contact_number = book_get_number();

            switch(msg->sub_id) {
            case 0:
            case 1:
            case 2:
            case 3:
                index = msg->sub_id + g_contact_first_visible;
                if(index < contact_number) {
                    char* phone = book_get_item_number(index);
                    g_outgoing_number_length = strlen(phone);
                    strcpy(g_outgoing_number, phone);
                    if(g_prev_contact_window_ptr == &g_sms_new_window) {
                        UG_ButtonSetText(&g_sms_new_window, 5, g_outgoing_number);
                        UG_WindowShow(&g_sms_new_window);
                    } else if(g_prev_contact_window_ptr == &g_call_action_window) {
                        strcpy(UG_ButtonGetText(&g_call_action_window, 2), g_outgoing_number);
                        UG_WindowShow(&g_call_action_window);
                    } else if(g_prev_contact_window_ptr == &g_call_condition_window) {
                        strcpy(UG_ButtonGetText(&g_call_condition_window, 2), g_outgoing_number);
                        UG_WindowShow(&g_call_condition_window);
                    } else {
                        calling_window_show(g_outgoing_number);
                    }
                }
                break;
            case 4: // back
                UG_WindowShow(g_prev_contact_window_ptr);
                break;
            case 5: // down
                if(g_contact_first_visible < (contact_number - 1)) {
                    g_contact_first_visible++;
                    contact_list_window_update();
                }
                break;
            case 6: // up
                if(g_contact_first_visible > 0) {
                    g_contact_first_visible--;
                    contact_list_window_update();
                }
                break;
            case 7: // dial
                dial_window_show();
                break;
            default:
                break;
            }
        }
    }
}

void contact_list_window_create(void)
{
    static UG_BUTTON buttons[9];
    static UG_OBJECT objects[9];
    char* actions[] = { "1", "4", "3", "B" };
    int i = 0;

    UG_WindowCreate(&g_contact_list_window, objects, sizeof(objects) / sizeof(*objects), contact_list_window_callback);
    UG_WindowSetStyle(&g_contact_list_window, WND_STYLE_2D);

    for(i = 0; i < 4; i++) {
        UG_ButtonCreate(&g_contact_list_window, buttons + i, i, 0, 40 * i + 40, 239, 40 * (i + 1) + 40 - 1);
        UG_ButtonSetFont(&g_contact_list_window, i, &FONT_SIZE20);
        UG_ButtonSetStyle(&g_contact_list_window, i, BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

        UG_ButtonSetAlignment(&g_contact_list_window, i, ALIGN_CENTER_LEFT);
        UG_ButtonSetHSpace(&g_contact_list_window, i, 8);
    }

    for(i = 0; i < 4; i++) {
        int index = 4 + i;
        UG_ButtonCreate(&g_contact_list_window, buttons + index, index, 60 * i, 200, 60 * i + 60 - 1, 239);
        UG_ButtonSetFont(&g_contact_list_window, index, &FONT_ICON24);
        UG_ButtonSetText(&g_contact_list_window, index, actions[i]);
        UG_ButtonSetStyle(&g_contact_list_window, index, BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);
    }

    UG_ButtonCreate(&g_contact_list_window, buttons + 8, 8, 0, 0, 239, 39);
    UG_ButtonSetFont(&g_contact_list_window, 8, &FONT_SIZE20);
    UG_ButtonSetText(&g_contact_list_window, 8, "Contacts");
    UG_ButtonSetStyle(&g_contact_list_window, 8, BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_contact_list_window, 8, 0x000000);

    book_open();
    dial_window_create();
}

void contact_list_window_show()
{
    g_prev_contact_window_ptr = UG_GetActiveWindow();

    g_contact_first_visible = 0;
    contact_list_window_update();
    UG_WindowShow(&g_contact_list_window);
}

static void dial_window_callback(UG_MESSAGE* msg)
{
    if(msg->type == MSG_TYPE_OBJECT) {
        if(msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            switch(msg->sub_id) {
            case 15: // text
                break;
            case 12: // return
                UG_WindowShow(&g_contact_list_window);
                break;
            case 13: // call or send sms
                if(g_prev_contact_window_ptr == &g_sms_new_window) {
                    UG_ButtonSetText(&g_sms_new_window, 5, g_outgoing_number);
                    UG_WindowShow(&g_sms_new_window);
                } else if(g_prev_contact_window_ptr == &g_call_action_window) {
                    strcpy(UG_ButtonGetText(&g_call_action_window, 2), g_outgoing_number);
                    UG_WindowShow(&g_call_action_window);
                } else if(g_prev_contact_window_ptr == &g_call_condition_window) {
                    strcpy(UG_ButtonGetText(&g_call_condition_window, 2), g_outgoing_number);
                    UG_WindowShow(&g_call_condition_window);
                } else {
                    calling_window_show(g_outgoing_number);
                }
                break;
            case 14: // delete
                if(g_outgoing_number_length > 0) {
                    g_outgoing_number_length--;
                    g_outgoing_number[g_outgoing_number_length] = '\0';
                    UG_ButtonSetText(&g_dial_window, 15, g_outgoing_number);
                }
                break;
            default:
                if(g_outgoing_number_length < (sizeof(g_outgoing_number) - 1)) {
                    int index = msg->sub_id;
                    g_outgoing_number[g_outgoing_number_length] = *g_dial_button_text[index];
                    UG_ButtonSetText(&g_dial_window, 15, g_outgoing_number);
                    g_outgoing_number_length++;
                    g_outgoing_number[g_outgoing_number_length] = '\0';
                }
            }
        }
    }
}

void dial_window_create(void)
{
    static UG_BUTTON buttons[16];
    static UG_OBJECT objects[16];
    int i = 0;
    int j = 0;

    UG_WindowCreate(&g_dial_window, objects, 16, dial_window_callback);
    UG_WindowSetStyle(&g_dial_window, WND_STYLE_2D);

    for(i = 0; i < 4; i++) {
        for(j = 0; j < 3; j++) {
            int index = i * 3 + j;
            UG_ButtonCreate(
                &g_dial_window, buttons + index, index, 80 * j, 40 * (i + 1), 80 * (j + 1) - 1, 40 * (i + 2) - 1);
            UG_ButtonSetFont(&g_dial_window, index, &FONT_SIZE20);
            UG_ButtonSetText(&g_dial_window, index, g_dial_button_text[index]);
            UG_ButtonSetStyle(&g_dial_window, index, BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);
        }
    }

    for(j = 0; j < 3; j++) {
        int index = 4 * 3 + j;
        UG_ButtonCreate(
            &g_dial_window, buttons + index, index, 80 * j, 40 * (i + 1), 80 * (j + 1) - 1, 40 * (i + 2) - 1);
        UG_ButtonSetFont(&g_dial_window, index, &FONT_ICON24);
        UG_ButtonSetText(&g_dial_window, index, g_dial_button_text[index]);
        UG_ButtonSetStyle(&g_dial_window, index, BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);
    }

    UG_ButtonCreate(&g_dial_window, buttons + 15, 15, 0, 0, 239, 39);
    UG_ButtonSetFont(&g_dial_window, 15, &FONT_SIZE20);
    UG_ButtonSetText(&g_dial_window, 15, g_outgoing_number);
    UG_ButtonSetStyle(&g_dial_window, 15, BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_dial_window, 15, 0); // 0x37474f);

    // UG_ButtonSetBackColor(&g_dial_window, CALL_BTN_ID, 0x4CAF50);
}

void dial_window_show(void)
{
    char* icon;
    if(g_prev_contact_window_ptr == &g_sms_info_window) {
        icon = "E"; // sms
    } else if(g_prev_contact_window_ptr == &g_call_condition_window ||
              g_prev_contact_window_ptr == &g_call_action_window || g_prev_contact_window_ptr == &g_sms_new_window) {
        icon = ":"; // ok
    } else {
        icon = "0"; // call
    }
    UG_ButtonSetText(&g_dial_window, 13, icon);

    g_outgoing_number[0] = '\0';
    g_outgoing_number_length = 0;
    UG_ButtonSetText(&g_dial_window, 15, g_outgoing_number);

    UG_WindowShow(&g_dial_window);
}
