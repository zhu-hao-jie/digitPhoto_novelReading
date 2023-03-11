#include <encoding_manager.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>



static PEncodingOpr g_ptEncodingOprHead;

/* 添加处理进链表 */
void RegisterEncodingOpr(PEncodingOpr ptEncodingOpr)
{
	PEncodingOpr ptTmp;

	if (!g_ptEncodingOprHead)
	{
		g_ptEncodingOprHead   = ptEncodingOpr;
		ptEncodingOpr->ptNext = NULL;
	}
	else
	{
		ptTmp = g_ptEncodingOprHead;
		while (ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext	      = ptEncodingOpr;
		ptEncodingOpr->ptNext = NULL;
	}

}

/* 选择默认的操作 */
PEncodingOpr SelectDefaultEncodingOpr(unsigned char *pucFileBufHead)
{
	PEncodingOpr Ptemp = g_ptEncodingOprHead;
	while(Ptemp)
	{
		if(Ptemp->isSupport(pucFileBufHead))
		{
			return Ptemp;
		}
		Ptemp = Ptemp->ptNext;
	}
	return NULL;
}

PEncodingOpr SelectEncodingOprByName(char *name)
{
	PEncodingOpr Ptemp = g_ptEncodingOprHead;
	while(Ptemp)
	{
		if(strcmp(Ptemp->name,name)==0)
		{
			return Ptemp;
		}
		Ptemp = Ptemp->ptNext;
	}
	return NULL;

}


int AddFontOprForEncoding(PEncodingOpr ptEncodingOpr, PFontOpr ptFontOpr)
{
	PFontOpr ptFontOprCpy;
	
	if (!ptEncodingOpr || !ptFontOpr)
	{
		return -1;
	}
	else
	{
		ptFontOprCpy = malloc(sizeof(FontOpr));
		if (!ptFontOprCpy)
		{
			return -1;
		}
		else
		{
			memcpy(ptFontOprCpy, ptFontOpr, sizeof(FontOpr));
			ptFontOprCpy->ptNext = ptEncodingOpr->ptFontOprSupportedHead;
			ptEncodingOpr->ptFontOprSupportedHead = ptFontOprCpy;
			return 0;
		}		
	}
}

int DelFontOprFrmEncoding(PEncodingOpr ptEncodingOpr, PFontOpr ptFontOpr)
{
	PFontOpr ptTmp;
	PFontOpr ptPre;
		
	if (!ptEncodingOpr || !ptFontOpr)
	{
		return -1;
	}
	else
	{
		ptTmp = ptEncodingOpr->ptFontOprSupportedHead;
		if (strcmp(ptTmp->name, ptFontOpr->name) == 0)
		{
			/* 删除头节点 */
			ptEncodingOpr->ptFontOprSupportedHead = ptTmp->ptNext;
			free(ptTmp);
			return 0;
		}

		ptPre = ptEncodingOpr->ptFontOprSupportedHead;
		ptTmp = ptPre->ptNext;
		while (ptTmp)
		{
			if (strcmp(ptTmp->name, ptFontOpr->name) == 0)
			{
				/* 从链表里取出、释放 */
				ptPre->ptNext = ptTmp->ptNext;
				free(ptTmp);
				return 0;
			}
			else
			{
				ptPre = ptTmp;
				ptTmp = ptTmp->ptNext;
			}
		}

		return -1;
	}
}



/* 显示操作方法 */
void ShowEncodingOpr(void)
{
	int i = 0;
	PEncodingOpr ptTmp = g_ptEncodingOprHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

/* 注册全部编码操作方法 */
int EncodingInit(void)
{
	AsciiEncodingInit();
	Utf8EncodingInit();
	Utf16beEncodingInit();
	Utf16leEncodingInit();
	return 0;
}



