#include <render.h>
#include <disp_manager.h>
#include <file.h>
#include <stdio.h>
#include <stdlib.h>
#include <encoding_manager.h>
#include <string.h>
#include <picfmt_manager.h>
#include <draw.h>
#include <font_manager.h>



void FlushVideoMemToDev(PT_VideoMem ptVideoMem)
{
	if (!ptVideoMem->bDevFrameBuffer)
	{
		GetDefaultDisplay()->ShowPage(ptVideoMem);
	}	
}

int GetPixelDatasForIcon(char *strFileName, PPixelDatas ptPixelDatas)
{
	int err;
	T_FileMap tFileMap;
	
	snprintf(tFileMap.strFileName, 128, "%s/%s", ICON_PATH, strFileName);
	tFileMap.strFileName[127] = '\0';

	err = MapFile(&tFileMap);
	if(err == -1)
	{
		printf("MapFile err\n");
		return -1;
	}

	err = Parser("bmp")->isSupport(&tFileMap);
	if(err == 0)
	{
		printf("this is not bmp\n");
		return -1;
	}

	/* 获得fb的iBpp */
	ptPixelDatas->iBpp = GetDisplayBuffer()->iBpp;
	err = Parser("bmp")->GetPixelDatas(&tFileMap,ptPixelDatas);
	if(err == -1)
	{
		printf("GetPixelDatas err\n");
		return -1;		
	}

	return 0;
}

int GetPixelDatasFrmFile(char *strFileName, PPixelDatas ptPixelDatas)
{
	T_FileMap tFileMap;
	int iError;
    PPicFileParser ptParser;
	PDispBuff ptDispBuff; 

	strncpy(tFileMap.strFileName, strFileName, 256);
	tFileMap.strFileName[255] = '\0';
	
	iError = MapFile(&tFileMap);
	if (iError)
	{
		printf("MapFile %s error!\n", strFileName);
		return -1;
	}

	ptDispBuff = GetDisplayBuffer();
	if(ptDispBuff == NULL)
	{
		printf("GetDisplayBuffer is err\n");
	}
    ptParser = GetParser(&tFileMap);
	if (ptParser == NULL)
	{
		printf("%s is not jpg file\n", strFileName);
        UnMapFile(&tFileMap);
		return -1;
	}

	
	ptPixelDatas->iBpp = ptDispBuff->iBpp;
	iError = ptParser->GetPixelDatas(&tFileMap, ptPixelDatas);
	if (iError)
	{
		printf("GetPixelDatas for %s error!\n", strFileName);
        UnMapFile(&tFileMap);
		return -1;
	}

    UnMapFile(&tFileMap);
	return 0;
}


void FreePixelDatasFrmFile(PPixelDatas ptPixelDatas)
{
	//Parser("bmp")->FreePixelDatas(ptPixelDatas);
	free(ptPixelDatas->aucPixelDatas);
}



void FreePixelDatasForIcon(PPixelDatas ptPixelDatas)
{
	Parser("bmp")->FreePixelDatas(ptPixelDatas);
}

static void InvertButton(PT_Layout ptLayout)
{
	int iY;
	int i;
	int iButtonWidthBytes;
	int iLineWidth;
	unsigned char *pucVideoMem;
	PDispBuff ptDispBuff = GetDisplayBuffer();
	iLineWidth = ptDispBuff->iXres * ptDispBuff->iBpp / 8;

	pucVideoMem = ptDispBuff->buff;
	pucVideoMem += ptLayout->iTopLeftY * iLineWidth + ptLayout->iTopLeftX * ptDispBuff->iBpp / 8; /* 图标在Framebuffer中的地址 */
	iButtonWidthBytes = (ptLayout->iBotRightX - ptLayout->iTopLeftX) * ptDispBuff->iBpp / 8;

	for (iY = ptLayout->iTopLeftY; iY <= ptLayout->iBotRightY; iY++)
	{
		for (i = 0; i < iButtonWidthBytes; i++)
		{
			pucVideoMem[i] = ~pucVideoMem[i];  /* 取反 */
		}
		pucVideoMem += iLineWidth;
	}
	
}

/* 返回值: 设置了VideoMem中多少字节 */
static int SetColorForPixelInVideoMem(int iX, int iY, PT_VideoMem ptVideoMem, unsigned int dwColor)
{
	unsigned char *pucVideoMem;
	unsigned short *pwVideoMem16bpp;
	unsigned int *pdwVideoMem32bpp;
	unsigned short wColor16bpp; /* 565 */
	int iRed;
	int iGreen;
	int iBlue;

	pucVideoMem      = ptVideoMem->tPixelDatas.aucPixelDatas;
	pucVideoMem      += iY * ptVideoMem->tPixelDatas.iLineBytes + iX * ptVideoMem->tPixelDatas.iBpp / 8;
	pwVideoMem16bpp  = (unsigned short *)pucVideoMem;
	pdwVideoMem32bpp = (unsigned int *)pucVideoMem;

	
	switch (ptVideoMem->tPixelDatas.iBpp)
	{
		case 8:
		{
			*pucVideoMem = (unsigned char)dwColor;
			return 1;
			break;
		}
		case 16:
		{
			iRed   = (dwColor >> (16+3)) & 0x1f;
			iGreen = (dwColor >> (8+2)) & 0x3f;
			iBlue  = (dwColor >> 3) & 0x1f;
			wColor16bpp = (iRed << 11) | (iGreen << 5) | iBlue;
			*pwVideoMem16bpp	= wColor16bpp;
			return 2;
			break;
		}
		case 32:
		{
			*pdwVideoMem32bpp = dwColor;
			return 4;
			break;
		}
		default :
		{			
			return -1;
		}
	}

	return -1;
}

void ClearRectangleInVideoMem(int iTopLeftX, int iTopLeftY, int iBotRightX, int iBotRightY, PT_VideoMem ptVideoMem, unsigned int dwColor)
{
	int x, y;
	for (y = iTopLeftY; y <= iBotRightY; y++)
		for (x = iTopLeftX; x <= iBotRightX; x++)
			SetColorForPixelInVideoMem(x, y, ptVideoMem, dwColor);
}

static int MergeOneFontToVideoMem(PFontBitMap ptFontBitMap, PT_VideoMem ptVideoMem)
{
	int i=0;
	int x, y;
	int bit;
	int iNum;
	unsigned char ucByte;

	if (ptFontBitMap->iBpp == 1)
	{
		for (y = ptFontBitMap->iYTop; y < ptFontBitMap->iYMax; y++)
		{
			i = (y - ptFontBitMap->iYTop) * ptFontBitMap->iPitch;
			for (x = ptFontBitMap->iXLeft, bit = 7; x < ptFontBitMap->iXMax; x++)
			{
				if (bit == 7)
				{
					ucByte = ptFontBitMap->pucBuffer[i++];
				}
				
				if (ucByte & (1<<bit))
				{
					iNum = SetColorForPixelInVideoMem(x, y, ptVideoMem, COLOR_FOREGROUND);
				}
				else
				{
					/* 使用背景色 */
					// g_ptDispOpr->ShowPixel(x, y, 0); /* 黑 */
					iNum = SetColorForPixelInVideoMem(x, y, ptVideoMem, COLOR_BACKGROUND);
				}
				if (iNum == -1)
				{
					return -1;
				}
				bit--;
				if (bit == -1)
				{
					bit = 7;
				}
			}
		}
	}
	else if (ptFontBitMap->iBpp == 8)
	{
		for (y = ptFontBitMap->iYTop; y < ptFontBitMap->iYMax; y++)
			for (x = ptFontBitMap->iXLeft; x < ptFontBitMap->iXMax; x++)
			{
				//g_ptDispOpr->ShowPixel(x, y, ptFontBitMap->pucBuffer[i++]);
				if (ptFontBitMap->pucBuffer[i++])
				{
					iNum = SetColorForPixelInVideoMem(x, y, ptVideoMem, COLOR_FOREGROUND);
				}
				else
				{
					iNum = SetColorForPixelInVideoMem(x, y, ptVideoMem, COLOR_BACKGROUND);
				}
				
				if (iNum == -1)
				{
					return -1;
				}
			}
	}
	else
	{
		printf("ShowOneFont error, can't support %d bpp\n", ptFontBitMap->iBpp);
		return -1;
	}
	return 0;
}

static int isFontInArea(int iTopLeftX, int iTopLeftY, int iBotRightX, int iBotRightY, PFontBitMap ptFontBitMap)
{
    if ((ptFontBitMap->iXLeft >= iTopLeftX) && (ptFontBitMap->iXMax <= iBotRightX) && \
         (ptFontBitMap->iYTop >= iTopLeftY) && (ptFontBitMap->iYMax <= iBotRightY))
         return 1;
    else
        return 0;
        
}


int MergerStringToCenterOfRectangleInVideoMem(int iTopLeftX, int iTopLeftY, int iBotRightX, int iBotRightY, unsigned char *pucTextString, PT_VideoMem ptVideoMem)
{
	int iLen;
	int iError;
	unsigned char *pucBufStart;
	unsigned char *pucBufEnd;
	unsigned int dwCode;
	FontBitMap tFontBitMap;
	PEncodingOpr ptEncodingOpr;
	PFontOpr ptFontOpr;
	
	int bHasGetCode = 0;

	int iMinX = 32000, iMaxX = -1;
	int iMinY = 32000, iMaxY = -1;

	int iStrTopLeftX , iStrTopLeftY;

	int iWidth, iHeight;

	tFontBitMap.iCurOriginX = 0;
	tFontBitMap.iCurOriginY = 0;
	pucBufStart = pucTextString;
	pucBufEnd   = pucTextString + strlen((char *)pucTextString);   

	/* 0. 清除这个区域 */
	ClearRectangleInVideoMem(iTopLeftX, iTopLeftY, iBotRightX, iBotRightY, ptVideoMem, COLOR_BACKGROUND);

	ptEncodingOpr = SelectEncodingOprByName("utf-8");
	if(ptEncodingOpr == NULL)
	{
		printf("SelectEncodingOprByName is err\n");
		return -1;
	}

	ptFontOpr = GetFontOpr("freetype");
	if(ptFontOpr == NULL)
	{
		printf("GetFontOpr is err\n");
		return -1;
	}	
	
	/* 1.先计算字符串显示的总体宽度、高度 */
	while (1)
	{
		/* 从字符串中逐个取出字符 */
		iLen = ptEncodingOpr->GetCodeFrmBuf(pucBufStart, pucBufEnd, &dwCode);
		if (0 == iLen)
		{
			/* 字符串结束 */
			if (!bHasGetCode)
			{
				//printf("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
				return -1;
			}
			else
			{
				break;
			}
		}

		bHasGetCode = 1;
		pucBufStart += iLen;

		/* 获得字符的位图, 位图信息里含有字符显示时的左上角、右下角坐标 */
		iError = ptFontOpr->GetFontBitMap(dwCode, &tFontBitMap);
		if (0 == iError)
		{									
			if (iMinX > tFontBitMap.iXLeft)
			{
				iMinX = tFontBitMap.iXLeft;
			}
			if (iMaxX < tFontBitMap.iXMax)
			{
				iMaxX = tFontBitMap.iXMax;
			}

			if (iMinY > tFontBitMap.iYTop)
			{
				iMinY = tFontBitMap.iYTop;
			}
			if (iMaxY < tFontBitMap.iXMax)
			{
				iMaxY = tFontBitMap.iYMax;
			}
			
			tFontBitMap.iCurOriginX = tFontBitMap.iNextOriginX;
			tFontBitMap.iCurOriginY = tFontBitMap.iNextOriginY;
		}
		else
		{
			printf("GetFontBitmap for calc width/height error!\n");
		}
	}	

	iWidth  = iMaxX - iMinX;
	iHeight = iMaxY - iMinY;

    /* 如果字符串过长 */
    if (iWidth > iBotRightX - iTopLeftX)
    {
        iWidth = iBotRightX - iTopLeftX;
    }

    /* 如果字符串过高 */
	if (iHeight > iBotRightY - iTopLeftY)
	{
		printf("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		//printf("iHeight = %d, iBotRightY - iTopLeftX = %d - %d = %d\n", iHeight, iBotRightY, iTopLeftY, iBotRightY - iTopLeftY);
		return -1;
	}
	//printf("iWidth = %d, iHeight = %d\n", iWidth, iHeight);

	/* 2.确定第1个字符的原点 
	 * 2.1 先计算左上角坐标
	 */
	iStrTopLeftX = iTopLeftX + (iBotRightX - iTopLeftX - iWidth) / 2;
	iStrTopLeftY = iTopLeftY + (iBotRightY - iTopLeftY - iHeight) / 2;
	//printf("iNewFirstFontTopLeftX = %d, iNewFirstFontTopLeftY = %d\n", iNewFirstFontTopLeftX, iNewFirstFontTopLeftY);

	/*	 
	 * 2.2 再计算第1个字符原点坐标
	 * iMinX - 原来的iCurOriginX(0) = iStrTopLeftX - 新的iCurOriginX
	 * iMinY - 原来的iCurOriginY(0) = iStrTopLeftY - 新的iCurOriginY
	 */
	tFontBitMap.iCurOriginX = iStrTopLeftX - iMinX;
	tFontBitMap.iCurOriginY = iStrTopLeftY - iMinY;

	//printf("iCurOriginX = %d, iCurOriginY = %d\n", tFontBitMap.iCurOriginX, tFontBitMap.iCurOriginY);
	
	pucBufStart = pucTextString;	
	bHasGetCode = 0;
	while (1)
	{
		/* 从字符串中逐个取出字符 */
		iLen = ptEncodingOpr->GetCodeFrmBuf(pucBufStart, pucBufEnd, &dwCode);
		if (0 == iLen)
		{
			/* 字符串结束 */
			if (!bHasGetCode)
			{
				printf("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
				return -1;
			}
			else
			{
				break;
			}
		}

		bHasGetCode = 1;
		pucBufStart += iLen;

		/* 获得字符的位图 */
		iError = ptFontOpr->GetFontBitMap(dwCode, &tFontBitMap);
		if (0 == iError)
		{
			//printf("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
			/* 显示一个字符 */
            if (isFontInArea(iTopLeftX, iTopLeftY, iBotRightX, iBotRightY, &tFontBitMap))
            {
    			if (MergeOneFontToVideoMem(&tFontBitMap, ptVideoMem))
    			{
    				printf("MergeOneFontToVideoMem error for code 0x%x\n", dwCode);
    				return -1;
    			}
            }
            else
            {
                return 0;
            }
			//printf("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
			
			tFontBitMap.iCurOriginX = tFontBitMap.iNextOriginX;
			tFontBitMap.iCurOriginY = tFontBitMap.iNextOriginY;
		}
		else
		{
			printf("GetFontBitmap for drawing error!\n");
		}
	}

	return 0;
}

int isPictureFileSupported(char *strFileName)
{
	T_FileMap tFileMap;
	int iError;

	strncpy(tFileMap.strFileName, strFileName, 256);
	tFileMap.strFileName[255] = '\0';
    iError = MapFile(&tFileMap);
    if (iError)
    {
        printf("MapFile %s error!\n", strFileName);
        return 0;
    }

    if (GetParser(&tFileMap) == NULL)
    {
        UnMapFile(&tFileMap);
        return 0;
    }

    UnMapFile(&tFileMap);
    return 1;
}

int isFiletxtSupported(char *strFileName)
{
	int err;
	extern char Font;

	if(strstr(strFileName,"txt") == NULL)
	{
		printf("this is not .txt file\n");
		return 0;
	}
	err = OpenTextFile(strFileName);
	if(err)
	{
		printf("OpenTextFile err\n");
		return 0;
	}

	
	return 1;
}



void ReleaseButton(PT_Layout ptLayout)
{
	InvertButton(ptLayout);
}

void PressButton(PT_Layout ptLayout)
{
	InvertButton(ptLayout);
}


