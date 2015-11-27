
#ifndef _LGSMSMS_H
#define _LGSMSMS_H


#define GSM_SMS_NUM_MAX		20


typedef struct
{
    char number[42];
    char content[100];
    int result;
}sms_send_struct;

typedef struct
{
    int id;
    char *number;
    char *content;
}sms_unread_msg_struct;


void gsm_sms_set_interrupt_event_handler(void);

VMUINT8 gsm_sms_ready(void);
VMUINT8 gsm_sms_begin(void* userdata);
VMUINT8 gsm_sms_send(void* userdata);

VMUINT16 gsm_sms_get_list_number(void);
char* gsm_sms_remote_number(VMUINT8 num);
char* gsm_sms_remote_content(VMUINT8 num);
void gsm_sms_set_new_message_callback(void (*cb)(char *, char *));
void gsm_sms_set_send_message_callback(void (*cb)(int));

void sms_inbox_show();

#endif
