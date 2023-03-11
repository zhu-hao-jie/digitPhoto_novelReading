#ifndef _DEBUG_MANAGER_H
#define _DEBUG_MANAGER_H

#include <stdio.h>
#include <string.h>

typedef struct DebugOpr {
	char *name;
	int isCanUse;
	int (*DebugInit)(void);
	int (*DebugExit)(void);
	int (*DebugPrint)(char *strData);
	struct DebugOpr *ptNext;
}T_DebugOpr, *PT_DebugOpr;

void RegisterDebugOpr(PT_DebugOpr ptDebugOpr);

void DebugOprInit(void);
void ShowdebugOpr(void);
PT_DebugOpr GetDebugOpr(char *pcname);


#endif


