#include <page_manager.h>
#include <Pic_operation.h>
#include <stdio.h>
#include <disp_manager.h>
#include <input_manager.h>
#include <render.h>
#include <stdlib.h>

static T_Layout g_atMainPageLayout[] = {
	{0, 0, 0, 0, "browse_mode.bmp"},
	{0, 0, 0, 0, "continue_mod.bmp"},
	{0, 0, 0, 0, "setting.bmp"},
	{0, 0, 0, 0, NULL},
};

static T_PageLayout g_tMainPageLayout = {
	.iMaxTotalBytes = 0,
	.atLayout       = g_atMainPageLayout,
};


static int MainPageGetInputEvent(PT_PageLayout ptPageLayout, PInputEvent ptInputEvent);

static void  MainPagePageCalcLayout(PT_PageLayout ptPageLayout)
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
	/* browse_mode图标 */
	atLayout[0].iTopLeftY  = iStartY;
	atLayout[0].iBotRightY = atLayout[0].iTopLeftY + iHeight - 1;
	atLayout[0].iTopLeftX  = (ptPDispBuff->iXres - iWidth * 2) / 2;
	atLayout[0].iBotRightX = atLayout[0].iTopLeftX + iWidth * 2 - 1;

	iTmpTotalBytes = (atLayout[0].iBotRightX - atLayout[0].iTopLeftX + 1) * (atLayout[0].iBotRightY - atLayout[0].iTopLeftY + 1) * ptPDispBuff->iBpp / 8;
	if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes)
	{
		ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
	}

	/* continue_mod图标 */
	atLayout[1].iTopLeftY  = atLayout[0].iBotRightY + iHeight / 2 + 1;
	atLayout[1].iBotRightY = atLayout[1].iTopLeftY + iHeight - 1;
	atLayout[1].iTopLeftX  = (ptPDispBuff->iXres - iWidth * 2) / 2;
	atLayout[1].iBotRightX = atLayout[1].iTopLeftX + iWidth * 2 - 1;

	iTmpTotalBytes = (atLayout[1].iBotRightX - atLayout[1].iTopLeftX + 1) * (atLayout[1].iBotRightY - atLayout[1].iTopLeftY + 1) * ptPDispBuff->iBpp / 8;
	if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes)
	{
		ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
	}


	/* setting图标 */
	atLayout[2].iTopLeftY  = atLayout[1].iBotRightY + iHeight / 2 + 1;
	atLayout[2].iBotRightY = atLayout[2].iTopLeftY + iHeight - 1;
	atLayout[2].iTopLeftX  = (ptPDispBuff->iXres - iWidth * 2) / 2;
	atLayout[2].iBotRightX = atLayout[2].iTopLeftX + iWidth * 2 - 1;

	iTmpTotalBytes = (atLayout[2].iBotRightX - atLayout[2].iTopLeftX + 1) * (atLayout[2].iBotRightY - atLayout[2].iTopLeftY + 1) * ptPDispBuff->iBpp / 8;
	if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes)
	{
		ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
	}	
}


static void ShowMainpage(PT_PageLayout ptPageLayout)
{
	PT_VideoMem ptVideoMem;
	
	PT_Layout atLayout = ptPageLayout->atLayout;
	int iError;

	
	/* 1. 获得显存 */
	ptVideoMem = GetVideoMem(ID("main"),1);
	if(ptVideoMem == NULL)
	{
		printf("GetVideoMem err\n");
		return ;
	}

	/* 2. 描画数据 */
	/* 如果还没有计算过各图标的坐标 */
	if(atLayout[0].iTopLeftX == 0)
	{
		MainPagePageCalcLayout(ptPageLayout);
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


static void MainPageRun(PT_PageParams ptParentPageParams)
{
	int iIndex;
	InputEvent tInputEvent;
	int bPressed = 0;
	int iIndexPressed = -1;
    T_PageParams tPageParams;

    tPageParams.iPageID = ID("main");	

	
	/* 1. 显示页面 */
	ShowMainpage(&g_tMainPageLayout);	
	/* 2. 创建Prepare线程 */

	/* 3. 调用GetInputEvent获得输入事件，进而处理 */
	while (1)
	{
		iIndex = MainPageGetInputEvent(&g_tMainPageLayout,&tInputEvent);

		if(tInputEvent.iPressure == 0)
		{
			/* 如果是松开 */
			if(bPressed)
			{
				/* 曾经有按钮被按下 */
				ReleaseButton(&g_atMainPageLayout[iIndexPressed]);
				bPressed = 0;
				
				if (iIndexPressed == iIndex) /* 按下和松开都是同一个按钮 */
				{
					switch (iIndexPressed)
					{
						case 0:
						{
							tPageParams.strCurPictureFile[0] = '\0';
							GetPages("manual")->Run(&tPageParams);
							ShowMainpage(&g_tMainPageLayout);
							break;
						}
						case 1:
						{
							GetPages("auto")->Run(&tPageParams);
							ShowMainpage(&g_tMainPageLayout);							
							break;
						}
						case 2: /* 设置按钮 */
						{
							GetPages("setting")->Run(&tPageParams);

							/* 从设置页面返回后显示当首的主页面 */
							ShowMainpage(&g_tMainPageLayout);

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
					PressButton(&g_atMainPageLayout[iIndexPressed]);
				}
			}

		}	
	}
}


static int MainPageGetInputEvent(PT_PageLayout ptPageLayout, PInputEvent ptInputEvent)
{
	return GenerateGetInputEvent(ptPageLayout,ptInputEvent);	
}

int MainPagePrepare(void)
{
	return 0;
}


static PageAction g_tMainPageAction = {
	.name          = "main",
	.Run           = MainPageRun,
	.GetTouchInputEvent = MainPageGetInputEvent,
	.Prepare       = MainPagePrepare,
};


void RegisterMainPage(void)
{
	RegisterPage(&g_tMainPageAction);
}

