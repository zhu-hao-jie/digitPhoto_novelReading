#include <debug_manager.h>

static int stdoutDebugInit(void)
{
	return 0;
}

static int stdoutDebugExit(void)
{
	return 0;
}


static int stdoutDebugPrint(char *strData)
{
	/* 直接把输出信息用printf打印出来 */
	printf("%s", strData);
	return strlen(strData);		
	return 0;
}



static T_DebugOpr stdoutPrintOpr={
	.name=		"stdout",
	.isCanUse=	1,
	.DebugInit=	stdoutDebugInit,
	.DebugExit=	stdoutDebugExit,
	.DebugPrint=stdoutDebugPrint,
};

void RegisterstdoutPrint(void)
{
	RegisterDebugOpr(&stdoutPrintOpr);
}

