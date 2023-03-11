#include <stdio.h>
#include <font_manager.h>
#include <string.h>


static PFontOpr g_ptFonts = NULL;
static PFontOpr g_ptDefaulFontOpr = NULL;


/* 注册字体库进链表 */
void RegisterFont(PFontOpr ptFontOpr)
{
	ptFontOpr->ptNext = g_ptFonts;
	g_ptFonts = ptFontOpr;
}

/* 注册字体库 */
void FontSystemRegister(void)
{
	extern void FreetypeRegister(void);
	extern void AsciiFontRegister(void);
	extern void GbkFontRegister(void);
	FreetypeRegister();
	AsciiFontRegister();
	GbkFontRegister();
}

/* 选择初始化字体 */
int SelectAndInitFont(char *aFontOprName, char *aFontFileName)
{
	PFontOpr ptTmp = g_ptFonts;
	while (ptTmp)
	{
		if (strcmp(ptTmp->name, aFontOprName) == 0)
			break;
		ptTmp = ptTmp->ptNext;
	}

	if (!ptTmp)
		return -1;


	g_ptDefaulFontOpr = ptTmp;
	return ptTmp->FontInit(aFontFileName);
}

/* 显示支持的字符操作方法 */
void ShowFontOpr(void)
{
	int i = 0;
	PFontOpr ptTmp = g_ptFonts;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

/* 得到字符操作方法 */
PFontOpr GetFontOpr(char *pcName)
{
	PFontOpr ptTmp = g_ptFonts;
	
	while (ptTmp)
	{
		if (strcmp(ptTmp->name, pcName) == 0)
		{
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	return NULL;
} 



/* 设置字体大小 */
int SetFontSize(int iFontSize)
{
	return g_ptDefaulFontOpr->SetFontSize(iFontSize);
}

/* 得到位图 */
int GetFontBitMap(unsigned int dwCode, PFontBitMap ptFontBitMap)
{
	return g_ptDefaulFontOpr->GetFontBitMap(dwCode, ptFontBitMap);
}

/* 得到字符外框 */
int GetStringsRegionCar(char *str, PRegionCartesian ptRegionCar)
{
	return g_ptDefaulFontOpr->GetStringRegionCar(str, ptRegionCar);
}


