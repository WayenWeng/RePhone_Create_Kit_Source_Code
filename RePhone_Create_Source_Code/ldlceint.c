
#include "vmsystem.h"
#include "vmtype.h"
#include "vmlog.h"
#include "vmdcl.h"
#include "vmdcl_gpio.h"
#include "vmdcl_eint.h"
#include "string.h"
#include "ldlceint.h"


static Exinterrupts_Struct gExinterruptsPio[EXTERNAL_NUM_INTERRUPTS] =
{
    {VM_DCL_HANDLE_INVALID,  0,  0, 0, NULL},// gpio0
    {VM_DCL_HANDLE_INVALID,  1,  1, 0, NULL},// gpio1
	{VM_DCL_HANDLE_INVALID,  2,  2, 0, NULL},// gpio2
    {VM_DCL_HANDLE_INVALID, 52, 23, 0, NULL},// agpio52
	{VM_DCL_HANDLE_INVALID, 13, 11, 0, NULL},// gpio13
	{VM_DCL_HANDLE_INVALID, 18, 13, 0, NULL},// gpio18
	{VM_DCL_HANDLE_INVALID, 25, 15, 0, NULL},// gpio25
	{VM_DCL_HANDLE_INVALID, 46, 20, 0, NULL},// gpio46
	{VM_DCL_HANDLE_INVALID, 10,  9, 0, NULL},// gpio10
	{VM_DCL_HANDLE_INVALID, 11, 10, 0, NULL},// gpio11
};

VMUINT8 no_interrupt = 1;

void interrupts(void)
{
    no_interrupt = 0;
}

void noInterrupts(void )
{
    no_interrupt = 1;
}

VMUINT8 noStopInterrupts(void)
{
	return no_interrupt;
}

static void eint_callback(void* parameter, VM_DCL_EVENT event, VM_DCL_HANDLE device_handle)
{
    int i;

    for(i=0; i<EXTERNAL_NUM_INTERRUPTS; i++)
    {
        if(gExinterruptsPio[i].handle == device_handle)
        {
            if(noStopInterrupts())
            {
            	interrupts();
            	gExinterruptsPio[i].cb();
            	noInterrupts();
            }
            break;
        }
    }
}

void attachInterrupt(VMUINT32 pin, void (*callback)(void), VMUINT32 mode)
{

	VM_DCL_HANDLE gpio_handle;
    VM_DCL_HANDLE eint_handle;
    vm_dcl_eint_control_config_t eint_config;
    vm_dcl_eint_control_sensitivity_t sens_data;
    vm_dcl_eint_control_hw_debounce_t deboun_time;
    VM_DCL_STATUS status;

    VMUINT8 i;
    for(i=0;i<EXTERNAL_NUM_INTERRUPTS;i++)
	{
    	if(gExinterruptsPio[i].pin == pin)
		{
    		vm_log_info("open EINT number is %d",gExinterruptsPio[i].eint);
			break;
		}
	}
    if(i == EXTERNAL_NUM_INTERRUPTS)
	{
    	vm_log_info("open EINT number error");
    	return;
	}

	detachInterrupt(pin);

	gpio_handle = vm_dcl_open(VM_DCL_GPIO,pin);
	if(gpio_handle == VM_DCL_HANDLE_INVALID)
	{
		vm_log_info("open eint gpio error, gpio_handle is %d", gpio_handle);
		return;
	}
	switch(pin)
	{
		case 0:case 1:case 2:
			vm_log_info("VM_DCL_GPIO_COMMAND_SET_MODE_1");
			vm_dcl_control(gpio_handle,VM_DCL_GPIO_COMMAND_SET_MODE_1,NULL);
		break;

		case 13:case 18:case 46:case 52:
			vm_log_info("VM_DCL_GPIO_COMMAND_SET_MODE_2");
			vm_dcl_control(gpio_handle,VM_DCL_GPIO_COMMAND_SET_MODE_2,NULL);
		break;

		case 10:case 11:
			vm_log_info("VM_DCL_GPIO_COMMAND_SET_MODE_3");
			vm_dcl_control(gpio_handle,VM_DCL_GPIO_COMMAND_SET_MODE_3,NULL);
		break;

		case 25:
			vm_log_info("VM_DCL_GPIO_COMMAND_SET_MODE_4");
			vm_dcl_control(gpio_handle,VM_DCL_GPIO_COMMAND_SET_MODE_4,NULL);
		break;

		default:
		break;
	}
	vm_dcl_control(gpio_handle,VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
	vm_dcl_close(gpio_handle);


    memset(&eint_config,0, sizeof(vm_dcl_eint_control_config_t));
    memset(&sens_data,0, sizeof(vm_dcl_eint_control_sensitivity_t));
    memset(&deboun_time,0, sizeof(vm_dcl_eint_control_hw_debounce_t));

	eint_handle = vm_dcl_open(VM_DCL_EINT,gExinterruptsPio[i].eint);

    if(VM_DCL_HANDLE_INVALID == eint_handle)
    {
        vm_log_info("open EINT error, eint_handle is %d",eint_handle);
        return;
    }

    gExinterruptsPio[i].handle = eint_handle;
    gExinterruptsPio[i].cb = callback;

    status = vm_dcl_control(eint_handle ,VM_DCL_EINT_COMMAND_MASK,NULL);  // Usually, before we config eint, we mask it firstly.
    if(status != VM_DCL_STATUS_OK)
    {
    	vm_log_info("VM_DCL_EINT_COMMAND_MASK  = %d", status);
    }

	status = vm_dcl_register_callback(eint_handle , VM_DCL_EINT_EVENT_TRIGGER,(vm_dcl_callback)eint_callback,(void*)NULL );
	if(status != VM_DCL_STATUS_OK)
	{
		vm_log_info("VM_DCL_EINT_EVENT_TRIGGER = %d", status);
	}

    if(gExinterruptsPio[i].first == 0)
    {
	    if (mode == CHANGE)
	    {
			sens_data.sensitivity = 0;
			eint_config.act_polarity = 0;
			eint_config.auto_unmask = 1;
	    }
	    else
	    {
			if(mode == FALLING)
			{
				sens_data.sensitivity = 0;
				eint_config.act_polarity = 0;
				eint_config.auto_unmask = 1;
			}
			else if(mode == RISING)
			{
				sens_data.sensitivity = 0;
				eint_config.act_polarity = 1;
				eint_config.auto_unmask = 1;
			}
			else
			{
				vm_log_info("mode not support = %d", mode);
			}
	    }

	    status = vm_dcl_control(eint_handle ,VM_DCL_EINT_COMMAND_SET_SENSITIVITY,(void *)&sens_data);  /* set eint sensitivity */
	    if(status != VM_DCL_STATUS_OK)
	    {
	        vm_log_info("VM_DCL_EINT_COMMAND_SET_SENSITIVITY = %d", status);
	    }

	    deboun_time.debounce_time = 1;  // debounce time 1ms
	    status = vm_dcl_control(eint_handle ,VM_DCL_EINT_COMMAND_SET_HW_DEBOUNCE,(void *)&deboun_time); /* set debounce time */
	    if(status != VM_DCL_STATUS_OK)
	    {
	        vm_log_info("VM_DCL_EINT_COMMAND_SET_HW_DEBOUNCE = %d", status);
	    }

	    status = vm_dcl_control(eint_handle ,VM_DCL_EINT_COMMAND_MASK,NULL);  /* Usually, before we config eint, we mask it firstly. */
	    if(status != VM_DCL_STATUS_OK)
	    {
	    	vm_log_info("VM_DCL_EINT_COMMAND_MASK  = %d", status);
	    }

	    eint_config.debounce_enable = 0;    // 1 means enable hw debounce, 0 means disable.

	    status = vm_dcl_control(eint_handle ,VM_DCL_EINT_COMMAND_CONFIG,(void *)&eint_config);   // Please call this api finally, because we will unmask eint in this command.
	    if(status != VM_DCL_STATUS_OK)
	    {
	        vm_log_info("VM_DCL_EINT_COMMAND_CONFIG = %d", status);
	    }

	    if (mode == CHANGE)
	    {
	    	vm_dcl_eint_control_auto_change_polarity_t auto_change;
		    auto_change.auto_change_polarity = 1;
	        status = vm_dcl_control(eint_handle ,VM_DCL_EINT_COMMAND_SET_AUTO_CHANGE_POLARITY,(void *)&auto_change);   // Please call this api finally, because we will unmask eint in this command.
	        if(status != VM_DCL_STATUS_OK)
	        {
	            vm_log_info("VM_DCL_EINT_COMMAND_CONFIG change = %d", status);
	        }
	    }

	    gExinterruptsPio[i].first ++;

	    vm_log_info("attach Interrupt ok.");
    }
    else
    {
        status = vm_dcl_control(eint_handle,VM_DCL_EINT_COMMAND_UNMASK,NULL);  // call this function to unmask this eint.
        if(status != VM_DCL_STATUS_OK)
        {
             vm_log_info("VM_DCL_EINT_COMMAND_CONFIG = %d", status);
        }
    }
}

void detachInterrupt(VMUINT32 pin)
{
	VMUINT32 ulPin;

	switch(pin)
	{
		case 0: ulPin = 0;break;
		case 1: ulPin = 1;break;
		case 2: ulPin = 2;break;
		case 52:ulPin = 3;break;
		case 13:ulPin = 4;break;
		case 18:ulPin = 5;break;
		case 25:ulPin = 6;break;
		case 46:ulPin = 7;break;
		case 10:ulPin = 8;break;
		case 11:ulPin = 9;break;
		default:break;
	}

    if(VM_DCL_HANDLE_INVALID != gExinterruptsPio[ulPin].handle)
    {
    	vm_dcl_close(gExinterruptsPio[ulPin].handle);
    }
	gExinterruptsPio[ulPin].handle = VM_DCL_HANDLE_INVALID;
    gExinterruptsPio[ulPin].cb = NULL;
	gExinterruptsPio[ulPin].first = 0;
}

