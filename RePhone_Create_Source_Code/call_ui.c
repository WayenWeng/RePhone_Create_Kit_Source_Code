
#include "ugui.h"
#include "vmlog.h"
#include "lvoicecall.h"
#include "laudio.h"
#include "vmtimer.h"
#include "address_book.h"
#include "sensor.h"


char g_incoming_phone_number[32];
int g_calling_time = 0;
char g_calling_time_string[6] = { '0', '0', ':', '0', '0', 0 };
VM_TIMER_ID_PRECISE g_calling_timer_id;

UG_WINDOW g_calling_window;
UG_WINDOW g_incoming_call_window;

extern UG_WINDOW g_home_window;

extern void contact_list_window_create();
extern void contact_list_window_show();
extern void screen_resume(void);

void update_calling_time_callback(VM_TIMER_ID_PRECISE tid, void* user_data)
{
    g_calling_time++;
    g_calling_time_string[4] = '0' + (g_calling_time % 10);
    g_calling_time_string[3] = '0' + ((g_calling_time / 10) % 6);
    g_calling_time_string[1] = '0' + ((g_calling_time / 60) % 10);
    g_calling_time_string[0] = '0' + ((g_calling_time / 600) % 6);

    UG_TextboxSetText(&g_calling_window, TXB_ID_1, g_calling_time_string);
}

static void calling_window_callback(UG_MESSAGE *msg)
{
    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            callhangCall(0);

            vm_timer_delete_precise(g_calling_timer_id);
            UG_WindowShow(&g_home_window);
        }
    }
}

void calling_window_create(void)
{
    static UG_BUTTON button;
    static UG_TEXTBOX textboxs[2];
    static UG_OBJECT objects[3];

    UG_WindowCreate(&g_calling_window, objects, 3, calling_window_callback);

    UG_TextboxCreate(&g_calling_window, textboxs, TXB_ID_0, 10, 10, 230, 80);
    UG_TextboxSetFont(&g_calling_window, TXB_ID_0, &FONT_SIZE20);

    UG_TextboxCreate(&g_calling_window, textboxs + 1, TXB_ID_1, 10, 90, 230,
            170);
    UG_TextboxSetFont(&g_calling_window, TXB_ID_1, &FONT_SIZE20);

    UG_ButtonCreate(&g_calling_window, &button, BTN_ID_0, 0, 180, 239, 239);
    UG_ButtonSetFont(&g_calling_window, BTN_ID_0, &FONT_SIZE20);
    UG_ButtonSetText(&g_calling_window, BTN_ID_0, "END");
    UG_ButtonSetStyle(&g_calling_window, BTN_ID_0,
            BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_calling_window, BTN_ID_0, 0xf44336);
}

void calling_window_show(char *phone_number)
{
    char *status_string;
    char *action_string;

    if (callvoiceCall(phone_number) == TRUE) {
        status_string = "calling";
        action_string = "END";
    } else {
        status_string = "failed";
        action_string = "BACK";
    }

    UG_TextboxSetText(&g_calling_window, 0, phone_number);
    UG_TextboxSetText(&g_calling_window, 1, status_string);
    UG_WindowShow(&g_calling_window);
}

static void incoming_call_window_callback(UG_MESSAGE *msg)
{
    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            switch (msg->sub_id) {
                case 1: // accept
                    callanswerCall(0);

                    UG_TextboxSetText(&g_calling_window, TXB_ID_0,
                            g_incoming_phone_number);
                    vm_log_info("connected");
                    g_calling_time = 0;
                    g_calling_time_string[0] = '0';
                    g_calling_time_string[1] = '0';
                    g_calling_time_string[3] = '0';
                    g_calling_time_string[4] = '0';
                    UG_TextboxSetText(&g_calling_window, TXB_ID_1,
                            g_calling_time_string);
                    g_calling_timer_id = vm_timer_create_precise(1000,
                            update_calling_time_callback, NULL);

                    UG_WindowShow(&g_calling_window);
                    break;
                case 0: // reject
                    callhangCall(0);
                    audioStop(0);
                    UG_WindowShow(&g_home_window);
                    break;
            }

        }
    }
}

void incoming_call_window_create(void)
{
    static UG_BUTTON buttons[2];
    static UG_TEXTBOX textboxs[2];
    static UG_OBJECT objects[4];

    UG_WindowCreate(&g_incoming_call_window, objects, 4,
            incoming_call_window_callback);

    UG_TextboxCreate(&g_incoming_call_window, textboxs, TXB_ID_0, 10, 10, 230,
            80);
    UG_TextboxSetFont(&g_incoming_call_window, TXB_ID_0, &FONT_SIZE20);

    UG_TextboxCreate(&g_incoming_call_window, textboxs + 1, TXB_ID_1, 10, 90,
            230, 170);
    UG_TextboxSetFont(&g_incoming_call_window, TXB_ID_1, &FONT_SIZE20);
    UG_TextboxSetText(&g_incoming_call_window, TXB_ID_1, "incoming");

    UG_ButtonCreate(&g_incoming_call_window, buttons, BTN_ID_0, 0, 180, 119,
            239);
    UG_ButtonSetFont(&g_incoming_call_window, BTN_ID_0, &FONT_SIZE20);
    UG_ButtonSetText(&g_incoming_call_window, BTN_ID_0, "reject");
    UG_ButtonSetStyle(&g_incoming_call_window, BTN_ID_0,
            BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_incoming_call_window, BTN_ID_0, 0xf44336);

    UG_ButtonCreate(&g_incoming_call_window, buttons + 1, BTN_ID_1, 120, 180,
            239, 239);
    UG_ButtonSetFont(&g_incoming_call_window, BTN_ID_1, &FONT_SIZE20);
    UG_ButtonSetText(&g_incoming_call_window, BTN_ID_1, "accept");
    UG_ButtonSetStyle(&g_incoming_call_window, BTN_ID_1,
            BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_incoming_call_window, BTN_ID_1, 0x4CAF50);
}

void incoming_call_window_show()
{
    callretrieveCallingNumber(g_incoming_phone_number);

    UG_TextboxSetText(&g_incoming_call_window, TXB_ID_0,
            g_incoming_phone_number);
    UG_WindowShow(&g_incoming_call_window);
}

void call_state_changed_callback(VMINT8 state)
{
    static uint8_t ringtone_is_playing = 0;
    vm_log_info("call event");

    if (state == RECEIVINGCALL) {


        vm_log_info("incoming call");

        // audioPlay(storageFlash, "ringtone.mp3");
        ringtone_is_playing = 1;

        screen_resume();
        incoming_call_window_show();

        {
            sensor_t *call_sensor = sensor_find(CALL_ID);
            call_sensor->p = g_incoming_phone_number;
            ifttt_check();;
        }
    } else if (state == CALLING) {
        vm_log_info("calling");
    } else if (state == TALKING) {
        if (ringtone_is_playing) {
            ringtone_is_playing = 0;
            // audioStop(NULL);
        }

        vm_log_info("connected");
        g_calling_time = 0;
        g_calling_time_string[0] = '0';
        g_calling_time_string[1] = '0';
        g_calling_time_string[3] = '0';
        g_calling_time_string[4] = '0';
        UG_TextboxSetText(&g_calling_window, TXB_ID_1, g_calling_time_string);
        g_calling_timer_id = vm_timer_create_precise(1000,
                update_calling_time_callback, NULL);
    } else if (state == IDLE_CALL) {
        vm_log_info("endded");

        if (ringtone_is_playing) {
            ringtone_is_playing = 0;
            // audioStop(NULL);
        }

        vm_timer_delete_precise(g_calling_timer_id);

        UG_WindowShow(&g_home_window);

        {
            sensor_t *call_sensor = sensor_find(CALL_ID);
            call_sensor->p = 0;
            ifttt_check();
        }
    }
}



void call_window_create()
{
    contact_list_window_create();
    incoming_call_window_create();
    calling_window_create();

    callregisterCallback(call_state_changed_callback);
}

void call_window_show()
{
    contact_list_window_show();
}
