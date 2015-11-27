

#include "vmlog.h"
#include "vmgsm_tel.h"
#include "vmchset.h"
#include "vmgsm_sim.h"
#include "string.h"
#include "lvoicecall.h"


vm_gsm_tel_call_listener_callback g_call_status_callback = NULL;

static void (*g_call_state_changed_callback)(VMINT8) = NULL;


vm_gsm_tel_id_info_t g_uid_info;
VMINT8 g_call_status = IDLE_CALL;
VMINT8 g_number[42];


void call_listener_func(vm_gsm_tel_call_listener_data_t* data)
{
	vm_log_info("call_listener_func");

    if(data->call_type == VM_GSM_TEL_INDICATION_INCOMING_CALL)
    {
    	vm_gsm_tel_call_info_t* ind = (vm_gsm_tel_call_info_t*)data->data;
		g_uid_info.call_id = ind->uid_info.call_id;
		g_uid_info.group_id = ind->uid_info.group_id;
		g_uid_info.sim = ind->uid_info.sim;
		strcpy(g_number, (char*)ind->num_uri);
		g_call_status = RECEIVINGCALL;

		vm_log_info("incoming call");
    }

    else if(data->call_type == VM_GSM_TEL_INDICATION_OUTGOING_CALL)
    {
    	vm_gsm_tel_call_info_t* ind = (vm_gsm_tel_call_info_t*)data->data;
		g_uid_info.call_id = ind->uid_info.call_id;
		g_uid_info.group_id = ind->uid_info.group_id;
		g_uid_info.sim = ind->uid_info.sim;
		strcpy(g_number, (char*)ind->num_uri);
		g_call_status = CALLING;

		vm_log_info("calling");
    }

    else if(data->call_type == VM_GSM_TEL_INDICATION_CONNECTED)
    {
    	vm_gsm_tel_connect_indication_t* ind = (vm_gsm_tel_connect_indication_t*)data->data;
		g_uid_info.call_id = ind->uid_info.call_id;
		g_uid_info.group_id = ind->uid_info.group_id;
		g_uid_info.sim = ind->uid_info.sim;
		g_call_status = TALKING;

		vm_log_info("connected");
    }

    else if(data->call_type == VM_GSM_TEL_INDICATION_CALL_ENDED)
    {
        g_call_status = IDLE_CALL;
        vm_log_info("endded");
    }

    else
    {
    	vm_log_info("bad operation type");
    }

    vm_log_info("g_call_status is %d",g_call_status);

    if (g_call_state_changed_callback) {
        g_call_state_changed_callback(g_call_status);
    }
}

void callregisterCallback(void (*call_state_changed_callback)(VMINT8))
{
    g_call_state_changed_callback = call_state_changed_callback;

    vm_gsm_tel_call_reg_listener(call_listener_func);
}

static void call_voiceCall_callback(vm_gsm_tel_call_actions_callback_data_t* data)
{
   vm_log_info("call_voiceCall_callback");

   if(data->type_action == VM_GSM_TEL_CALL_ACTION_DIAL)
    {

    	if(data->data_act_rsp.result_info.result == VM_GSM_TEL_OK)
	    {
			g_call_status = CALLING;
    	}
		else
		{
			g_call_status = IDLE_CALL;
		}
    }

    else if(data->type_action == VM_GSM_TEL_CALL_ACTION_ACCEPT)
    {
    	if(data->data_act_rsp.result_info.result == VM_GSM_TEL_OK)
	    {
			g_call_status = TALKING;
    	}
		else
		{
			g_call_status = IDLE_CALL;
		}
    }

    else if(data->type_action == VM_GSM_TEL_CALL_ACTION_HOLD)
    {

    }

    else if(data->type_action == VM_GSM_TEL_CALL_ACTION_END_SINGLE)
    {
		g_call_status = IDLE_CALL;
    }
    else
    {

    }
}

VMUINT8 callvoiceCall(void* user_data)
{
	  VMINT ret;
	  char* to = (char*)user_data;
	  call_info_struct callInfo;
	  strcpy(callInfo.num, to);

	  vm_gsm_tel_dial_action_request_t req;
	  vm_gsm_tel_call_actions_data_t data;

	  //vm_log_info("callvoiceCall");

	  req.sim = VM_GSM_TEL_CALL_SIM_1;
	  req.is_ip_dial = 0;
	  req.module_id = 0;

	  vm_chset_ascii_to_ucs2((VMWSTR)req.num_uri, VM_GSM_TEL_MAX_NUMBER_LENGTH, (VMSTR)&callInfo.num);

	  req.phonebook_data = NULL;

	  data.action = VM_GSM_TEL_CALL_ACTION_DIAL;
	  data.data_action = (void*)&req;
	  data.user_data = NULL;
	  data.callback = call_voiceCall_callback;

	  ret = vm_gsm_tel_call_actions(&data);

	  vm_gsm_tel_set_output_device(VM_GSM_TEL_DEVICE_LOUDSPK);
	  vm_gsm_tel_set_volume(VM_AUDIO_VOLUME_6);

	  if(ret < 0)
	  {
		  vm_log_info("callvoiceCall FALSE");
		  callInfo.result = 0;
		  return FALSE;
	  }
	  else
	  {
		  vm_log_info("callvoiceCall TRUE");
	  	  callInfo.result = 1;
	  	  return TRUE;
	  }
}

VMUINT8 callanswerCall(void* user_data)
{
	  VMINT ret;
	  vm_gsm_tel_single_call_action_request_t req;
	  vm_gsm_tel_call_actions_data_t data;

	  req.action_id.sim   = g_uid_info.sim;
	  req.action_id.call_id = g_uid_info.call_id;
	  req.action_id.group_id = g_uid_info.group_id;

	  data.action = VM_GSM_TEL_CALL_ACTION_ACCEPT;
	  data.data_action = (void*)&req;
	  data.user_data = NULL;
	  data.callback = call_voiceCall_callback;

	  ret = vm_gsm_tel_call_actions(&data);

	  vm_gsm_tel_set_output_device(VM_GSM_TEL_DEVICE_LOUDSPK);
	  vm_gsm_tel_set_volume(VM_AUDIO_VOLUME_6);

	  if(ret < 0)
	  {
		  vm_log_info("callanswerCall FALSE");
	  	  return FALSE;
	  }
	  else
	  {
		  vm_log_info("callanswerCall true");
	  	  return TRUE;
	  }
}

VMUINT8 callhangCall(void* user_data)
{
	  VMINT ret;
	  vm_gsm_tel_single_call_action_request_t req;
	  vm_gsm_tel_call_actions_data_t data;

	  //vm_log_info("callhangCall");

	  if(IDLE_CALL == g_call_status)return;

//	  req.action_id.sim   = g_uid_info.sim;
//	  req.action_id.call_id = g_uid_info.call_id;
//	  req.action_id.group_id = g_uid_info.group_id;
	    req.action_id.sim   = 1;
	      req.action_id.call_id = 1;
	      req.action_id.group_id = 1;

	  data.action = VM_GSM_TEL_CALL_ACTION_END_SINGLE;
	  data.data_action = (void*)&req;
	  data.user_data = NULL;
	  data.callback = call_voiceCall_callback;

	  ret = vm_gsm_tel_call_actions(&data);

	  if(ret < 0)
	  {
		  vm_log_info("callhangCall FALSE");
	  	  return FALSE;
	  }
	  else
	  {
		  vm_log_info("callhangCall TRUE");
	  	  return TRUE;
	  }
}


void callretrieveCallingNumber(void* user_data)
{
	VMINT8* callInfo =  (VMINT8*)user_data;

	if(g_call_status == RECEIVINGCALL)
	{
		strcpy(callInfo, g_number);
	}
}

void callgetVoiceCallStatus(void* user_data)
{
	char* status = (char*)user_data;

	*status = g_call_status;
}
