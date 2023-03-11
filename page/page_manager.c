#include <page_manager.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <render.h>
#include <Pic_operation.h>
#include <input_manager.h>

static PPageAction g_Pages = NULL;

extern void GetSelectedDir(char *strSeletedDir);
extern void GetIntervalTime(int *piIntervalSecond);



/* 将页面注册进链表 */
void RegisterPage(PPageAction ptPage)
{
	ptPage->ptNext = g_Pages;
	g_Pages = ptPage;
}

void ShowPages(void)
{
	int i = 0;
	PPageAction ptTmp = g_Pages;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

PPageAction GetPages(char *name)
{
	PPageAction ptTmp = g_Pages;

	while (ptTmp)
	{
		if(strcmp(ptTmp->name,name)==0)
		{
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	return NULL;
}

int ID(char *strName)
{
	return (int)(strName[0]) + (int)(strName[1]) + (int)(strName[2]) + (int)(strName[3]);
}

int GeneratePage(PT_PageLayout ptPageLayout, PT_VideoMem ptVideoMem)
{
	PixelDatas tOriginIconPixelDatas;
	PixelDatas tIconPixelDatas;
	int iError;
	PT_Layout atLayout = ptPageLayout->atLayout;
		
	/* 描画数据 */
	if (ptVideoMem->ePicState != PS_GENERATED)
	{
		ClearVideoMem(ptVideoMem, 0);

		tIconPixelDatas.iBpp          = ptPageLayout->iBpp;
		tIconPixelDatas.aucPixelDatas = malloc(ptPageLayout->iMaxTotalBytes);
		if (tIconPixelDatas.aucPixelDatas == NULL)
		{
			return -1;
		}
		
		while (atLayout->strIconName)
		{
			iError = GetPixelDatasForIcon(atLayout->strIconName, &tOriginIconPixelDatas);
			if (iError)
			{
				printf("GetPixelDatasForIcon %s error!\n", atLayout->strIconName);
				return -1;
			}
 			tIconPixelDatas.iHeight = atLayout->iBotRightY - atLayout->iTopLeftY + 1;
			tIconPixelDatas.iWidth  = atLayout->iBotRightX - atLayout->iTopLeftX+ 1;
			tIconPixelDatas.iLineBytes  = tIconPixelDatas.iWidth * tIconPixelDatas.iBpp / 8;
			tIconPixelDatas.iTotalBytes = tIconPixelDatas.iLineBytes * tIconPixelDatas.iHeight;
 			PicZoom(&tOriginIconPixelDatas, &tIconPixelDatas);
 			PicMerge(atLayout->iTopLeftX, atLayout->iTopLeftY, &tIconPixelDatas, &ptVideoMem->tPixelDatas);
 			FreePixelDatasForIcon(&tOriginIconPixelDatas);
 			atLayout++;
		}
		free(tIconPixelDatas.aucPixelDatas);
		ptVideoMem->ePicState = PS_GENERATED;
	}

	return 0;
}

int GenerateGetInputEvent(PT_PageLayout ptPageLayout, PInputEvent ptInputEvent)
{
	InputEvent tInputEvent;
	int i = 0;
	int err;
	PT_Layout atLayout = ptPageLayout->atLayout;
	/* 获得原始的触摸屏数据 
	 * 它是调用input_manager.c的函数，此函数会让当前线否休眠
	 * 当触摸屏线程获得数据后，会把它唤醒
	 */
	err = GetInputEvent(&tInputEvent);
	if(err)
	{
		printf("GetInputEvent err\n");
		return -1;
	}

	if(tInputEvent.iType != INPUT_TYPE_TOUCH)
	{
		printf("this is not INPUT_TYPE_TOUCH\n");
		return -1;
	}

	*ptInputEvent = tInputEvent;
	
	/* 处理数据 */
	/* 确定触点位于哪一个按钮上 */
	while (atLayout[i].strIconName)
	{
//		printf("iTopLeftX:%d iTopLeftY:%d\n",atLayout[i].iTopLeftX,atLayout[i].iTopLeftY);
//		printf("iBotRightX:%d iBotRightY:%d\n",atLayout[i].iBotRightX,atLayout[i].iBotRightY);
		
		if ((tInputEvent.iX >= atLayout[i].iTopLeftX) && (tInputEvent.iX <= atLayout[i].iBotRightX) && \
			 (tInputEvent.iY >= atLayout[i].iTopLeftY) && (tInputEvent.iY <= atLayout[i].iBotRightY))
		{
			/* 找到了被点中的按钮 */
			return i;
		}
		else
		{
			i++;
		}			
	}

	/* 触点没有落在按钮上 */
	return -1;	

}


void GetPageCfg(PT_PageCfg ptPageCfg)
{
    GetSelectedDir(ptPageCfg->strSeletedDir);
    GetIntervalTime(&ptPageCfg->iIntervalSecond);
}


void PagesInit(void)
{
	extern void RegisterMainPage(void);
	RegisterMainPage();
	extern void RegisterSettingPage(void);
	RegisterSettingPage();
	extern void RegisterintervalPage(void);
	RegisterintervalPage();
	extern void RegisterbrowsePage(void);
	RegisterbrowsePage();
	extern void RegisterautoPage(void);
	RegisterautoPage();
	extern void RegistermanualPage(void);
	RegistermanualPage();
	extern void RegisternovelPage(void);
	RegisternovelPage();
}


