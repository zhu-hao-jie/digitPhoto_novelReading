#include <page_manager.h>
#include <Pic_operation.h>
#include <stdio.h>
#include <disp_manager.h>
#include <input_manager.h>
#include <render.h>
#include <stdlib.h>
#include <string.h>

static T_Layout g_atSettingPageLayout[] = {
	{0, 0, 0, 0, "select_fold.bmp"},
	{0, 0, 0, 0, "interval.bmp"},
	{0, 0, 0, 0, "return.bmp"},
	{0, 0, 0, 0, NULL},
};

static T_PageLayout g_tSettingPageLayout = {
	.iMaxTotalBytes = 0,
	.atLayout       = g_atSettingPageLayout,
};



static int SettingPageGetInputEvent(PT_PageLayout ptPageLayout, PInputEvent ptInputEvent);

static void  SettingPagePageCalcLayout(PT_PageLayout ptPageLayout)
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

	iHeight = ptPDispBuff->iYres * 2 / 10;
	iWidth  = iHeight;
	iStartY = iHeight / 2;

	
	/* select_fold图标 */
	atLayout[0].iTopLeftY  = iStartY;
	atLayout[0].iBotRightY = atLayout[0].iTopLeftY + iHeight - 1;
	atLayout[0].iTopLeftX  = (ptPDispBuff->iXres - iWidth * 2) / 2;
	atLayout[0].iBotRightX = atLayout[0].iTopLeftX + iWidth * 2 - 1;

	iTmpTotalBytes = (atLayout[0].iBotRightX - atLayout[0].iTopLeftX + 1) * (atLayout[0].iBotRightY - atLayout[0].iTopLeftY + 1) * ptPDispBuff->iBpp / 8;
	if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes)
	{
		ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
	}


	/* interval图标 */
	atLayout[1].iTopLeftY  = atLayout[0].iBotRightY + iHeight / 2 + 1;
	atLayout[1].iBotRightY = atLayout[1].iTopLeftY + iHeight - 1;
	atLayout[1].iTopLeftX  = (ptPDispBuff->iXres - iWidth * 2) / 2;
	atLayout[1].iBotRightX = atLayout[1].iTopLeftX + iWidth * 2 - 1;

	iTmpTotalBytes = (atLayout[1].iBotRightX - atLayout[1].iTopLeftX + 1) * (atLayout[1].iBotRightY - atLayout[1].iTopLeftY + 1) * ptPDispBuff->iBpp / 8;
	if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes)
	{
		ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
	}

	/* return图标 */
	atLayout[2].iTopLeftY  = atLayout[1].iBotRightY + iHeight / 2 + 1;
	atLayout[2].iBotRightY = atLayout[2].iTopLeftY + iHeight - 1;
	atLayout[2].iTopLeftX  = (ptPDispBuff->iXres - iWidth) / 2;
	atLayout[2].iBotRightX = atLayout[2].iTopLeftX + iWidth - 1;

	iTmpTotalBytes = (atLayout[2].iBotRightX - atLayout[2].iTopLeftX + 1) * (atLayout[2].iBotRightY - atLayout[2].iTopLeftY + 1) * ptPDispBuff->iBpp / 8;
	if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes)
	{
		ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
	}

}


static void ShowSettingpage(PT_PageLayout ptPageLayout)
{
	PT_VideoMem ptVideoMem;
	
	PT_Layout atLayout = ptPageLayout->atLayout;
	int iError;	
	
	/* 1. 获得显存 */
	ptVideoMem = GetVideoMem(ID("setting"),1);
	if(ptVideoMem == NULL)
	{
		printf("GetVideoMem err\n");
		return ;
	}

	/* 2. 描画数据 */
	/* 如果还没有计算过各图标的坐标 */
	if(atLayout[0].iTopLeftX == 0)
	{
		SettingPagePageCalcLayout(ptPageLayout);
	}

	/* 创建页面 */
	iError = GeneratePage(ptPageLayout, ptVideoMem);
	if(iError!=0)
	{
		printf("GeneratePage err\n");
	}

	
	
	/* 3. 刷到设备上去 */
	FlushVideoMemToDev(ptVideoMem);
	
	/* 4. 解放显存 */
	PutVideoMem(ptVideoMem);


}


static void SettingPageRun(PT_PageParams ptParentPageParams)
{
	int iIndex;
	InputEvent tInputEvent;
	int bPressed = 0;
	int iIndexPressed = -1;
    T_PageParams tPageParams;

    tPageParams.iPageID = ID("setting");	

	
	/* 1. 显示页面 */
	ShowSettingpage(&g_tSettingPageLayout);	
	/* 2. 创建Prepare线程 */

	/* 3. 调用GetInputEvent获得输入事件，进而处理 */
	while (1)
	{
		iIndex = SettingPageGetInputEvent(&g_tSettingPageLayout,&tInputEvent);

		if(tInputEvent.iPressure == 0)
		{
			/* 如果是松开 */
			if(bPressed)
			{
				/* 曾经有按钮被按下 */
				ReleaseButton(&g_atSettingPageLayout[iIndexPressed]);
				bPressed = 0;
				
				if (iIndexPressed == iIndex) /* 按下和松开都是同一个按钮 */
				{
					switch (iIndexPressed)
					{
						case 0:/* "选择目录"按钮 */
						{
                            /* browse页面的Run参数含义: 
                             * 0 - 用于观看图片
                             * 1 - 用于浏览/选择文件夹, 点击文件无反应 
                             */
							GetPages("browse")->Run(&tPageParams);
							ShowSettingpage(&g_tSettingPageLayout);
						}
						break;
						case 1:
						{
							GetPages("interval")->Run(&tPageParams);
							ShowSettingpage(&g_tSettingPageLayout);	
						}
						break;
						case 2: /* 设置按钮 */
						{
							return ;
						}
						break;
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
					PressButton(&g_atSettingPageLayout[iIndexPressed]);
				}
			}

		}	
	}	
}


static int SettingPageGetInputEvent(PT_PageLayout ptPageLayout, PInputEvent ptInputEvent)
{
	return GenerateGetInputEvent(ptPageLayout,ptInputEvent);
}


int SettingPagePrepare(void)
{
	return 0;
}


static PageAction g_tSettingPageAction = {
	.name          = "setting",
	.Run           = SettingPageRun,
	.GetTouchInputEvent = SettingPageGetInputEvent,
	.Prepare       = SettingPagePrepare,
};


void RegisterSettingPage(void)
{
	RegisterPage(&g_tSettingPageAction);
}


