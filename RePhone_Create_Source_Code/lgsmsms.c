

#include "vmlog.h"
#include "vmdcl.h"
#include "vmgsm_sim.h"
#include "vmgsm_sms.h"
#include "vmmemory.h"
#include "stdio.h"
#include "string.h"
#include "vmdatetime.h"
#include "lstorage.h"
#include "lgsmsms.h"
#include "lstorage.h"


sms_send_struct g_sms_send_data;
void (*g_sms_new_message_cb)(char *, char *) = NULL;
void (*g_sms_send_message_cb)(int) = NULL;

unsigned int msgIdMax = 0;        				     // max SMS id
char  msgNumber[GSM_SMS_NUM_MAX][42] = {0};    // number data of current SMS
char  msgContent[GSM_SMS_NUM_MAX][100] = {0};  // content data of current SMS

void sms_inbox_open();
void sms_inbox_save();
void sms_inbox_show();


VMUINT8 gsm_sms_ready(void)
{
    if(vm_gsm_sim_get_card_count() > 0 && vm_gsm_sms_is_sms_ready())
    {
    	return TRUE;
    }
    else
    {
    	return FALSE;
    }
}

void sms_send_callback(vm_gsm_sms_callback_t* callback_data)
{
    g_sms_send_data.result = callback_data->result;

    vm_log_debug("sms end result: %d",  callback_data->result);

    if (g_sms_send_message_cb) {
        g_sms_send_message_cb(callback_data->result);
    }
}

VMUINT8 gsm_sms_begin(void* userdata)
{
	char* to = (char*)userdata;
	strcpy(g_sms_send_data.number, to);

	return TRUE;
}

VMUINT8 gsm_sms_send(void* userdata)
{
	char* to = (char*)userdata;
	strcpy(g_sms_send_data.content, to);

	VMWCHAR number[42];
    VMWCHAR content[100];

    VMUINT8 res;

	vm_chset_ascii_to_ucs2(content, 100*2, g_sms_send_data.content);

	vm_chset_ascii_to_ucs2(number, 42*2, g_sms_send_data.number);

	res = vm_gsm_sms_send(number, content, sms_send_callback, NULL);

    if(res == 0)
    {
    	g_sms_send_data.result = TRUE;
    	vm_log_debug("sms send success!");
        return TRUE;
    }
    else
    {
		g_sms_send_data.result = FALSE;
		vm_log_debug("sms send fail!");
		return FALSE;
    }
}

void sms_read_callback(vm_gsm_sms_callback_t* callback_data)
{
    static char phone_number[42];
    static char content[100];
    vm_gsm_sms_read_message_data_callback_t* read_msg;
    VMINT8 *g_sms_number_buf = 0;
    VMINT8 *g_sms_content_buf = 0;

    vm_log_info("got new message");
    if(callback_data->action == VM_GSM_SMS_ACTION_READ)
    {
        if(callback_data->cause == VM_GSM_SMS_CAUSE_NO_ERROR)
        {
            if(callback_data->action_data)
            {
				read_msg = (vm_gsm_sms_read_message_data_callback_t*)callback_data->action_data;

				int size = vm_wstr_string_length((VMWCHAR*)read_msg->message_data->number);
				g_sms_number_buf = phone_number;
				vm_chset_ucs2_to_ascii((signed char *)g_sms_number_buf, 42, (VMWCHAR*)read_msg->message_data->number);

				// assume dcs = UCS2
				g_sms_content_buf = content;
				vm_chset_ucs2_to_ascii((signed char *)g_sms_content_buf, 100, (VMWCHAR*)read_msg->message_data->content_buffer);

				VMUINT8 i;

				msgIdMax ++;
				if(msgIdMax >= GSM_SMS_NUM_MAX)msgIdMax = GSM_SMS_NUM_MAX;

				for(i=0;i<(GSM_SMS_NUM_MAX - 1);i++)
				{
					strcpy(msgNumber[(GSM_SMS_NUM_MAX - 2 - i) + 1], msgNumber[(GSM_SMS_NUM_MAX - 2) - i]);
					strcpy(msgContent[(GSM_SMS_NUM_MAX - 2 - i) + 1],msgContent[(GSM_SMS_NUM_MAX - 2) - i]);
				}

				strcpy(msgNumber[0], (char*)g_sms_number_buf);
				strcpy(msgContent[0], (char*)g_sms_content_buf);

                vm_log_info("save  message");
				sms_inbox_save();
                vm_log_info("saved");

                if (g_sms_new_message_cb) {
                    g_sms_new_message_cb(g_sms_number_buf, g_sms_content_buf);
                }
                vm_log_info("callback done");

				/*
				for(i=0;i<msgIdMax;i++)
				{
					vm_log_info("msgNumber %s", msgNumber[i]);
					vm_log_info("msgContent %s", msgContent[i]);
				}
				*/

				// Frees the memory allocated by the malloc()
				// vm_free(read_msg->message_data->content_buffer);
				// vm_free(read_msg->message_data);
                // vm_log_info("free memory done");
			}
        }
        else
        {
        	vm_log_info("read msg failed");
        }
    }
}

VMUINT8 gsm_sms_get_unread(void* userdata)
{
	sms_unread_msg_struct *dest = (sms_unread_msg_struct*)userdata;
    VMINT16 message_id;
    vm_gsm_sms_read_message_data_t* message_data = NULL;
    VMWCHAR* content_buff;
    VMINT res;

    //message_id = vm_gsm_sms_get_message_id(VM_GSM_SMS_BOX_INBOX, 0);
	//dest -> id = message_id;
    message_id = dest->id;

    if(message_id >= 0)
    {
		// Allocates memory for the message data
		message_data = (vm_gsm_sms_read_message_data_t*)vm_calloc(sizeof(vm_gsm_sms_read_message_data_t));
		if(message_data == NULL)
		{
			vm_log_info("sms read malloc message data fail");
			return FALSE;
		}

		// Allocates memory for the content buffer of the message
		content_buff = (VMWCHAR*)vm_calloc((500+1)*sizeof(VMWCHAR));
		if(content_buff == NULL)
		{
			vm_free(message_data);
			vm_log_info("sms read malloc content fail");
			return FALSE;

		}
		message_data->content_buffer = content_buff;
		message_data->content_buffer_size = 500;

		// Reads the message
		res = vm_gsm_sms_read_message(message_id, VM_TRUE, message_data, sms_read_callback, NULL);
    	//vm_log_info("vm_gsm_sms_read_message message_id is %d", message_id);

		if(res != VM_GSM_SMS_RESULT_OK)
		{
			vm_free(content_buff);
			vm_free(message_data);

			vm_log_info("register read callback fail");
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}
}

VMUINT8 gsm_sms_read_available(VMINT16 msgID)
{
	sms_unread_msg_struct msgRead;

	msgRead.id = msgID;
	gsm_sms_get_unread(&msgRead);
	if(msgRead.id < 0)return FALSE;

	return TRUE;
}

void sms_delete_callback(vm_gsm_sms_callback_t* callback_data){}

VMUINT8 gsm_sms_delete(void)
{
	VMINT16 message_num;
	VMINT16 message_id;

	message_num = vm_gsm_sms_get_box_size(VM_GSM_SMS_BOX_INBOX);
	//vm_log_info("VM_GSM_SMS_BOX_INBOX number is %d", message_num);
	message_id = vm_gsm_sms_get_message_id(VM_GSM_SMS_BOX_INBOX, message_num - 1);
	//vm_log_info("VM_GSM_SMS_BOX_INBOX id is %d", message_id);

	if(message_id >= 0)
	{
		vm_gsm_sms_delete_message(message_id, sms_delete_callback, NULL);
		return TRUE;
	}

	return FALSE;
}

VMINT gsm_sms_new_message_interrupt_proc(vm_gsm_sms_event_t* event_data)
{
	vm_gsm_sms_event_new_sms_t* event_new_message_ptr;
	vm_gsm_sms_new_message_t* new_message_ptr = NULL;

	if(event_data->event_id == VM_GSM_SMS_EVENT_ID_SMS_NEW_MESSAGE)
	{
		event_new_message_ptr = (vm_gsm_sms_event_new_sms_t *)event_data->event_info; // Gets the event info.
		new_message_ptr = event_new_message_ptr->message_data; // Gets the message data.

		vm_log_info("custom_sms_new_message_interrupt_proc have got ready message");
		gsm_sms_read_available(new_message_ptr->message_id);
	}

	return TRUE;
}

void gsm_sms_set_interrupt_event_handler(void)
{
	VMINT res = 0;

	sms_inbox_open();

	res = vm_gsm_sms_set_interrupt_event_handler(VM_GSM_SMS_EVENT_ID_SMS_NEW_MESSAGE, gsm_sms_new_message_interrupt_proc, NULL);

	if(res != VM_GSM_SMS_RESULT_OK)
	{
		vm_log_info("gsm sms set interrupt fail!");
	}
	else
	{
		vm_log_info("gsm sms set interrupt success!");
	}
}

VMUINT16 gsm_sms_get_list_number(void)
{
	return msgIdMax;
}

char* gsm_sms_remote_number(VMUINT8 num)
{
	if(num >= msgIdMax)return NULL;
	return msgNumber[num];
}

char* gsm_sms_remote_content(VMUINT8 num)
{
	if(num >= msgIdMax)return NULL;
	return msgContent[num];
}

void sms_inbox_open()
{
	unsigned long len;
	unsigned long i;
	msgIdMax = 0;

	file_open("sms_inbox.txt");
	file_size("sms_inbox.txt", &len);

	if(len > 2)
	{
		char str_tmp[GSM_SMS_NUM_MAX*(42+100+5)];
		file_read("sms_inbox.txt", str_tmp, len, 0);

		for(i=0; i<len; )
		{
			sscanf(str_tmp + i,"%[^,]", msgNumber[msgIdMax]);
			sscanf(str_tmp + i,"%*[^,],%[^\r\n]", msgContent[msgIdMax]);

			i += strlen(msgNumber[msgIdMax]) + strlen(msgContent[msgIdMax]) + 3;
			msgIdMax ++;

			if(msgIdMax >= GSM_SMS_NUM_MAX)msgIdMax = GSM_SMS_NUM_MAX;
		}
	}
}

void sms_inbox_save()
{
	int i;

	file_delete("sms_inbox.txt");
	file_open("sms_inbox.txt");

	char str[150];
	for(i=0;i<msgIdMax;i++)
	{
		sprintf(str, "%s,%s\r\n", msgNumber[i], msgContent[i]);
		file_write("sms_inbox.txt", str, 0);
	}
}

void sms_inbox_show()
{
	int i;
    for(i=0; i<msgIdMax; i++)
    {
    	vm_log_info("Number: %s , Content: %s", msgNumber[i], msgContent[i]);
    }
}

void gsm_sms_set_new_message_callback(void (*cb)(char *, char *))
{
    g_sms_new_message_cb = cb;

    gsm_sms_set_interrupt_event_handler();
}

void gsm_sms_set_send_message_callback(void (*cb)(int))
{
    g_sms_send_message_cb = cb;
}
