#include <page_manager.h>
#include <Pic_operation.h>
#include <stdio.h>
#include <disp_manager.h>
#include <input_manager.h>
#include <render.h>
#include <stdlib.h>
#include <font_manager.h>

static T_Layout g_atintervalPageLayout[] = {
	{0, 0, 0, 0, "inc.bmp"},
	{0, 0, 0, 0, "time.bmp"},
	{0, 0, 0, 0, "dec.bmp"},
	{0, 0, 0, 0, "ok.bmp"},
	{0, 0, 0, 0, "cancel.bmp"},
	{0, 0, 0, 0, NULL},
};

static T_PageLayout g_tIntervalPageLayout = {
	.iMaxTotalBytes = 0,/* 图片信息占据最大的字节数 */
	.atLayout       = g_atintervalPageLayout,
};


/* 显示数字所用 */
static T_Layout g_tIntervalNumberLayout;

static int g_iIntervalSecond = 7;

void GetIntervalTime(int *piIntervalSecond)
{
    *piIntervalSecond = g_iIntervalSecond;
}


static int intervalPageGetInputEvent(PT_PageLayout ptPageLayout, PInputEvent ptInputEvent);

/* 计算各图标坐标值 */
static void  IntervalPageCalcLayout(PT_PageLayout ptPageLayout)
{
	int iStartY;
	int iWidth;
	int iHeight;
	int iTmpTotalBytes;
	PT_Layout atLayout;
	PDispBuff ptPDispBuff;

	atLayout = ptPageLayout->atLayout;
	ptPDispBuff = GetDisplayBuffer();
	ptPageLayout->iBpp = ptPDispBuff->iBpp;

	iHeight = ptPDispBuff->iYres / 3;
	iWidth  = iHeight;
	iStartY = iHeight / 2;
	
	/* inc图标 */
	atLayout[0].iTopLeftY  = iStartY;
	atLayout[0].iBotRightY = atLayout[0].iTopLeftY + iHeight * 28 / 128 - 1;
	atLayout[0].iTopLeftX  = (ptPDispBuff->iXres - iWidth * 52 / 128) / 2;
	atLayout[0].iBotRightX = atLayout[0].iTopLeftX + iWidth * 52 / 128 - 1;

	iTmpTotalBytes = (atLayout[0].iBotRightX - atLayout[0].iTopLeftX + 1) * (atLayout[0].iBotRightY - atLayout[0].iTopLeftY + 1) * ptPDispBuff->iBpp / 8;
	if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes)
	{
		ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
	}

	/* time图标 */
	atLayout[1].iTopLeftY  = atLayout[0].iBotRightY + 1;
	atLayout[1].iBotRightY = atLayout[1].iTopLeftY + iHeight * 72 / 128 - 1;
	atLayout[1].iTopLeftX  = (ptPDispBuff->iXres - iWidth) / 2;
	atLayout[1].iBotRightX = atLayout[1].iTopLeftX + iWidth - 1;
	iTmpTotalBytes = (atLayout[1].iBotRightX - atLayout[1].iTopLeftX + 1) * (atLayout[1].iBotRightY - atLayout[1].iTopLeftY + 1) * ptPDispBuff->iBpp / 8;
	if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes)
	{
		ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
	}

	/* dec图标 */
	atLayout[2].iTopLeftY  = atLayout[1].iBotRightY + 1;
	atLayout[2].iBotRightY = atLayout[2].iTopLeftY + iHeight * 28 / 128 - 1;
	atLayout[2].iTopLeftX  = (ptPDispBuff->iXres - iWidth * 52 / 128) / 2;
	atLayout[2].iBotRightX = atLayout[2].iTopLeftX + iWidth * 52 / 128 - 1;
	iTmpTotalBytes = (atLayout[2].iBotRightX - atLayout[2].iTopLeftX + 1) * (atLayout[2].iBotRightY - atLayout[2].iTopLeftY + 1) * ptPDispBuff->iBpp / 8;
	if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes)
	{
		ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
	}

	/* ok图标 */
	atLayout[3].iTopLeftY  = atLayout[2].iBotRightY + iHeight / 2 + 1;
	atLayout[3].iBotRightY = atLayout[3].iTopLeftY + iHeight / 2 - 1;
	atLayout[3].iTopLeftX  = (ptPDispBuff->iXres - iWidth) / 3;
	atLayout[3].iBotRightX = atLayout[3].iTopLeftX + iWidth / 2 - 1;
	iTmpTotalBytes = (atLayout[3].iBotRightX - atLayout[3].iTopLeftX + 1) * (atLayout[3].iBotRightY - atLayout[3].iTopLeftY + 1) * ptPDispBuff->iBpp / 8;
	if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes)
	{
		ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
	}

	/* cancel图标 */
	atLayout[4].iTopLeftY  = atLayout[3].iTopLeftY;
	atLayout[4].iBotRightY = atLayout[3].iBotRightY;
	atLayout[4].iTopLeftX  = atLayout[3].iTopLeftX * 2 + iWidth/2;
	atLayout[4].iBotRightX = atLayout[4].iTopLeftX + iWidth/2 - 1;
	iTmpTotalBytes = (atLayout[4].iBotRightX - atLayout[4].iTopLeftX + 1) * (atLayout[4].iBotRightY - atLayout[4].iTopLeftY + 1) * ptPDispBuff->iBpp / 8;
	if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes)
	{
		ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
	}

	iWidth  = atLayout[1].iBotRightX - atLayout[1].iTopLeftX + 1;
	iHeight = atLayout[1].iBotRightY - atLayout[1].iTopLeftY + 1;

	g_tIntervalNumberLayout.iTopLeftX  = atLayout[1].iTopLeftX + (128 - 52) / 2 * iWidth / 128;
	g_tIntervalNumberLayout.iBotRightX = atLayout[1].iBotRightX - (128 - 52) / 2 * iWidth / 128 + 1;

	g_tIntervalNumberLayout.iTopLeftY  = atLayout[1].iTopLeftY + (72 - 40) / 2 * iHeight / 72;
	g_tIntervalNumberLayout.iBotRightY = atLayout[1].iBotRightY - (72 - 40) / 2 * iHeight / 72 + 1;	
}

/* 绘制图标中的数字 */
static int GenerateIntervalPageSpecialIcon(int dwNumber, PT_VideoMem ptVideoMem)
{
	unsigned int dwFontSize;
	char strNumber[3];
	int iError;
	

	dwFontSize = g_tIntervalNumberLayout.iBotRightY - g_tIntervalNumberLayout.iTopLeftY;
	SetFontSize(dwFontSize);
	

	/* 显示两位数字: 00~59 */
	if (dwNumber > 59)
	{
		return -1;
	}

	snprintf(strNumber, 3, "%02d", dwNumber);
	//DBG_PRINTF("strNumber = %s, len = %d\n", strNumber, strlen(strNumber));

	iError = MergerStringToCenterOfRectangleInVideoMem(g_tIntervalNumberLayout.iTopLeftX, g_tIntervalNumberLayout.iTopLeftY, g_tIntervalNumberLayout.iBotRightX, g_tIntervalNumberLayout.iBotRightY, (unsigned char *)strNumber, ptVideoMem);

	return iError;
}



static void Showintervalpage(PT_PageLayout ptPageLayout)
{
	PT_VideoMem ptVideoMem;
	
	PT_Layout atLayout = ptPageLayout->atLayout;
	int iError;
	
	/* 1. 获得显存 */
	ptVideoMem = GetVideoMem(ID("interval"),1);
	if(ptVideoMem == NULL)
	{
		printf("GetVideoMem err\n");
		return ;
	}

	/* 2. 描画数据 */
	/* 如果还没有计算过各图标的坐标 */
	if(atLayout[0].iTopLeftX == 0)
	{
		IntervalPageCalcLayout(ptPageLayout);
	}

	/* 创建页面 */
	iError = GeneratePage(ptPageLayout, ptVideoMem);
	GenerateIntervalPageSpecialIcon(g_iIntervalSecond,ptVideoMem);
	if(iError!=0)
	{
		printf("GeneratePage err\n");
	}
	
	/* 3. 刷到设备上去 */
	FlushVideoMemToDev(ptVideoMem);
	
	/* 4. 解放显存 */
	PutVideoMem(ptVideoMem);
}


static void intervalPageRun(PT_PageParams ptParentPageParams)
{
	int iIndex;
	InputEvent tInputEvent;
	int bPressed = 0;
	int iIndexPressed = -1;
	int iNumber = g_iIntervalSecond;
	PT_VideoMem ptVideoMem;

	ptVideoMem = GetVideoMemDev();
	
	/* 1. 显示页面 */
	Showintervalpage(&g_tIntervalPageLayout);	
	/* 2. 创建Prepare线程 */

	/* 3. 调用GetInputEvent获得输入事件，进而处理 */
	while (1)
	{
		iIndex = intervalPageGetInputEvent(&g_tIntervalPageLayout,&tInputEvent);

		if(tInputEvent.iPressure == 0)
		{
			/* 如果是松开 */
			if(bPressed)
			{
				/* 曾经有按钮被按下 */
				ReleaseButton(&g_atintervalPageLayout[iIndexPressed]);
				bPressed = 0;
				
				if (iIndexPressed == iIndex) /* 按下和松开都是同一个按钮 */
				{
					switch (iIndexPressed)
					{
						case 0: /* inc按钮 */
						{
							iNumber++;
							if(iNumber == 60)
							{	
								iNumber = 0;
							}
							GenerateIntervalPageSpecialIcon(iNumber,ptVideoMem);
							break;
						}
						case 2: /* dec按钮 */
						{
							iNumber--;
							if(iNumber == -1)
							{	
								iNumber = 59;
							}
							GenerateIntervalPageSpecialIcon(iNumber,ptVideoMem);
							break;
						}
						case 3: /* ok按钮 */
						{
							g_iIntervalSecond = iNumber;
							return ;
							break;
						}
						case 4: /* cancel按钮 */
						{
							return ;
							break;
						}
						default:
						{
							break;
						}
					}

				}
				iIndexPressed = -1;
			}
		}
		else
		{
			/* 按下状态 */
			if (iIndex != -1)
			{
				if (!bPressed)
				{
					/* 未曾按下按钮 */
					bPressed = 1;
					iIndexPressed = iIndex;
					PressButton(&g_atintervalPageLayout[iIndexPressed]);
				}
			}
			
		}	
	}
}


static int intervalPageGetInputEvent(PT_PageLayout ptPageLayout, PInputEvent ptInputEvent)
{
	return GenerateGetInputEvent(ptPageLayout,ptInputEvent);
}

int intervalPagePrepare(void)
{
	return 0;
}


static PageAction g_tintervalPageAction = {
	.name          = "interval",
	.Run           = intervalPageRun,
	.GetTouchInputEvent = intervalPageGetInputEvent,
	.Prepare       = intervalPagePrepare,
};


void RegisterintervalPage(void)
{
	RegisterPage(&g_tintervalPageAction);
}


