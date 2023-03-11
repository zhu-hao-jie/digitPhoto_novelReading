#include <debug_manager.h>


static PT_DebugOpr g_PDebugOpr = NULL;

void RegisterDebugOpr(PT_DebugOpr ptDebugOpr)
{
	ptDebugOpr->ptNext = g_PDebugOpr;
	g_PDebugOpr = ptDebugOpr;
}

void DebugOprInit(void)
{
	extern void RegisterNetPrint(void);
	RegisterNetPrint();
	extern void RegisterstdoutPrint(void);
	RegisterstdoutPrint();
}

void ShowdebugOpr(void)
{
	PT_DebugOpr Ptemp = g_PDebugOpr;
	while(Ptemp)
	{
		printf("%s\n",Ptemp->name);
		Ptemp = Ptemp->ptNext;
	}
}

PT_DebugOpr GetDebugOpr(char *pcname)
{
	PT_DebugOpr Ptemp = g_PDebugOpr;
	while(Ptemp)
	{
		if(strcmp(Ptemp->name,pcname) == 0)
		{
			return Ptemp;
		}
		Ptemp = Ptemp->ptNext;
	}
	return NULL;
}



