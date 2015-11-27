

#include "vmlog.h"
#include "vmchset.h"
#include "vmaudio.h"
#include "vmaudio_play.h"
#include "vmfs.h"
#include "string.h"
#include "vmstdlib.h"
#include "laudio.h"


VMINT8 m_path[256];
VMINT8 m_volume;
VMINT8 m_drv;
VMINT8 m_type;

static int status_result = 0 ;

static VMINT g_handle = -1;          // The handle of play
static VMINT g_interrupt_handle = 0; // The handle of interrupt


void audio_play_callback(VM_AUDIO_HANDLE handle, VM_AUDIO_RESULT result, void* userdata)
{
  int* res = (int*)userdata;

  * res = result;
  switch (result)
  {
	case VM_AUDIO_RESULT_END_OF_FILE:
	// When the end of file is reached, it needs to stop and close the handle
	vm_audio_play_stop(g_handle);
	vm_audio_play_close(g_handle);
	g_handle = -1;
	break;

	case VM_AUDIO_RESULT_INTERRUPT:
	// The playback is terminated by another application, for example an incoming call
	vm_audio_play_stop(g_handle);
	vm_audio_play_close(g_handle);
	//vm_audio_play_pause(g_handle);
	g_handle = -1;
	break;
	/*
	case VM_AUDIO_FAILED:
	break;

	case VM_AUDIO_SUCCEED:
	break;

	case VM_AUDIO_RESULT_STOP:
	break;

	case VM_AUDIO_RESULT_INTERRUPT_RESUME:
	vm_audio_play_resume(g_handle);
	break;
	*/
	default:
	break;
  }
}

VMUINT8 audioPlay(StorageEnum drv, VMINT8 *songName)
{
	VMWCHAR path[256];
	VMCHAR path_a[256];
	vm_audio_play_parameters_t play_parameters;
	VM_AUDIO_VOLUME volume;

	strcpy((char*)m_path, songName);
	m_drv = drv;
	m_type = TRUE;

	if(m_drv == 0)
	{
		drv = vm_fs_get_internal_drive_letter();
	}
	else
	{
		drv = vm_fs_get_removable_drive_letter();
	}

	if(drv >= 0)
	{
		if(m_type)
		{
			sprintf(path_a,(const signed char*)"%c:\\%s", drv, (VMINT8*)m_path);
			vm_chset_ascii_to_ucs2(path, 256, path_a);
		}
		else
		{
			sprintf(path_a,(const signed char*)"%c:\\", drv);
			vm_chset_ascii_to_ucs2(path, 256, path_a);
			vm_wstr_concatenate(path, (VMWSTR)m_path);
		}
	}
	else
	{
		vm_log_info("AudioPlay get driver error");
		return FALSE;
	}

	// set play parameters
	memset(&play_parameters, 0, sizeof(vm_audio_play_parameters_t));
	play_parameters.filename = path;
	play_parameters.reserved = 0;  //
	play_parameters.format = VM_AUDIO_FORMAT_MP3; //
	//play_parameters.output_path = VM_AUDIO_DEVICE_SPEAKER2;
	//play_parameters.output_path = VM_AUDIO_DEVICE_LOUDSPEAKER;
	play_parameters.output_path = VM_AUDIO_DEVICE_SPEAKER_BOTH;
	play_parameters.async_mode = 0;
	play_parameters.callback = audio_play_callback;
	play_parameters.user_data = &status_result;

	g_handle = vm_audio_play_open(&play_parameters);

	if(g_handle >= VM_OK)
	{
	  vm_log_info("open success");
	  //*res = 0;
	}
	else
	{
	  vm_log_info("open failed");
	  //*res = -1;
	  return FALSE;
	}
	// start to play
	vm_audio_play_start(g_handle);
	// set volume
	// vm_audio_set_volume(VM_AUDIO_VOLUME_6);
	// register interrupt callback
	g_interrupt_handle = vm_audio_register_interrupt_callback(audio_play_callback,&status_result);
}

VMUINT8 audioPause(void* user_data)
{
  if(g_handle >= 0)
  {
	  vm_audio_play_pause(g_handle);
	  return TRUE;
  }

  return FALSE;
}

VMUINT8 audioResume(void* user_data)
{
  if(g_handle >= 0)
  {
	  vm_audio_play_resume(g_handle);
	  return TRUE;
  }
  return FALSE;
}

VMUINT8 audioStop(void* user_data)
{
  if(g_handle >= 0)
  {
	  vm_audio_play_stop(g_handle);
      vm_audio_play_close(g_handle);

      if(g_interrupt_handle!=0)
      {
        vm_audio_clear_interrupt_callback(g_interrupt_handle);
      }
	  return TRUE;
  }
  return FALSE;
}

VMUINT8 audioSetVolume(VMUINT8 volData)
{
	switch(volData)
	{
		case 0:
		vm_audio_set_volume(VM_AUDIO_VOLUME_0);
		break;;
		case 1:
		vm_audio_set_volume(VM_AUDIO_VOLUME_1);
		break;
		case 2:
		vm_audio_set_volume(VM_AUDIO_VOLUME_2);
		break;
		case 3:
		vm_audio_set_volume(VM_AUDIO_VOLUME_3);
		break;
		case 4:
		vm_audio_set_volume(VM_AUDIO_VOLUME_4);
		break;
		case 5:
		vm_audio_set_volume(VM_AUDIO_VOLUME_5);
		break;
		case 6:
		vm_audio_set_volume(VM_AUDIO_VOLUME_6);
		break;
		default:
		break;
	}
	return TRUE;
}

VMINT8  audioGetStatus(void* user_data)
{
	return status_result;
}
