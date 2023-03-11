#include <stdio.h>
#include <string.h>
#include <disp_manager.h>
#include <font_manager.h>
#include <common.h>
#include <stdlib.h>
#include <page_manager.h>

/* 管理底层的LCD、WEB */
static PDispOpr g_DispDevs = NULL;
static PDispOpr g_DispDefault = NULL;
static DispBuff g_tDispBuff;

static PT_VideoMem g_tVideoMems = NULL;

static int line_width;
static int pixel_width;

int screen_size;


/* 清屏 */
void CleanScreen(unsigned int dwBackColor)
{
	int i,j;

	int x = 0;
	int y = 0;
	int width = g_tDispBuff.iXres;
	int heigh = g_tDispBuff.iYres;

	for (j = y; j < y + heigh; j++)
	{
		for (i = x; i < x + width; i++)
			PutPixel(i, j, dwBackColor);
	}
}

/* 显示字体位图 */
int DrawFontBitMap(PFontBitMap ptFontBitMap, unsigned int dwColor)
{
	int x;
	int y;
	unsigned char ucByte = 0;
	int i = 0;
	int bit;
	
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
					PutPixel(x, y, dwColor);
				}
				else
				{
					/* 使用背景色, 不用描画 */
					// g_ptDispOpr->ShowPixel(x, y, 0); /* 黑 */
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
					PutPixel(x, y, dwColor);
			}
	}
	else
	{
		printf("ShowOneFont error, can't support %d bpp\n", ptFontBitMap->iBpp);
		return -1;
	}
	return 0;


}


/* 画一个区域 */
void DrawRegion(PRegion ptRegion, unsigned int dwColor)
{
	int x = ptRegion->iLeftUpX;
	int y = ptRegion->iLeftUpY;
	int width = ptRegion->iWidth;
	int heigh = ptRegion->iHeigh;

	int i,j;

	for (j = y; j < y + heigh; j++)
	{
		for (i = x; i < x + width; i++)
			PutPixel(i, j, dwColor);
	}
}

/* 描点函数 */
int PutPixel(int x, int y, unsigned int dwColor)
{
	unsigned char *pen_8 = (unsigned char *)(g_tDispBuff.buff+y*line_width+x*pixel_width);
	unsigned short *pen_16;	
	unsigned int *pen_32;	

	unsigned int red, green, blue;	

	pen_16 = (unsigned short *)pen_8;
	pen_32 = (unsigned int *)pen_8;

	switch (g_tDispBuff.iBpp)
	{
		case 8:
		{
			*pen_8 = dwColor;
			break;
		}
		case 16:
		{
			/* 565 */
			red   = (dwColor >> 16) & 0xff;
			green = (dwColor >> 8) & 0xff;
			blue  = (dwColor >> 0) & 0xff;
			dwColor = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
			*pen_16 = dwColor;
			break;
		}
		case 32:
		{
			*pen_32 = dwColor;
			break;
		}
		default:
		{
			printf("can't surport %dbpp\n", g_tDispBuff.iBpp);
			return -1;
			break;
		}
	}

	return 0;
}



/* 将设备注册进链表 */
void RegisterDisplay(PDispOpr ptDispOpr)
{
	ptDispOpr->ptNext = g_DispDevs;
	g_DispDevs = ptDispOpr;
}


/* 选择默认设备 */
int SelectDefaultDisplay(char *name)
{
	PDispOpr pTmp = g_DispDevs;
	while (pTmp) 
	{
		if (strcmp(name, pTmp->name) == 0)
		{
			g_DispDefault = pTmp;
			return 0;
		}

		pTmp = pTmp->ptNext;
	}

	return -1;
}

void ExitDefaultDisplay(void)
{
	g_DispDefault->DeviceExit();
}

void ShowDispOpr(void)
{
	int i = 0;
	PDispOpr ptTmp = g_DispDevs;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}

}

/* 初始化默认设备 */
int InitDefaultDisplay(void)
{
	int ret;
	
	ret = g_DispDefault->DeviceInit();
	if (ret)
	{
		printf("DeviceInit err\n");
		return -1;
	}

	
	ret = g_DispDefault->GetBuffer(&g_tDispBuff);
	if (ret)
	{
		printf("GetBuffer err\n");
		return -1;
	}

	line_width  = g_tDispBuff.iXres * g_tDispBuff.iBpp/8;
	pixel_width = g_tDispBuff.iBpp/8;

	return 0;
}


/* 得到Buffer空间 */
PDispBuff GetDisplayBuffer(void)
{
	return &g_tDispBuff;
}

PDispOpr GetDefaultDisplay(void)
{
	return g_DispDefault;
}

/* 将想要显示的信息刷新到屏幕或者其他设备 */
int FlushDisplayRegion(PRegion ptRegion, PDispBuff ptDispBuff)
{
	return g_DispDefault->FlushRegion(ptRegion, ptDispBuff);
}

/* 申请空间 */
int AllocVideoMem(int iNum)
{
	int i;
	int iVMSize;

	PT_VideoMem ptNew;

	
	iVMSize = g_tDispBuff.iXres * g_tDispBuff.iYres * g_tDispBuff.iBpp/8;

	/* 先把设备本身的framebuffer放入链表 */
	ptNew = malloc(sizeof(T_VideoMem));
	if (ptNew == NULL)
	{
		return -1;
	}	
	ptNew->tPixelDatas.aucPixelDatas = g_tDispBuff.buff;
	
	ptNew->iID = 0;
	ptNew->bDevFrameBuffer = 1;
	ptNew->eVideoMemState = VMS_FREE;
	ptNew->ePicState	  = PS_BLANK;
	ptNew->tPixelDatas.iWidth  = g_tDispBuff.iXres;
	ptNew->tPixelDatas.iHeight = g_tDispBuff.iYres;
	ptNew->tPixelDatas.iBpp    = g_tDispBuff.iBpp;
	ptNew->tPixelDatas.iLineBytes  = line_width;
	ptNew->tPixelDatas.iTotalBytes = iVMSize;

	if (iNum != 0)
	{
		/* 将fb设置为禁止使用 */
		ptNew->eVideoMemState = VMS_USED_FOR_CUR;
	}
	
	/* 放入链表 */
	ptNew->ptNext = g_tVideoMems;
	g_tVideoMems = ptNew;	
	
	for(i=0;i<iNum;i++)
	{
		ptNew = malloc(sizeof(T_VideoMem) + iVMSize);
		if(ptNew == NULL)
		{
			printf("can't malloc the PNew\n");
			return -1;
		}


		ptNew->tPixelDatas.aucPixelDatas = (unsigned char *)(ptNew+1);

		ptNew->iID = 0;
		ptNew->bDevFrameBuffer = 0;
		ptNew->eVideoMemState = VMS_FREE;
		ptNew->ePicState      = PS_BLANK;
		ptNew->tPixelDatas.iWidth  = g_tDispBuff.iXres;
		ptNew->tPixelDatas.iHeight = g_tDispBuff.iYres;
		ptNew->tPixelDatas.iBpp    = g_tDispBuff.iBpp;
		ptNew->tPixelDatas.iLineBytes = line_width;
		ptNew->tPixelDatas.iTotalBytes = iVMSize;

		/* 放入链表 */
		ptNew->ptNext = g_tVideoMems;
		g_tVideoMems = ptNew;
	}

	return 0;
}

/* 取出fb的显存空间 */
PT_VideoMem GetVideoMemDev()
{
	PT_VideoMem Ptemp = g_tVideoMems;
	while(Ptemp)
	{
		if(Ptemp->bDevFrameBuffer)
		{
			return Ptemp;
		}
		Ptemp = Ptemp->ptNext;
	}
	return NULL;
}


/* 得到一块空间 */
PT_VideoMem GetVideoMem(int iID, int bCur)
{
	PT_VideoMem Ptemp = g_tVideoMems;

	/* 1. 优先: 取出空闲的、ID相同的videomem */
	/* ID存在说明此空间已经被设置过 */
	while(Ptemp)
	{
		if((Ptemp->eVideoMemState == VMS_FREE)&&(Ptemp->iID == iID))
		{
			/* 取出一块空间后设置空间状态 */
			Ptemp->eVideoMemState = bCur ? VMS_USED_FOR_CUR : VMS_USED_FOR_PREPARE;
			return Ptemp;
		}
		Ptemp = Ptemp->ptNext;
	}

	/* 2. 优先: 取出任意一个空闲videomem */
	/* ID不存在说明此空间未被使用过 */
	Ptemp = g_tVideoMems;
	while(Ptemp)
	{
		if (Ptemp->eVideoMemState == VMS_FREE)
		{
			Ptemp->iID = iID;
			Ptemp->ePicState = PS_BLANK;
			Ptemp->eVideoMemState = bCur ? VMS_USED_FOR_CUR : VMS_USED_FOR_PREPARE;
			return Ptemp;
		}		
		Ptemp = Ptemp->ptNext;
	}
	
	return NULL;
}

void ClearVideoMem(PT_VideoMem ptVideoMem, unsigned int dwColor)
{
	unsigned char *pucVM;
	unsigned short *pwVM16bpp;
	unsigned int *pdwVM32bpp;
	unsigned short wColor16bpp; /* 565 */
	int iRed;
	int iGreen;
	int iBlue;
	int i = 0;

	pucVM	   = ptVideoMem->tPixelDatas.aucPixelDatas;
	pwVM16bpp  = (unsigned short *)pucVM;
	pdwVM32bpp = (unsigned int *)pucVM;

	switch (ptVideoMem->tPixelDatas.iBpp)
	{
		case 8:
		{
			memset(pucVM, dwColor, ptVideoMem->tPixelDatas.iTotalBytes);
			break;
		}
		case 16:
		{
			iRed   = (dwColor >> (16+3)) & 0x1f;
			iGreen = (dwColor >> (8+2)) & 0x3f;
			iBlue  = (dwColor >> 3) & 0x1f;
			wColor16bpp = (iRed << 11) | (iGreen << 5) | iBlue;
			while (i < ptVideoMem->tPixelDatas.iTotalBytes)
			{
				*pwVM16bpp	= wColor16bpp;
				pwVM16bpp++;
				i += 2;
			}
			break;
		}
		case 32:
		{
			while (i < ptVideoMem->tPixelDatas.iTotalBytes)
			{
				*pdwVM32bpp = dwColor;
				pdwVM32bpp++;
				i += 4;
			}
			break;
		}
		default :
		{
			printf("can't support %d bpp\n", ptVideoMem->tPixelDatas.iBpp);
			return;
		}
	}

}

/* 把显存中某区域全部清为某种颜色 */
void ClearVideoMemRegion(PT_VideoMem ptVideoMem, PT_Layout ptLayout, unsigned int dwColor)
{
	unsigned char *pucVM;
	unsigned short *pwVM16bpp;
	unsigned int *pdwVM32bpp;
	unsigned short wColor16bpp; /* 565 */
	int iRed;
	int iGreen;
	int iBlue;
	int iX;
	int iY;
    int iLineBytesClear;
    int i;

	pucVM	   = ptVideoMem->tPixelDatas.aucPixelDatas + ptLayout->iTopLeftY * ptVideoMem->tPixelDatas.iLineBytes + ptLayout->iTopLeftX * ptVideoMem->tPixelDatas.iBpp / 8;
	pwVM16bpp  = (unsigned short *)pucVM;
	pdwVM32bpp = (unsigned int *)pucVM;

    iLineBytesClear = (ptLayout->iBotRightX - ptLayout->iTopLeftX + 1) * ptVideoMem->tPixelDatas.iBpp / 8;

	switch (ptVideoMem->tPixelDatas.iBpp)
	{
		case 8:
		{
            for (iY = ptLayout->iTopLeftY; iY <= ptLayout->iBotRightY; iY++)
            {
    			memset(pucVM, dwColor, iLineBytesClear);
                pucVM += ptVideoMem->tPixelDatas.iLineBytes;
            }
			break;
		}
		case 16:
		{
			iRed   = (dwColor >> (16+3)) & 0x1f;
			iGreen = (dwColor >> (8+2)) & 0x3f;
			iBlue  = (dwColor >> 3) & 0x1f;
			wColor16bpp = (iRed << 11) | (iGreen << 5) | iBlue;
            for (iY = ptLayout->iTopLeftY; iY <= ptLayout->iBotRightY; iY++)
            {
                i = 0;
                for (iX = ptLayout->iTopLeftX; iX <= ptLayout->iBotRightX; iX++)
    			{
    				pwVM16bpp[i++]	= wColor16bpp;
    			}
                pwVM16bpp = (unsigned short *)((unsigned int)pwVM16bpp + ptVideoMem->tPixelDatas.iLineBytes);
            }
			break;
		}
		case 32:
		{
            for (iY = ptLayout->iTopLeftY; iY <= ptLayout->iBotRightY; iY++)
            {
                i = 0;
                for (iX = ptLayout->iTopLeftX; iX <= ptLayout->iBotRightX; iX++)
    			{
    				pdwVM32bpp[i++]	= dwColor;
    			}
                pdwVM32bpp = (unsigned int *)((unsigned int)pdwVM32bpp + ptVideoMem->tPixelDatas.iLineBytes);
            }
			break;
		}
		default :
		{
			printf("can't support %d bpp\n", ptVideoMem->tPixelDatas.iBpp);
			return;
		}
	}

}



/* 释放一块空间的状态 */
void PutVideoMem(PT_VideoMem ptVideoMem)
{
	ptVideoMem->eVideoMemState = VMS_FREE;
}


/* 注册显示设备 */
void DisplaySystemRegister(void)
{
	
	extern void FramebufferRegister(void);
	FramebufferRegister();
}


