
#include "vmtype.h"
#include "vmsystem.h"
#include "vmthread.h"
#include "vmlog.h"
#include "vmdcl.h"
#include "vmdcl_gpio.h"
#include "vmdcl_adc.h"
#include "vmdatetime.h"
#include "vmboard.h"
#include "ldlcgpio.h"


static VM_DCL_HANDLE g_adc_handle = VM_DCL_HANDLE_INVALID;
static VMINT32 g_adc_result = 0;


void gpio_delay_ms(VMUINT32 millisecs)
{
   VMUINT32 timeStop;
   VMUINT32 timeStart;
   VMUINT32 Freq = 0;
   volatile VMUINT32 i;
   millisecs = millisecs*1000;

    timeStart = vm_time_ust_get_count();
    while( Freq  < millisecs)
    {
    	for(i=0;i<5000;i++){}
        timeStop = vm_time_ust_get_count();
        Freq = timeStop - timeStart + 1;
    }
}


VMINT8 pinMode(VMUINT8 ulPin,VMUINT8 ulMode)
{
	VM_DCL_HANDLE gpio_handle;
	gpio_handle = vm_dcl_open(VM_DCL_GPIO, ulPin);

    if(gpio_handle != VM_DCL_HANDLE_INVALID)
    {
		switch(ulMode)
		{
			case INPUT:
				vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
				vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
			break;

			case OUTPUT:
				vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
				vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_OUT, NULL);
			break;

			case INPUT_PULLUP:
				vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
				vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
				vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_ENABLE_PULL, NULL);
				vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);
			break;

			case INPUT_PULLDN:
				vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
				vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_DIRECTION_IN, NULL);
				vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_ENABLE_PULL, NULL);
				vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_PULL_LOW, NULL);
			break;

			default:
			break;
		}
    }
    else
    {
    	vm_log_info("gpio set pin mode fail");
    	return -1;
    }

	vm_dcl_close(gpio_handle);
	return TRUE;
}

VMINT8 digitalWrite(VMUINT8 ulPin,VMUINT8 ulData)
{
	VM_DCL_HANDLE gpio_handle;
	gpio_handle = vm_dcl_open(VM_DCL_GPIO, ulPin);

    if(gpio_handle != VM_DCL_HANDLE_INVALID)
    {
		if(ulData == LOW)
			vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_WRITE_LOW, NULL);
		else if(ulData == HIGH)
			vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_WRITE_HIGH, NULL);
    }
    else
    {
    	vm_log_info("gpio write pin fail");
    	return -1;
    }

	vm_dcl_close(gpio_handle);
	return TRUE;
}

VMINT8 digitalRead(VMUINT8 ulPin)
{
	VM_DCL_HANDLE gpio_handle;
	vm_dcl_gpio_control_level_status_t data;
	VMUINT8 ret;

	gpio_handle = vm_dcl_open(VM_DCL_GPIO, ulPin);

    if(gpio_handle != VM_DCL_HANDLE_INVALID)
    {
		vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_READ, (void*)&data);

		if (data.level_status == 1)ret = HIGH;
		else ret = LOW;
    }
    else
    {
    	vm_log_info("gpio read pin fail");
    	return -1;
    }

    vm_dcl_close(gpio_handle);
    return ret;
}

void adc_callback(void* parameter, VM_DCL_EVENT event, VM_DCL_HANDLE device_handle)
{
    vm_dcl_callback_data_t *data;
    vm_dcl_adc_measure_done_confirm_t * result;
    vm_dcl_adc_control_send_stop_t stop_data;
    VMINT status = 0;

    if(parameter!=NULL)
      {
        data = ( vm_dcl_callback_data_t*)parameter;
        result = (vm_dcl_adc_measure_done_confirm_t *)(data->local_parameters);

        if( result != NULL )
        {
            double *p;

            p =(double*)&(result->value);
            g_adc_result = (unsigned int)*p;
            vm_log_info("get adc data is %d",g_adc_result);
        }
     }

    // Stop ADC
    stop_data.owner_id = vm_dcl_get_owner_id();
    status = vm_dcl_control(g_adc_handle,VM_DCL_ADC_COMMAND_SEND_STOP,(void *)&stop_data);

    vm_dcl_close(g_adc_handle);
}

VMINT32 analogRead(VMUINT8 ulPin)
{
	vm_dcl_adc_control_create_object_t obj_data;
	VMINT status = 0 , i;
	vm_dcl_adc_control_send_start_t start_data;

	if(ulPin > 3)// for ADC0 ADC1 ADC2 ADC3
	{
		vm_log_info("ADC pin number is error");
		return -1;
	}

	vm_dcl_close(g_adc_handle);

	g_adc_handle = vm_dcl_open(VM_DCL_GPIO, ulPin);

	vm_dcl_control(g_adc_handle,VM_DCL_GPIO_COMMAND_SET_MODE_2,NULL);
	vm_dcl_close(g_adc_handle);

	// Open ADC device
	g_adc_handle = vm_dcl_open(VM_DCL_ADC,0);
	// register ADC result callback
	status = vm_dcl_register_callback(g_adc_handle, VM_DCL_ADC_GET_RESULT ,(vm_dcl_callback)adc_callback, (void *)NULL);

	// Indicate to the ADC module driver to notify the result.
	obj_data.owner_id = vm_dcl_get_owner_id();
	// Set physical ADC channel which should be measured.
	obj_data.channel = PIN2CHANNEL(ulPin);
	// Set measurement period, the unit is in ticks.
	obj_data.period = 1;
	// Measurement count.
	obj_data.evaluate_count = 1;
	// Whether to send message to owner module or not.
	obj_data.send_message_primitive = 1;

	// setup ADC object
	status = vm_dcl_control(g_adc_handle,VM_DCL_ADC_COMMAND_CREATE_OBJECT,(void *)&obj_data);

	// start ADC
	start_data.owner_id = vm_dcl_get_owner_id();
	status = vm_dcl_control(g_adc_handle,VM_DCL_ADC_COMMAND_SEND_START,(void *)&start_data);

	return g_adc_result;
}
