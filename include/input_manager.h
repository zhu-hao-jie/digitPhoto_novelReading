#ifndef _INPUT_MANAGER_H
#define _INPUT_MANAGER_H

#include <sys/time.h>


#ifndef NULL
#define NULL (void *)0
#endif

#define INPUT_TYPE_TOUCH 	1
#define INPUT_TYPE_NET   	2
#define INPUT_TYPE_STDIN	3

#define INPUT_VALUE_UP          3   
#define INPUT_VALUE_DOWN        1
#define INPUT_VALUE_EXIT        2
#define INPUT_VALUE_UNKNOWN     -1


typedef struct InputEvent {
	struct timeval	tTime;
	int iType;
	int iX;
	int iY;
	int Val;
	int iPressure;
	char str[1024];
}InputEvent, *PInputEvent;


typedef struct InputDevice {
	char *name;
	int (*GetInputEvent)(PInputEvent ptInputEvent);
	int (*DeviceInit)(void);
	int (*DeviceExit)(void);
	struct InputDevice *ptNext;
}InputDevice, *PInputDevice;


void RegisterInputDevice(PInputDevice ptInputDev);
void InputSystemRegister(void);
void IntpuDeviceInit(void);
int GetInputEvent(PInputEvent ptInputEvent);
void ShowInputOpr(void);



#endif




