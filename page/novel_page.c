#include <page_manager.h>
#include <Pic_operation.h>
#include <stdio.h>
#include <disp_manager.h>
#include <input_manager.h>
#include <render.h>
#include <stdlib.h>
#include <draw.h>

static T_Layout g_atnovelPageLayout[] = {
	{0, 0, 0, 0, "return.bmp"},
	{0, 0, 0, 0, "pre_pic.bmp"},
    {0, 0, 0, 0, "next_pic.bmp"},
	{0, 0, 0, 0, NULL},
};

static T_PageLayout g_tnovelPageLayout = {
	.iMaxTotalBytes = 0,
	.atLayout       = g_atnovelPageLayout,
};


static int novelPageGetInputEvent(PT_PageLayout ptPageLayout, PInputEvent ptInputEvent);

static void  novelPagePageCalcLayout(PT_PageLayout ptPageLayout)
{
	int iWidth;
	int iHeight;
	PDispBuff ptPDispBuff = GetDisplayBuffer();
	int iTmpTotalBytes;
	PT_Layout atLayout;
	int i;

	atLayout = ptPageLayout->atLayout;
	ptPageLayout->iBpp = ptPDispBuff->iBpp;
	iHeight  = ptPDispBuff->iYres / 6;
	iWidth = iHeight;

	/* return图标 */
	atLayout[0].iTopLeftY  = 0;
	atLayout[0].iBotRightY = atLayout[0].iTopLeftY + iHeight - 1;
	atLayout[0].iTopLeftX  = 0;
	atLayout[0].iBotRightX = atLayout[0].iTopLeftX + iWidth - 1;
	
    /* 其他2个图标 */
    for (i = 1; i < 3; i++)
    {
		atLayout[i].iTopLeftY  = atLayout[i-1].iBotRightY+ 1;
		atLayout[i].iBotRightY = atLayout[i].iTopLeftY + iHeight - 1;
		atLayout[i].iTopLeftX  = 0;
		atLayout[i].iBotRightX = atLayout[i].iTopLeftX + iWidth - 1;
    }		

	i = 0;
	while (atLayout[i].strIconName)
	{
		iTmpTotalBytes = (atLayout[i].iBotRightX - atLayout[i].iTopLeftX + 1) * (atLayout[i].iBotRightY - atLayout[i].iTopLeftY + 1) * ptPDispBuff->iBpp / 8;
		if (ptPageLayout->iMaxTotalBytes < iTmpTotalBytes)
		{
			ptPageLayout->iMaxTotalBytes = iTmpTotalBytes;
		}
		i++;
	}	
}


static void Shownovelpage(PT_PageLayout ptPageLayout)
{
	PT_VideoMem ptVideoMem;
	
	PT_Layout atLayout = ptPageLayout->atLayout;
	int iError;

	
	/* 1. 获得显存 */
	ptVideoMem = GetVideoMem(ID("novel"),1);
	if(ptVideoMem == NULL)
	{
		printf("GetVideoMem err\n");
		return ;
	}

	/* 2. 描画数据 */
	/* 如果还没有计算过各图标的坐标 */
	if(atLayout[0].iTopLeftX == 0)
	{
		novelPagePageCalcLayout(ptPageLayout);
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


static void novelPageRun(PT_PageParams ptParentPageParams)
{
	int iIndex;
	InputEvent tInputEvent;
	int bPressed = 0;
	int iIndexPressed = -1;
    T_PageParams tPageParams;

    tPageParams.iPageID = ID("novel");	

	
	/* 1. 显示页面 */
	Shownovelpage(&g_tnovelPageLayout);
	SetFontsDetail(60);
	ShowNextPage();
	/* 2. 创建Prepare线程 */

	/* 3. 调用GetInputEvent获得输入事件，进而处理 */
	while (1)
	{
		iIndex = novelPageGetInputEvent(&g_tnovelPageLayout,&tInputEvent);

		if(tInputEvent.iPressure == 0)
		{
			/* 如果是松开 */
			if(bPressed)
			{
				/* 曾经有按钮被按下 */
				ReleaseButton(&g_atnovelPageLayout[iIndexPressed]);
				bPressed = 0;
				
				if (iIndexPressed == iIndex) /* 按下和松开都是同一个按钮 */
				{
					switch (iIndexPressed)
					{
						case 0:
						{
							return ;
							break;
						}
						case 1:
						{
							ShowPrePage();						
							break;
						}
						case 2: /* 设置按钮 */
						{
							
							ShowNextPage();	
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
					PressButton(&g_atnovelPageLayout[iIndexPressed]);
				}
			}

		}	
	}
}


static int novelPageGetInputEvent(PT_PageLayout ptPageLayout, PInputEvent ptInputEvent)
{
	return GenerateGetInputEvent(ptPageLayout,ptInputEvent);	
}

int novelPagePrepare(void)
{
	return 0;
}


static PageAction g_tnovelPageAction = {
	.name          = "novel",
	.Run           = novelPageRun,
	.GetTouchInputEvent = novelPageGetInputEvent,
	.Prepare       = novelPagePrepare,
};


void RegisternovelPage(void)
{
	RegisterPage(&g_tnovelPageAction);
}

