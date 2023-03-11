#include <font_manager.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>


static int g_Fd_hzk16;
static struct stat g_Hzk_stat;
static unsigned char *g_Hzkmem;



static int GbkFontInit(char *aFineName)
{
	g_Fd_hzk16 = open("HZK16", O_RDONLY);
	if (g_Fd_hzk16 < 0)
	{
		printf("can't open HZK16\n");
		return -1;
	}
	if(fstat(g_Fd_hzk16, &g_Hzk_stat))
	{
		printf("can't get fstat\n");
		return -1;
	}
	g_Hzkmem = (unsigned char *)mmap(NULL , g_Hzk_stat.st_size, PROT_READ, MAP_SHARED, g_Fd_hzk16, 0);
	if (g_Hzkmem == (unsigned char *)-1)
	{
		printf("can't mmap for hzk16\n");
		return -1;
	}
	return 0;
}

static int GbkSetFontSize(int iFontSize)
{
	return 0;
}

static int GbkGetFontBitMap(unsigned int dwCode, PFontBitMap ptFontBitMap)
{
	int iArea;
	int iWhere;

	int iPenX = ptFontBitMap->iCurOriginX;
	int iPenY = ptFontBitMap->iCurOriginY;

	printf("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

	if (dwCode & 0xffff0000)
	{
		printf("don't support this code : 0x%x\n", dwCode);
		return -1;
	}	

	iArea  = (int)(dwCode & 0xff) - 0xA1;
	iWhere = (int)((dwCode >> 8) & 0xff) - 0xA1;

	if ((iArea < 0) || (iWhere < 0))
	{
		printf("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	
	ptFontBitMap->iXLeft    = iPenX;
	ptFontBitMap->iYTop     = iPenY - 16;
	ptFontBitMap->iXMax     = iPenX + 16;
	ptFontBitMap->iYMax     = iPenY;
	ptFontBitMap->iBpp      = 1;
	ptFontBitMap->iPitch    = 2;
	ptFontBitMap->pucBuffer = g_Hzkmem + (iArea * 94 + iWhere)*32;
	

//	if (ptFontBitMap->pucBuffer >= g_Hzkmem)
//	{
//		return -1;
//	}

	ptFontBitMap->iNextOriginX = iPenX + 16;
	ptFontBitMap->iNextOriginY = iPenY;
	
	printf("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;

}



static FontOpr g_tGbkOpr={
	.name			= "gbk",
	.FontInit		= GbkFontInit,
	.SetFontSize	= GbkSetFontSize,
	.GetFontBitMap	= GbkGetFontBitMap,
};

void GbkFontRegister(void)
{
	RegisterFont(&g_tGbkOpr);	
}



