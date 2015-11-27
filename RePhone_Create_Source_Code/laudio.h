

#ifndef _LAUDIO_H
#define _LAUDIO_H


typedef enum
{
	AudioCommonFailed = -1,     // Playback fails (e.g. the audio file is corrupted).
	AudioStop = 1,              // Playback is stopped.
	AudioPause = 7,             // Playback is paused (and can resume).
	AudioResume = 8,            // Playback resumes
	AudioEndOfFile = 5          // Playback is finished.
}AudioStatus;

// storage location
typedef enum
{
	storageFlash,    // Flash
	storageSD        // SD
}StorageEnum;


VMUINT8 audioPlay(StorageEnum drv, VMINT8 *songName);
VMUINT8 audioPause(void* user_data);
VMUINT8 audioResume(void* user_data);
VMUINT8 audioSetVolume(VMUINT8 volData);
VMUINT8 audioStop(void* user_data);
VMINT8  audioGetStatus(void* user_data);


#endif
