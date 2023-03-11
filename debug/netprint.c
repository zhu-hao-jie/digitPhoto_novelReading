#include <debug_manager.h>

static int NetDebugInit(void)
{
	return 0;
}

static int NetDebugExit(void)
{
	return 0;
}


static int NetDebugPrint(char *strData)
{
	return 0;
}

static T_DebugOpr NetPrintOpr={
	.name		="NetPrint",
	.isCanUse	=1,
	.DebugInit	=NetDebugInit,
	.DebugExit	=NetDebugExit,
	.DebugPrint	=NetDebugPrint,
};

void RegisterNetPrint(void)
{
	RegisterDebugOpr(&NetPrintOpr);
}

