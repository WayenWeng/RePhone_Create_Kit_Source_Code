

#ifndef _LVOICECALL_H
#define _LVOICECALL_H

#include "vmgsm_tel.h"

enum VoiceCall_Status
{
	IDLE_CALL,
	CALLING,
	RECEIVINGCALL,
	TALKING
};

typedef struct
{
    VMINT result;
    VMINT8 num[42];
}call_info_struct;


VMUINT8 callvoiceCall(void* user_data);
VMUINT8 callanswerCall(void* user_data);
VMUINT8 callhangCall(void* user_data);

void callretrieveCallingNumber(void* user_data);
void callgetVoiceCallStatus(void* user_data);

void callregisterCallback(void (*call_state_changed_callback)(VMINT8));

void call_listener_func(vm_gsm_tel_call_listener_data_t* data);


#endif
