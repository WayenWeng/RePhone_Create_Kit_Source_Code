
#ifndef _LDLCEINT_H
#define _LDLCEINT_H

#include "vmdcl.h"

//#define LOW     0
//#define HIGH    1
#define FALLING 2
#define RISING  3
#define CHANGE  4


typedef void (*voidFuncPtr)( void ) ;


typedef enum _EExt_Interrupts
{
  EXTERNAL_INT_0=0,//
  EXTERNAL_INT_1=1,//
  EXTERNAL_INT_2=2,//
  EXTERNAL_INT_3=3,//
  EXTERNAL_INT_4=4,//
  EXTERNAL_INT_5=5,//
  EXTERNAL_INT_6=6,//
  EXTERNAL_INT_7=7,//
  EXTERNAL_INT_8=8,//
  EXTERNAL_INT_9=9,//
  EXTERNAL_NUM_INTERRUPTS
}EExt_Interrupts;


typedef struct _Exinterrupts_Struct
{
	VM_DCL_HANDLE handle;
	VMUINT32 pin;
	VMUINT32 eint;
	VMUINT32 first;
	voidFuncPtr cb;
}Exinterrupts_Struct;


typedef void (*callback_ptr)(void);
void attachInterrupt(VMUINT32 pin, callback_ptr callback, VMUINT32 mode);
void detachInterrupt(VMUINT32 pin);
void interrupts(void);
void noInterrupts(void );


#endif
