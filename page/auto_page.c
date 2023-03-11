#include <page_manager.h>
#include <Pic_operation.h>
#include <stdio.h>
#include <disp_manager.h>
#include <input_manager.h>
#include <render.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>



static pthread_t g_tAutoPlayThreadID;
static pthread_mutex_t g_tAutoPlayThreadMutex  = PTHREAD_MUTEX_INITIALIZER; /* 互斥量 */
static int g_bAutoPlayThreadShouldExit = 0;
static T_PageCfg g_tPageCfg;


static int g_iStartNumberToRecord = 0;
static int g_iCurFileNumber = 0;
static int g_iFileCountHaveGet = 0;
static int g_iFileCountTotal = 0;
static int g_iNextProcessFileIndex = 0;

#define FILE_COUNT 10
static char g_apstrFileNames[FILE_COUNT][256];

static void ResetAutoPlayFile(void)
{
    g_iStartNumberToRecord = 0;
    g_iCurFileNumber = 0;
    g_iFileCountHaveGet = 0;
    g_iFileCountTotal = 0;
    g_iNextProcessFileIndex = 0;
}

static int GetNextAutoPlayFile(char *strFileName)
{
    int iError;
    
    if (g_iNextProcessFileIndex < g_iFileCountHaveGet)
    {
        strncpy(strFileName, g_apstrFileNames[g_iNextProcessFileIndex], 256);
        g_iNextProcessFileIndex++;
        return 0;
    }
    else
    {
        g_iCurFileNumber    = 0;
        g_iFileCountHaveGet = 0;
        g_iFileCountTotal   = FILE_COUNT;
        g_iNextProcessFileIndex = 0;
        iError = GetFilesIndir(g_tPageCfg.strSeletedDir, &g_iStartNumberToRecord, &g_iCurFileNumber, &g_iFileCountHaveGet, g_iFileCountTotal, g_apstrFileNames);
        if (iError || (g_iNextProcessFileIndex >= g_iFileCountHaveGet))
        {
            /* 再次从头读起(连播模式下循环显示) */
            g_iStartNumberToRecord = 0;
            g_iCurFileNumber    = 0;
            g_iFileCountHaveGet = 0;
            g_iFileCountTotal = FILE_COUNT;
            g_iNextProcessFileIndex = 0;
            
            iError = GetFilesIndir(g_tPageCfg.strSeletedDir, &g_iStartNumberToRecord, &g_iCurFileNumber, &g_iFileCountHaveGet, g_iFileCountTotal, g_apstrFileNames);
        }
        
        if (iError == 0)
        {   
            if (g_iNextProcessFileIndex < g_iFileCountHaveGet)
            {
                strncpy(strFileName, g_apstrFileNames[g_iNextProcessFileIndex], 256);
                g_iNextProcessFileIndex++;
                return 0;
            }
        }
    }

    return -1;
}

/* bCur = 1 表示必须获得videomem, 因为这是马上就要在LCD上显示出来的
 * bCur = 0 表示这是做准备用的, 有可能无法获得videomem
 */
static PT_VideoMem PrepareNextPicture(int bCur)
{
	PixelDatas tOriginIconPixelDatas;
	PixelDatas tPicPixelDatas;
    PT_VideoMem ptVideoMem;
	int iError;
	PDispBuff ptDispBuff; 
    int iTopLeftX, iTopLeftY;
    float k;
    char strFileName[256];
    
	ptDispBuff = GetDisplayBuffer();
    
	/* 获得显存 */
	ptVideoMem = GetVideoMem(-1, bCur);
	if (ptVideoMem == NULL)
	{
		printf("can't get video mem for browse page!\n");
		return NULL;
	}
    ClearVideoMem(ptVideoMem, COLOR_BACKGROUND);

    while (1)
    {
        iError = GetNextAutoPlayFile(strFileName);
        if (iError)
        {
            printf("GetNextAutoPlayFile error\n");
            PutVideoMem(ptVideoMem);
            return NULL;
        }
        
        iError = GetPixelDatasFrmFile(strFileName, &tOriginIconPixelDatas);
        if (0 == iError)
        {
            break;
        }
    }

    /* 把图片按比例缩放到LCD屏幕上, 居中显示 */
    k = (float)tOriginIconPixelDatas.iHeight / tOriginIconPixelDatas.iWidth;
    tPicPixelDatas.iWidth  = ptDispBuff->iXres;
    tPicPixelDatas.iHeight = ptDispBuff->iXres * k;
    if (tPicPixelDatas.iHeight > ptDispBuff->iYres)
    {
        tPicPixelDatas.iWidth  = ptDispBuff->iYres / k;
        tPicPixelDatas.iHeight = ptDispBuff->iYres;
    }
    tPicPixelDatas.iBpp        = ptDispBuff->iBpp;
    tPicPixelDatas.iLineBytes  = tPicPixelDatas.iWidth * tPicPixelDatas.iBpp / 8;
    tPicPixelDatas.iTotalBytes = tPicPixelDatas.iLineBytes * tPicPixelDatas.iHeight;
    tPicPixelDatas.aucPixelDatas = malloc(tPicPixelDatas.iTotalBytes);
    if (tPicPixelDatas.aucPixelDatas == NULL)
    {
        PutVideoMem(ptVideoMem);
        return NULL;
    }

    PicZoom(&tOriginIconPixelDatas, &tPicPixelDatas);

    /* 算出居中显示时左上角坐标 */
    iTopLeftX = (ptDispBuff->iXres - tPicPixelDatas.iWidth) / 2;
    iTopLeftY = (ptDispBuff->iYres - tPicPixelDatas.iHeight) / 2;
    PicMerge(iTopLeftX, iTopLeftY, &tPicPixelDatas, &ptVideoMem->tPixelDatas);
    FreePixelDatasFrmFile(&tOriginIconPixelDatas);
    free(tPicPixelDatas.aucPixelDatas);

    return ptVideoMem;
}

static void *AutoPlayThreadFunction(void *pVoid)
{
    int bFirst = 1;
    int bExit;
    PT_VideoMem ptVideoMem;

    ResetAutoPlayFile();
    
    while (1)
    {
        /* 1. 先判断是否要退出 */
        pthread_mutex_lock(&g_tAutoPlayThreadMutex);
        bExit = g_bAutoPlayThreadShouldExit;
        pthread_mutex_unlock(&g_tAutoPlayThreadMutex);

        if (bExit)
        {
            return NULL;
        }

        /* 2. 准备要显示的图片 */
        ptVideoMem = PrepareNextPicture(0);

        /* 3. 时间到后就显示出来 */

        if (!bFirst)
        {
            sleep(g_tPageCfg.iIntervalSecond);       /* 先用休眠来代替 */
        }
        bFirst = 0;
        
        if (ptVideoMem == NULL)
        {
            ptVideoMem = PrepareNextPicture(1);
        }

    	/* 刷到设备上去 */
    	FlushVideoMemToDev(ptVideoMem);

    	/* 解放显存 */
    	PutVideoMem(ptVideoMem);        

    }
    return NULL;
}

static void autoPageRun(PT_PageParams ptParentPageParams)
{
	InputEvent tInputEvent;
	int iRet;
    
    g_bAutoPlayThreadShouldExit = 0;

    /* 获得配置值: 显示哪一个目录, 显示图片的间隔 */
    GetPageCfg(&g_tPageCfg);
        
    /* 1. 启动一个线程来连续显示图片 */
    pthread_create(&g_tAutoPlayThreadID, NULL, AutoPlayThreadFunction, NULL);

    /* 2. 当前线程等待触摸屏输入, 先做简单点: 如果点击了触摸屏, 让线程退出 */
    while (1)
    {
        iRet = GetInputEvent(&tInputEvent);
        if (0 == iRet)
        {
            pthread_mutex_lock(&g_tAutoPlayThreadMutex);
            g_bAutoPlayThreadShouldExit = 1;   /* AutoPlayThreadFunction线程检测到这个变量为1后会退出 */
            pthread_mutex_unlock(&g_tAutoPlayThreadMutex);
            pthread_join(g_tAutoPlayThreadID, NULL);  /* 等待子线程退出 */
            return;
        }
    }

}



static PageAction g_tautoPageAction = {
	.name          = "auto",
	.Run           = autoPageRun,
	//.GetTouchInputEvent = autoPageGetInputEvent,
	//.Prepare       = autoPagePrepare,
};


void RegisterautoPage(void)
{
	RegisterPage(&g_tautoPageAction);
}


