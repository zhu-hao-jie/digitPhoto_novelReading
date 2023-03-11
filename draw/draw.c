#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <draw.h>
#include <encoding_manager.h>
#include <disp_manager.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


static int g_iFdTextFile = 0;
static unsigned char *g_pucTextFileMem;
static unsigned char *g_pucTextFileMemEnd;
static PEncodingOpr g_ptEncodingOprForFile;

//static PDispOpr g_ptDispOpr;

static unsigned char *g_pucLcdFirstPosAtFile;
static unsigned char *g_pucLcdNextPosAtFile;

static int g_dwFontSize;

static PT_PageDesc g_ptPages   = NULL;
static PT_PageDesc g_ptCurPage = NULL;

static char oldnovelname;


int OpenTextFile(char *pcFileName)
{
	struct stat tStat;

	/*mmap the file*/
	if(g_iFdTextFile == 0)
	{
		/* open text file */
		g_iFdTextFile = open(pcFileName, O_RDONLY);
		if (0 > g_iFdTextFile)
		{
			printf("can't open text file %s\n", pcFileName);
			return -1;
		}

		if(fstat(g_iFdTextFile, &tStat))
		{
			printf("can't get fstat\n");
			return -1;
		}		
		g_pucTextFileMem = (unsigned char *)mmap(NULL , tStat.st_size, PROT_READ, MAP_SHARED, g_iFdTextFile, 0);
		g_pucTextFileMemEnd = g_pucTextFileMem + tStat.st_size;

		/* 判断是哪种编码格式 */
		g_ptEncodingOprForFile = SelectDefaultEncodingOpr(g_pucTextFileMem);

		if (g_ptEncodingOprForFile)
		{
			g_pucLcdFirstPosAtFile = g_pucTextFileMem + g_ptEncodingOprForFile->iHeadLen;
			printf("this EncodingOpr is %s\n",g_ptEncodingOprForFile->name);
			return 0;
		}
		else
		{
			return -1;
		}		
	}
	else
	{
		printf("this file is opened\n");
		return 0;
	}
	if (g_pucTextFileMem == (unsigned char *)-1)
	{
		printf("can't mmap for text file\n");
		return -1;
	}
}

int SetFontsDetail(unsigned int dwFontSize)
{
	//int iError = 0;
	PFontOpr ptFontOpr;
	PFontOpr ptTmp;
	int iRet = -1;

	g_dwFontSize = dwFontSize;
	

	ptFontOpr = g_ptEncodingOprForFile->ptFontOprSupportedHead;
	while (ptFontOpr)
	{
//		if (strcmp(ptFontOpr->name, "ascii") == 0)
//		{			
//			iError = ptFontOpr->FontInit(NULL);
//			ptFontOpr->SetFontSize(dwFontSize);
//		}
//		else if (strcmp(ptFontOpr->name, "gbk") == 0)
//		{
//			iError = ptFontOpr->FontInit(pcHZKFile);
//			ptFontOpr->SetFontSize(dwFontSize);
//		}
//		else
//		{
//			iError = ptFontOpr->FontInit(pcFileFreetype);
//			ptFontOpr->SetFontSize(dwFontSize);
//		}
		if(strcmp(ptFontOpr->name, "freetype") == 0)
		{
			ptFontOpr->SetFontSize(dwFontSize);
			printf("SetFontSize is ok\n");
		}
		printf("%s\n", ptFontOpr->name);

		ptTmp = ptFontOpr->ptNext;

//		if (iError == 0)
//		{
//			/* 比如对于ascii编码的文件, 可能用ascii字体也可能用gbk字体, 
//			 * 所以只要有一个FontInit成功, SetTextDetail最终就返回成功
//			 */
//			iRet = 0;
//		}
//		else
//		{
//			DelFontOprFrmEncoding(g_ptEncodingOprForFile, ptFontOpr);
//		}
		ptFontOpr = ptTmp;
	}
	printf("SetFonts is ok\n");
	return iRet;
}

int SelectAndInitDisplay(char *name)
{
	int iError;
	SelectDefaultDisplay("fb");
	iError = InitDefaultDisplay();
	return iError;
}

void ExitDisplay()
{
	ExitDefaultDisplay();
}

int IncLcdX(int iX)
{
	PDispBuff ptBuffer = GetDisplayBuffer();
	if (iX + 1 < ptBuffer->iXres)
		return (iX + 1);
	else
		return 0;
}

int IncLcdY(int iY)
{
	PDispBuff ptBuffer = GetDisplayBuffer();
	if (iY + g_dwFontSize < ptBuffer->iYres)
		return (iY + g_dwFontSize);
	else
		return 0;
}


int RelocateFontPos(PFontBitMap ptFontBitMap)
{
	int iLcdY;
	int iDeltaX;
	int iDeltaY;
	PDispBuff ptBuffer = GetDisplayBuffer();

	if (ptFontBitMap->iYMax > ptBuffer->iYres)
	{
		/* 满页了 */
		return -1;
	}

	/* 超过LCD最右边 */
	if (ptFontBitMap->iXMax > ptBuffer->iXres)
	{
		/* 换行 */		
		iLcdY = IncLcdY(ptFontBitMap->iCurOriginY);
		if (0 == iLcdY)
		{
			/* 满页了 */
			return -1;
		}
		else
		{
			/* 没满页 */
			iDeltaX = 0 - ptFontBitMap->iCurOriginX;
			iDeltaY = iLcdY - ptFontBitMap->iCurOriginY;

			ptFontBitMap->iCurOriginX  += iDeltaX;
			ptFontBitMap->iCurOriginY  += iDeltaY;

			ptFontBitMap->iNextOriginX += iDeltaX;
			ptFontBitMap->iNextOriginY += iDeltaY;

			ptFontBitMap->iXLeft += iDeltaX;
			ptFontBitMap->iXMax  += iDeltaX;

			ptFontBitMap->iYTop  += iDeltaY;
			ptFontBitMap->iYMax  += iDeltaY;
			
			return 0;	
		}
	}
	
	return 0;
}

int ShowOnePage(unsigned char *pucTextFileMemCurPos)
{
	int iLen;
	int iError;
	unsigned char *pucBufStart;
	unsigned int dwCode;
	PFontOpr ptFontOpr;
	FontBitMap tFontBitMap;
	
	int bHasNotClrSceen = 1;
	int bHasGetCode = 0;

	tFontBitMap.iCurOriginX = 100;
	tFontBitMap.iCurOriginY = g_dwFontSize;
	pucBufStart = pucTextFileMemCurPos;

	
	while (1)
	{
		iLen = g_ptEncodingOprForFile->GetCodeFrmBuf(pucBufStart, g_pucTextFileMemEnd, &dwCode);
		if (0 == iLen)
		{
			/* 文件结束 */
			if (!bHasGetCode)
			{
				return -1;
			}
			else
			{
				return 0;
			}
		}

		bHasGetCode = 1;
		
		pucBufStart += iLen;

		/* 有些文本, \n\r两个一起才表示回车换行
		 * 碰到这种连续的\n\r, 只处理一次
		 */
		if (dwCode == '\n')
		{
			g_pucLcdNextPosAtFile = pucBufStart;
			
			/* 回车换行 */
			tFontBitMap.iCurOriginX = 0;
			tFontBitMap.iCurOriginY = IncLcdY(tFontBitMap.iCurOriginY);
			if (0 == tFontBitMap.iCurOriginY)
			{
				/* 显示完当前一屏了 */
				return 0;
			}
			else
			{
				continue;
			}
		}
		else if (dwCode == '\r')
		{
			continue;
		}
		else if (dwCode == '\t')
		{
			/* TAB键用一个空格代替 */
			dwCode = ' ';
		}

		//printf("dwCode = 0x%x\n", dwCode);
		
		ptFontOpr = g_ptEncodingOprForFile->ptFontOprSupportedHead;
		while (ptFontOpr)
		{
			//printf("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
			iError = ptFontOpr->GetFontBitMap(dwCode, &tFontBitMap);
			//printf("%s %s %d, ptFontOpr->name = %s, %d\n", __FILE__, __FUNCTION__, __LINE__, ptFontOpr->name, iError);
			if (0 == iError)
			{
				//printf("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
				if (RelocateFontPos(&tFontBitMap))
				{
					/* 剩下的LCD空间不能满足显示这个字符 */
					return 0;
				}
				//printf("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

				if (bHasNotClrSceen)
				{
					/* 首先清屏 */
					CleanScreen(0xE7DBB5);
					bHasNotClrSceen = 0;
				}
				//printf("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
				/* 显示一个字符 */
				if (DrawFontBitMap(&tFontBitMap,0x514438))
				{
					return -1;
				}
				
				tFontBitMap.iCurOriginX = tFontBitMap.iNextOriginX;
				tFontBitMap.iCurOriginY = tFontBitMap.iNextOriginY;
				g_pucLcdNextPosAtFile = pucBufStart;

				/* 继续取出下一个编码来显示 */
				break;
			}
			ptFontOpr = ptFontOpr->ptNext;
		}		
	}

	return 0;
}

void RecordPage(PT_PageDesc ptPageNew)
{
	PT_PageDesc ptPage;
		
	if (!g_ptPages)
	{
		g_ptPages = ptPageNew;
	}
	else
	{
		ptPage = g_ptPages;
		while (ptPage->ptNextPage)
		{
			ptPage = ptPage->ptNextPage;
		}
		ptPage->ptNextPage   = ptPageNew;
		ptPageNew->ptPrePage = ptPage;
	}
}

int ShowNextPage(void)
{
	int iError;
	PT_PageDesc ptPage;
	unsigned char *pucTextFileMemCurPos;

	if (g_ptCurPage)
	{
		pucTextFileMemCurPos = g_ptCurPage->pucLcdNextPageFirstPosAtFile;
	}
	else
	{
		pucTextFileMemCurPos = g_pucLcdFirstPosAtFile;
	}
	iError = ShowOnePage(pucTextFileMemCurPos);
	//printf("%s %d, %d\n", __FUNCTION__, __LINE__, iError);
	if (iError == 0)
	{
		if (g_ptCurPage && g_ptCurPage->ptNextPage)
		{
			g_ptCurPage = g_ptCurPage->ptNextPage;
			return 0;
		}
		
		ptPage = malloc(sizeof(T_PageDesc));
		if (ptPage)
		{
			ptPage->pucLcdFirstPosAtFile         = pucTextFileMemCurPos;
			ptPage->pucLcdNextPageFirstPosAtFile = g_pucLcdNextPosAtFile;
			ptPage->ptPrePage                    = NULL;
			ptPage->ptNextPage                   = NULL;
			g_ptCurPage = ptPage;
			printf("%s %d, pos = 0x%x\n", __FUNCTION__, __LINE__, (unsigned int)ptPage->pucLcdFirstPosAtFile);
			RecordPage(ptPage);
			return 0;
		}
		else
		{
			return -1;
		}
	}
	return iError;
}

int ShowPrePage(void)
{
	int iError;

	//printf("%s %d\n", __FUNCTION__, __LINE__);
	if (!g_ptCurPage || !g_ptCurPage->ptPrePage)
	{
		return -1;
	}

	//printf("%s %d, pos = 0x%x\n", __FUNCTION__, __LINE__, (unsigned int)g_ptCurPage->ptPrePage->pucLcdFirstPosAtFile);
	iError = ShowOnePage(g_ptCurPage->ptPrePage->pucLcdFirstPosAtFile);
	if (iError == 0)
	{
		printf("%s %d\n", __FUNCTION__, __LINE__);
		g_ptCurPage = g_ptCurPage->ptPrePage;
	}
	return iError;
}


int DrawInit(void)
{
	int iError;
	DisplaySystemRegister();
	FontSystemRegister();
	iError = EncodingInit();
	if (iError)
	{
		printf("EncodingInit error!\n");
		return -1;
	}
	return 0;	
}


