
#include "ugui.h"
#include "vmgraphic.h"
#include "vmlog.h"
#include "sensor.h"
#include "actuator.h"
#include "condition.h"
#include "ifttt.h"
#include "ifttt_book.h"

UG_WINDOW g_ifttt_list_window;
UG_WINDOW g_ifttt_info_window;
UG_WINDOW g_ifttt_this_window;
UG_WINDOW g_ifttt_expr_window;
UG_WINDOW g_ifttt_that_window;
UG_WINDOW g_sms_condition_window;
UG_WINDOW g_call_condition_window;
UG_WINDOW g_call_action_window;

uint8_t g_ifttt_changed = 0;

uint8_t g_ifttt_first_visible = 0;
uint8_t g_ifttt_current_item = 0;

int8_t g_condition_index_map[SENSOR_MAX_NUMBER];
uint8_t g_condition_first_visible = 0;
uint8_t g_current_condition_position = 0;
char g_condition_string[4][32] = { 0, };

int8_t g_action_index_map[ACTUATOR_MAX_NUMBER];
uint8_t g_action_first_visible = 0;
uint8_t g_current_action_position = 0;

const char *g_operator_list[] = { ">", "=", "<" };
uint8_t g_current_operator = 0;
char g_expr_input_string[16] = { 0, };

extern UG_WINDOW g_home_window;
extern UG_WINDOW g_led_matrix_window;
extern void input_window_show(char *buf, int len, int id);
extern void contact_list_window_show();
extern void calling_window_show(char *phone_number);
extern void sms_new_window_show();

void ifttt_list_window_update()
{
    int i;
    char *str;
    int ifttt_number = ifttt_get_number();
    for (i = 0; i < 4; i++) {
        unsigned index = i + g_ifttt_first_visible;
        if (index >= ifttt_number) {
            UG_ButtonSetText(&g_ifttt_list_window, i, "");
        } else {
            UG_ButtonSetText(&g_ifttt_list_window, i, ifttt_get_name(index));
        }
    }
}

void ifttt_list_window_callback(UG_MESSAGE *msg)
{
    int index;
    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            switch (msg->sub_id) {
                case 0:
                case 1:
                case 2:
                case 3:
                    index = msg->sub_id + g_ifttt_first_visible;
                    if (index < ifttt_get_number()) {
                        g_ifttt_current_item = index;
                        ifttt_info_window_show(ifttt_get_description(index));
                    }
                    break;
                case 4: // back
                    if (g_ifttt_changed) {
                        g_ifttt_changed = 0;
                        ifttt_book_save();
                    }
                    UG_WindowShow(&g_home_window);
                    break;
                case 5: // down
                    g_ifttt_first_visible++;
                    ifttt_list_window_update();
                    break;
                case 6: // up
                    if (g_ifttt_first_visible > 0) {
                        g_ifttt_first_visible--;
                        ifttt_list_window_update();
                    }
                    break;
                case 7: // add
                    if (ifttt_get_number() < IFTTT_MAX_NUMBER) {
                        ifttt_this_window_show();
                    }
                    break;
                default:
                    break;
            }

        }
    }
}

void ifttt_list_window_create(void)
{
    static UG_BUTTON buttons[9];
    static UG_OBJECT objects[9];
    char *actions[] = { "1", "4", "3", "5" };
    int i = 0;

    UG_WindowCreate(&g_ifttt_list_window, objects,
            sizeof(objects) / sizeof(*objects), ifttt_list_window_callback);
    UG_WindowSetStyle(&g_ifttt_list_window, WND_STYLE_2D);

    for (i = 0; i < 4; i++) {
        UG_ButtonCreate(&g_ifttt_list_window, buttons + i, i, 0, 40 * i + 40,
                239, 40 * (i + 1) + 40 - 1);
        UG_ButtonSetFont(&g_ifttt_list_window, i, &FONT_SIZE20);
        UG_ButtonSetStyle(&g_ifttt_list_window, i,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

        UG_ButtonSetAlignment(&g_ifttt_list_window, i, ALIGN_CENTER_LEFT);
        UG_ButtonSetHSpace(&g_ifttt_list_window, i, 8);
    }

    for (i = 0; i < 4; i++) {
        int index = 4 + i;
        UG_ButtonCreate(&g_ifttt_list_window, buttons + index, index, 60 * i,
                200, 60 * i + 60 - 1, 239);
        UG_ButtonSetFont(&g_ifttt_list_window, index, &FONT_ICON24);
        UG_ButtonSetText(&g_ifttt_list_window, index, actions[i]);
        UG_ButtonSetStyle(&g_ifttt_list_window, index,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

    }

    UG_ButtonCreate(&g_ifttt_list_window, buttons + 8, 8, 0, 0, 239, 39);
    UG_ButtonSetFont(&g_ifttt_list_window, 8, &FONT_SIZE20);
    UG_ButtonSetText(&g_ifttt_list_window, 8, "if this then that");
    UG_ButtonSetStyle(&g_ifttt_list_window, 8,
            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_ifttt_list_window, 8, 0x000000);

    ifttt_list_window_update();
}

void ifttt_this_window_update()
{
    int i;
    int sensor_number = sensor_get_number();
    if (sensor_number > 0) {
        for (i = 0; i < 4; i++) {
            char *icon = NULL;
            int index = (i + g_condition_first_visible);
            if (index < sensor_number) {
                if (g_condition_index_map[index] != -1) {
                    condition_to_string(g_condition_index_map[index],
                            g_condition_string[i], sizeof(g_condition_string[0]));
                    icon = ":";
                } else {
                    sensor_set_title(index, g_condition_string[i],
                            sizeof(g_condition_string[0]));
                    icon = NULL;
                }
            } else {
                g_condition_string[i][0] = '\0';
            }

            UG_ButtonSetText(&g_ifttt_this_window, i, g_condition_string[i]);
            UG_ButtonSetText(&g_ifttt_this_window, i + 4, icon);
        }
    }
}

void ifttt_this_window_callback(UG_MESSAGE *msg)
{
    char *button_text;
    char *pstr;
    int condition_index;
    int sensor_index;
    int sensor_number;
    int index;
    int id;
    int i;

    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            sensor_number = sensor_get_number();
            switch (msg->sub_id) {
                case 0:
                case 1:
                case 2:
                case 3:
                    sensor_index = (msg->sub_id + g_condition_first_visible);
                    if (sensor_index >= sensor_number) {
                        return;
                    }

                    g_current_condition_position = sensor_index;
                    if (g_condition_index_map[sensor_index] == -1) {
                        condition_index = condition_add(sensor_get(sensor_index));
                        g_condition_index_map[sensor_index] = condition_index;
                    } else {
                        condition_index = g_condition_index_map[sensor_index];
                    }

                    id = sensor_get_id(sensor_index);
                    if (id == BUTTON_ID) {
                        condition_set_operator(condition_index, '=');
                        condition_set_value(condition_index, "1");
                        ifttt_this_window_update();
                    } else if (id == SMS_ID) {
                        condition_set_operator(condition_index, '=');
                        pstr = (char *)condition_get_value(condition_index);
                        *pstr = '\0';
                        UG_ButtonSetText(&g_call_condition_window, 2, pstr);
                        UG_WindowShow(&g_call_condition_window);
                    } else if (id == CALL_ID) {
                        condition_set_operator(condition_index, '=');
                        pstr = (char *)condition_get_value(condition_index);
                        *pstr = '\0';
                        UG_ButtonSetText(&g_call_condition_window, 2, pstr);
                        UG_WindowShow(&g_call_condition_window);
                    } else {
                        ifttt_expr_window_show(condition_index);
                    }
                    break;
                case 4:
                case 5:
                case 6:
                case 7:
                    index = msg->sub_id - 4;
                    button_text = UG_ButtonGetText(&g_ifttt_this_window,
                            msg->sub_id);
                    if (button_text[0] == ':') {
                        sensor_index = index + g_condition_first_visible;
                        condition_remove(g_condition_index_map[sensor_index]);
                        g_condition_index_map[sensor_index] = -1;
                        UG_ButtonSetText(&g_ifttt_this_window, msg->sub_id, NULL);
                        sensor_set_title(index, g_condition_string[i],
                            sizeof(g_condition_string[0]));
                        UG_ButtonSetText(&g_ifttt_this_window, index, g_condition_string[index]);
                    }
                    break;
                case 8: // back
                    // remove unused conditions and actions
                    for (i = 0; i < SENSOR_MAX_NUMBER; i++) {
                        if (g_condition_index_map[i] != -1) {
                            condition_remove(g_condition_index_map[i]);
                        }
                    }
                    UG_WindowShow(&g_ifttt_list_window);
                    break;
                case 9:
                    if (g_condition_first_visible < (sensor_number - 1)) {
                        g_condition_first_visible++;
                        ifttt_this_window_update();
                    }
                    break;
                case 10:
                    if (g_condition_first_visible > 0) {
                        g_condition_first_visible--;
                        ifttt_this_window_update();
                    }
                    break;
                case 11: // next
                    ifttt_that_window_show();
                    break;
                case 12:
                default:
                    break;
            }

        }
    }
}

void ifttt_this_window_create(void)
{
    static UG_BUTTON buttons[13];
    static UG_OBJECT objects[13];
    char *actions[] = { "1", "4", "3", "2" };
    int i;
    int offset;

    UG_WindowCreate(&g_ifttt_this_window, objects,
            sizeof(objects) / sizeof(*objects), ifttt_this_window_callback);
    UG_WindowSetStyle(&g_ifttt_this_window, WND_STYLE_2D);

    offset = 0;
    for (i = 0; i < 4; i++) {
        UG_ButtonCreate(&g_ifttt_this_window, buttons + 4 + i, 4 + i, 0,
                40 * i + 40, 59, 40 * (i + 1) + 40 - 1);
        UG_ButtonSetFont(&g_ifttt_this_window, 4 + i, &FONT_ICON24);
        UG_ButtonSetStyle(&g_ifttt_this_window, 4 + i,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

        UG_ButtonCreate(&g_ifttt_this_window, buttons + i, i, 59, 40 * i + 40,
                239, 40 * (i + 1) + 40 - 1);
        UG_ButtonSetFont(&g_ifttt_this_window, i, &FONT_SIZE20);
        UG_ButtonSetStyle(&g_ifttt_this_window, i,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

        UG_ButtonSetAlignment(&g_ifttt_this_window, i, ALIGN_CENTER_LEFT);
        UG_ButtonSetHSpace(&g_ifttt_this_window, i, 8);
    }

    offset = 8;
    for (i = 0; i < 4; i++) {
        UG_ButtonCreate(&g_ifttt_this_window, buttons + offset, offset, 60 * i,
                200, 60 * i + 60 - 1, 239);
        UG_ButtonSetFont(&g_ifttt_this_window, offset, &FONT_ICON24);
        UG_ButtonSetText(&g_ifttt_this_window, offset, actions[i]);
        UG_ButtonSetStyle(&g_ifttt_this_window, offset,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

        offset++;
    }

    offset = 12;
    UG_ButtonCreate(&g_ifttt_this_window, buttons + offset, offset, 0, 0, 239,
            39);
    UG_ButtonSetFont(&g_ifttt_this_window, offset, &FONT_SIZE20);
    UG_ButtonSetText(&g_ifttt_this_window, offset, "Set conditions");
    UG_ButtonSetStyle(&g_ifttt_this_window, offset,
            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_ifttt_this_window, offset, 0x000000);
}

void ifttt_this_window_show()
{
    int i;
    for (i = 0; i < 4; i++) {
        UG_ButtonSetText(&g_ifttt_this_window, 4 + i, NULL);
    }

    for (i = 0; i < SENSOR_MAX_NUMBER; i++) {
        g_condition_index_map[i] = -1;
    }

    g_condition_first_visible = 0;
    ifttt_this_window_update();

    UG_WindowShow(&g_ifttt_this_window);
}

static void ifttt_expr_window_callback(UG_MESSAGE *msg)
{
    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            int condition_index = g_condition_index_map[g_current_condition_position];
            switch (msg->sub_id) {
                case 4: // Back
                    condition_remove(condition_index);
                    g_condition_index_map[g_current_condition_position] = -1;
                    UG_WindowShow(&g_ifttt_this_window);
                    break;
                case 5: // OK
                    condition_set_value(condition_index,
                            UG_ButtonGetText(&g_ifttt_expr_window, 2));
                    condition_set_operator(condition_index, UG_ButtonGetText(&g_ifttt_expr_window, 1)[0]);
                    ifttt_this_window_update();
                    UG_WindowShow(&g_ifttt_this_window);
                    break;
                case 1: // operator
                    g_current_operator = (g_current_operator + 1) % 3;
                    UG_ButtonSetText(&g_ifttt_expr_window, 1,
                            g_operator_list[g_current_operator]);
                    break;
                case 2: // number
                    input_window_show(g_expr_input_string,
                            sizeof(g_expr_input_string), 2);
                    break;
            }
        }
    }
}

void ifttt_expr_window_create(void)
{
    static UG_BUTTON buttons[6];
    static UG_OBJECT objects[6];
    int i;

    UG_WindowCreate(&g_ifttt_expr_window, objects, sizeof(objects) / sizeof(*objects),
            ifttt_expr_window_callback);

    for (i = 0; i < 4; i++) {
        UG_ButtonCreate(&g_ifttt_expr_window, buttons + i, i, 0, i * 48, 239,
                i * 48 + 47);
        UG_ButtonSetFont(&g_ifttt_expr_window, i, &FONT_SIZE20);
        UG_ButtonSetStyle(&g_ifttt_expr_window, i,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);
    }

    UG_ButtonSetStyle(&g_ifttt_expr_window, 0,
                BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetStyle(&g_ifttt_expr_window, 3,
                BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_ifttt_expr_window, 0, 0);
    UG_ButtonSetBackColor(&g_ifttt_expr_window, 3, 0);

    UG_ButtonCreate(&g_ifttt_expr_window, buttons + 4, 4, 0, 192, 119, 239);
    UG_ButtonSetFont(&g_ifttt_expr_window, 4, &FONT_ICON24);
    UG_ButtonSetText(&g_ifttt_expr_window, 4, "1");
    UG_ButtonSetStyle(&g_ifttt_expr_window, 4,
            BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

    UG_ButtonCreate(&g_ifttt_expr_window, buttons + 5, 5, 120, 192, 239, 239);
    UG_ButtonSetFont(&g_ifttt_expr_window, 5, &FONT_ICON24);
    UG_ButtonSetText(&g_ifttt_expr_window, 5, ":");
    UG_ButtonSetStyle(&g_ifttt_expr_window, 5,
            BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

}

void ifttt_expr_window_show(int condition_index)
{
    int id = g_condition_list[condition_index].id;

    g_current_operator = 0;

    UG_ButtonSetText(&g_ifttt_expr_window, 0, sensor_info_table[id].name);
    UG_ButtonSetText(&g_ifttt_expr_window, 1,
            g_operator_list[g_current_operator]);
    UG_ButtonSetText(&g_ifttt_expr_window, 3, sensor_info_table[id].unit);

    g_expr_input_string[0] = '\0';
    UG_ButtonSetText(&g_ifttt_expr_window, 2, g_expr_input_string);

    UG_WindowShow(&g_ifttt_expr_window);
}

void ifttt_that_window_update()
{
    int i;
    char *icon;
    char *name;
    int actuator_number = actuator_get_number();
    for (i = 0; i < 4; i++) {
        int index = (i + g_action_first_visible);
        if (index < actuator_number) {
            if (g_action_index_map[index] != -1) {
                icon = ":";
            } else {
                icon = NULL;
            }
            name = actuator_get_name(index);
        } else {
            name = NULL;
        }
        UG_ButtonSetText(&g_ifttt_that_window, i + 4, icon);
        UG_ButtonSetText(&g_ifttt_that_window, i, name);
    }
}

void ifttt_that_window_callback(UG_MESSAGE *msg)
{
    int i;
    int actuator_index;
    char *button_text;
    actuator_pfunc_t pfunc;

    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            int actuator_number = actuator_get_number();
            switch (msg->sub_id) {
                case 0:
                case 1:
                case 2:
                case 3:
                    actuator_index = (msg->sub_id + g_action_first_visible);
                    if (actuator_index >= actuator_number) {
                        return;
                    }
                    g_action_index_map[actuator_index] = action_add(
                            actuator_get_action_function(actuator_index));
                    actuator_get_action_data(actuator_index,
                            action_get_data(
                                    g_action_index_map[actuator_index]));

                    UG_ButtonSetText(&g_ifttt_that_window, msg->sub_id + 4,
                            ":");
                    break;
                case 4:
                case 5:
                case 6:
                case 7:
                    button_text = UG_ButtonGetText(&g_ifttt_that_window,
                            msg->sub_id);
                    if (button_text[0] == ':') {
                        actuator_index = (msg->sub_id - 4 + g_action_first_visible);
                        action_remove(g_action_index_map[actuator_index]);
                        g_action_index_map[actuator_index] = -1;
                        UG_ButtonSetText(&g_ifttt_that_window, msg->sub_id, NULL);
                    }
                    break;
                case 8: // back
                    for (i = 0; i < ACTUATOR_MAX_NUMBER; i++) {
                        if (g_action_index_map[i] != -1) {
                            action_remove(g_action_index_map[i]);
                        }
                    }
                    UG_WindowShow(&g_ifttt_this_window);
                    break;
                case 9:
                    if (g_action_first_visible < (actuator_number - 1)) {
                        g_action_first_visible++;
                        ifttt_that_window_update();
                    }
                    break;
                case 10:
                    if (g_action_first_visible > 0) {
                        g_action_first_visible--;
                        ifttt_that_window_update();
                    }
                    break;
                case 11: // ok
                    if (ifttt_add(g_condition_index_map, g_action_index_map) >= 0) {
                        g_ifttt_changed = 1;
                        ifttt_list_window_update();
                    }
                    UG_WindowShow(&g_ifttt_list_window);
                    break;
                case 12:
                default:
                    break;
            }

        }
    }
}

void ifttt_that_window_create(void)
{
    static UG_BUTTON buttons[13];
    static UG_OBJECT objects[13];
    char *actions[] = { "1", "4", "3", ":" };
    int i;
    int offset;

    UG_WindowCreate(&g_ifttt_that_window, objects,
            sizeof(objects) / sizeof(*objects), ifttt_that_window_callback);
    UG_WindowSetStyle(&g_ifttt_that_window, WND_STYLE_2D);

    offset = 0;
    for (i = 0; i < 4; i++) {
        UG_ButtonCreate(&g_ifttt_that_window, buttons + 4 + i, 4 + i, 0,
                40 * i + 40, 59, 40 * (i + 1) + 40 - 1);
        UG_ButtonSetFont(&g_ifttt_that_window, 4 + i, &FONT_ICON24);
        UG_ButtonSetStyle(&g_ifttt_that_window, 4 + i,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

        UG_ButtonCreate(&g_ifttt_that_window, buttons + i, i, 59, 40 * i + 40,
                239, 40 * (i + 1) + 40 - 1);
        UG_ButtonSetFont(&g_ifttt_that_window, i, &FONT_SIZE20);
        UG_ButtonSetStyle(&g_ifttt_that_window, i,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

        UG_ButtonSetAlignment(&g_ifttt_that_window, i, ALIGN_CENTER_LEFT);
        UG_ButtonSetHSpace(&g_ifttt_that_window, i, 8);
    }

    offset = 8;
    for (i = 0; i < 4; i++) {
        UG_ButtonCreate(&g_ifttt_that_window, buttons + offset, offset, 60 * i,
                200, 60 * i + 60 - 1, 239);
        UG_ButtonSetFont(&g_ifttt_that_window, offset, &FONT_ICON24);
        UG_ButtonSetText(&g_ifttt_that_window, offset, actions[i]);
        UG_ButtonSetStyle(&g_ifttt_that_window, offset,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

        offset++;
    }

    offset = 12;
    UG_ButtonCreate(&g_ifttt_that_window, buttons + offset, offset, 0, 0, 239,
            39);
    UG_ButtonSetFont(&g_ifttt_that_window, offset, &FONT_SIZE20);
    UG_ButtonSetText(&g_ifttt_that_window, offset, "Set actions");
    UG_ButtonSetStyle(&g_ifttt_that_window, offset,
            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_ifttt_that_window, offset, 0x000000);
}

void ifttt_that_window_show()
{
    int i;
    for (i = 0; i < 4; i++) {
        UG_ButtonSetText(&g_ifttt_that_window, 4 + i, NULL);
    }

    for (i = 0; i < ACTUATOR_MAX_NUMBER; i++) {
        g_action_index_map[i] = -1;
    }

    g_action_first_visible = 0;
    ifttt_that_window_update();

    UG_WindowShow(&g_ifttt_that_window);
}

void ifttt_info_window_callback(UG_MESSAGE *msg)
{
    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            switch (msg->sub_id) {
                case 1:
                    ifttt_remove(g_ifttt_current_item);
                    g_ifttt_changed = 1;
                    ifttt_list_window_update();
                case 0:
                    UG_WindowShow(&g_ifttt_list_window);
                    break;
            }

        }
    }
}

void ifttt_info_window_create(void)
{
    static UG_BUTTON buttons[3];
    static UG_OBJECT objects[3];
    char *actions[] = { "1", "7" };
    int i = 0;

    UG_WindowCreate(&g_ifttt_info_window, objects,
            sizeof(objects) / sizeof(*objects), ifttt_info_window_callback);
    UG_WindowSetStyle(&g_ifttt_info_window, WND_STYLE_2D);

    for (i = 0; i < 2; i++) {
        int index = i;
        UG_ButtonCreate(&g_ifttt_info_window, buttons + index, index, 120 * i,
                200, 120 * i + 120 - 1, 239);
        UG_ButtonSetFont(&g_ifttt_info_window, index, &FONT_ICON24);
        UG_ButtonSetText(&g_ifttt_info_window, index, actions[i]);
        UG_ButtonSetStyle(&g_ifttt_info_window, index,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

    }

    UG_ButtonCreate(&g_ifttt_info_window, buttons + 2, 2, 10, 10, 229, 189);
    UG_ButtonSetFont(&g_ifttt_info_window, 2, &FONT_SIZE20);
    UG_ButtonSetStyle(&g_ifttt_info_window, 2,
            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_ifttt_info_window, 2, 0);
    UG_ButtonSetAlignment(&g_ifttt_info_window, 2, ALIGN_TOP_LEFT);
    UG_ButtonSetHSpace(&g_ifttt_info_window, 2, 8);
}

void ifttt_info_window_show(char *info)
{
    UG_ButtonSetText(&g_ifttt_info_window, 2, info);
    UG_WindowShow(&g_ifttt_info_window);
}



void sms_condition_window_callback(UG_MESSAGE *msg)
{
    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            int condition_index = g_condition_index_map[g_current_condition_position];
            char *ptr;
            switch (msg->sub_id) {
            case 0: // back
                condition_remove(condition_index);
                g_condition_index_map[g_current_condition_position] = -1;
                UG_WindowShow(&g_ifttt_this_window);
                break;
            case 1: // ok
                if (NULL == UG_ButtonGetText(&g_sms_condition_window, 5)) {
                    ptr = UG_ButtonGetText(&g_sms_condition_window, 2);
                    strcpy(ptr, "anyone");
                }
                ifttt_this_window_update();
                UG_WindowShow(&g_ifttt_this_window);
                break;
            case 2:
                contact_list_window_show();
                break;
            case 3:
            case 5:
                UG_ButtonSetText(&g_sms_condition_window, 5, ":");
                UG_ButtonSetText(&g_sms_condition_window, 6, NULL);
                break;
            case 4:
            case 6:
                UG_ButtonSetText(&g_sms_condition_window, 6, ":");
                UG_ButtonSetText(&g_sms_condition_window, 5, NULL);
                break;
            }
        }
    }
}

void sms_condition_window_create()
{
    static UG_BUTTON buttons[7];
    static UG_OBJECT objects[7];
    char *texts[] = {"SMS from number:", "SMS from anyone", 0, ":"};
    char *actions[] = {"1", ":"};
    int i;
    int index;

    UG_WindowCreate(&g_sms_condition_window,
                    objects,
                    sizeof(objects)/sizeof(*objects),
                    sms_condition_window_callback);

    for (i = 0; i < 2; i++) {
        int index = i;
        UG_ButtonCreate(&g_sms_condition_window, buttons + index, index, 120 * i,
                200, 120 * i + 120 - 1, 239);
        UG_ButtonSetFont(&g_sms_condition_window, index, &FONT_ICON24);
        UG_ButtonSetText(&g_sms_condition_window, index, actions[i]);
        UG_ButtonSetStyle(&g_sms_condition_window, index,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

    }

    UG_ButtonCreate(&g_sms_condition_window, buttons + 2, 2, 0,
                40, 239, 79);
    UG_ButtonSetFont(&g_sms_condition_window, 2, &FONT_SIZE20);
    UG_ButtonSetStyle(&g_sms_condition_window, 2,
            BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

    for (i = 0; i < 2; i++) {
        index = 3 + i;
        UG_ButtonCreate(&g_sms_condition_window, buttons + index, index, 40, 80 * i, 239, 80 * i + 39);
        UG_ButtonSetFont(&g_sms_condition_window, index, &FONT_SIZE20);
        UG_ButtonSetText(&g_sms_condition_window, index, texts[i]);
        UG_ButtonSetStyle(&g_sms_condition_window, index,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

        index += 2;
        UG_ButtonCreate(&g_sms_condition_window, buttons + index, index, 0, 80 * i, 39, 80 * i + 39);
        UG_ButtonSetFont(&g_sms_condition_window, index, &FONT_ICON24);
        UG_ButtonSetText(&g_sms_condition_window, index, texts[i + 2]);
        UG_ButtonSetStyle(&g_sms_condition_window, index,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);
    }
}

void call_condition_window_callback(UG_MESSAGE *msg)
{
    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            int condition_index = g_condition_index_map[g_current_condition_position];
            char *ptr;
            switch (msg->sub_id) {
            case 0: // back
                condition_remove(condition_index);
                g_condition_index_map[g_current_condition_position] = -1;
                UG_WindowShow(&g_ifttt_this_window);
                break;
            case 1: // ok
                if (NULL == UG_ButtonGetText(&g_call_condition_window, 5)) {
                    ptr = UG_ButtonGetText(&g_call_condition_window, 2);
                    strcpy(ptr, "anyone");
                }
                ifttt_this_window_update();
                UG_WindowShow(&g_ifttt_this_window);
                break;
            case 2:
                contact_list_window_show();
                break;
            case 3:
            case 5:
                UG_ButtonSetText(&g_call_condition_window, 5, ":");
                UG_ButtonSetText(&g_call_condition_window, 6, NULL);
                break;
            case 4:
            case 6:
                UG_ButtonSetText(&g_call_condition_window, 6, ":");
                UG_ButtonSetText(&g_call_condition_window, 5, NULL);
                break;
            }
        }
    }
}

void call_condition_window_create()
{
    static UG_BUTTON buttons[7];
    static UG_OBJECT objects[7];
    char *texts[] = {"Call from number:", "Call from anyone", 0, ":"};
    char *actions[] = {"1", ":"};
    int i;
    int index;

    UG_WindowCreate(&g_call_condition_window, objects, sizeof(objects)/sizeof(*objects),
            call_condition_window_callback);

    for (i = 0; i < 2; i++) {
        int index = i;
        UG_ButtonCreate(&g_call_condition_window, buttons + index, index, 120 * i,
                200, 120 * i + 120 - 1, 239);
        UG_ButtonSetFont(&g_call_condition_window, index, &FONT_ICON24);
        UG_ButtonSetText(&g_call_condition_window, index, actions[i]);
        UG_ButtonSetStyle(&g_call_condition_window, index,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

    }

    UG_ButtonCreate(&g_call_condition_window, buttons + 2, 2, 0,
                40, 239, 79);
    UG_ButtonSetFont(&g_call_condition_window, 2, &FONT_SIZE20);
    UG_ButtonSetStyle(&g_call_condition_window, 2,
            BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

    for (i = 0; i < 2; i++) {
        index = 3 + i;
        UG_ButtonCreate(&g_call_condition_window, buttons + index, index, 40, 80 * i, 239, 80 * i + 39);
        UG_ButtonSetFont(&g_call_condition_window, index, &FONT_SIZE20);
        UG_ButtonSetText(&g_call_condition_window, index, texts[i]);
        UG_ButtonSetStyle(&g_call_condition_window, index,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

        index += 2;
        UG_ButtonCreate(&g_call_condition_window, buttons + index, index, 0, 80 * i, 39, 80 * i + 39);
        UG_ButtonSetFont(&g_call_condition_window, index, &FONT_ICON24);
        UG_ButtonSetText(&g_call_condition_window, index, texts[i + 2]);
        UG_ButtonSetStyle(&g_call_condition_window, index,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);
    }
}

void call_action_window_callback(UG_MESSAGE *msg)
{
    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            switch (msg->sub_id) {
            case 0:
                UG_WindowShow(&g_ifttt_that_window);
                break;
            case 1:
                UG_WindowShow(&g_ifttt_that_window);
                break;
            case 2:
                contact_list_window_show();
                break;
            }
        }
    }
}

void call_action_window_create()
{
    static UG_BUTTON buttons[4];
    static UG_OBJECT objects[4];
    char *actions[] = {"1", ":"};
    int i;

    UG_WindowCreate(&g_call_action_window, objects, sizeof(objects)/sizeof(*objects),
            call_action_window_callback);

    for (i = 0; i < 2; i++) {
        int index = i;
        UG_ButtonCreate(&g_call_action_window, buttons + index, index, 120 * i,
                200, 120 * i + 120 - 1, 239);
        UG_ButtonSetFont(&g_call_action_window, index, &FONT_ICON24);
        UG_ButtonSetText(&g_call_action_window, index, actions[i]);
        UG_ButtonSetStyle(&g_call_action_window, index,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

    }

    UG_ButtonCreate(&g_call_action_window, buttons + 2, 2, 0, 40, 239, 79);
    UG_ButtonSetFont(&g_call_action_window, 2, &FONT_SIZE20);
    UG_ButtonSetStyle(&g_call_action_window, 2,
            BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

    UG_ButtonCreate(&g_call_action_window, buttons + 3, 3, 0, 0, 239, 39);
    UG_ButtonSetFont(&g_call_action_window, 3, &FONT_SIZE20);
    UG_ButtonSetText(&g_call_action_window, 3, "Call number:");
    UG_ButtonSetStyle(&g_call_action_window, 3,
            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_call_action_window, 3, 0);
}

void call_action_window_show(uint32_t *pdata)
{
    char *ptr = (char *)pdata[0];

    ptr[0] = 0;
    UG_ButtonSetText(&g_call_action_window, 2, ptr);
    UG_WindowShow(&g_call_action_window);
}

void call_do_action(uint32_t *pdata)
{
    char *ptr = (char *)pdata[0];

    calling_window_show(ptr);
}


void ifttt_window_create()
{
    ifttt_list_window_create();
    ifttt_info_window_create();
    ifttt_this_window_create();
    ifttt_that_window_create();
    ifttt_expr_window_create();
    sms_condition_window_create();
    call_condition_window_create();
    call_action_window_create();
}
