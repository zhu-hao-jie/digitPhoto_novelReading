#include <termios.h>
#include <unistd.h>
#include <input_manager.h>
#include <stdio.h>


static int stdinGetInputEvent(PInputEvent ptInputEvent)
{
	char c;
	/* 处理数据 */
	ptInputEvent->iType = INPUT_TYPE_STDIN;
	
	c = fgetc(stdin);  /* 会休眠直到有输入 */

	if (c == 'u')
	{
		ptInputEvent->Val = INPUT_VALUE_UP;
	}
	else if (c == 'n')
	{
		ptInputEvent->Val = INPUT_VALUE_DOWN;
	}
	else if (c == 'q')
	{
		ptInputEvent->Val = INPUT_VALUE_EXIT;
	}
	else
	{
		ptInputEvent->Val = INPUT_VALUE_UNKNOWN;
	}		
	return 0;

}

static int stdinDeviceInit(void)
{
    struct termios tTTYState;
 
    //get the terminal state
    tcgetattr(STDIN_FILENO, &tTTYState);
 
    //turn off canonical mode
    tTTYState.c_lflag &= ~ICANON;
    //minimum of number input read.
    tTTYState.c_cc[VMIN] = 1;   /* 有一个数据时就立刻返回 */

    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &tTTYState);

	return 0;

}

static int stdinDeviceExit(void)
{
    struct termios tTTYState;
 
    //get the terminal state
    tcgetattr(STDIN_FILENO, &tTTYState);
 
    //turn on canonical mode
    tTTYState.c_lflag |= ICANON;
	
    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &tTTYState);	
	return 0;

}


static InputDevice g_tStdinDev ={
	.name = "stdin",
	.GetInputEvent  = stdinGetInputEvent,
	.DeviceInit     = stdinDeviceInit,
	.DeviceExit     = stdinDeviceExit,
};

void StdinRegister(void)
{
	RegisterInputDevice(&g_tStdinDev);
}



