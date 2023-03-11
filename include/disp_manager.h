#ifndef _DISP_MANAGER_H
#define _DISP_MANAGER_H

#include <common.h>
#include <font_manager.h>
#include <Pic_operation.h>
#include <disp_manager.h>


typedef struct DispBuff {
	int iXres;
	int iYres;
	int iBpp;
	unsigned char *buff;
}DispBuff, *PDispBuff;

typedef enum {
	VMS_FREE = 0,/* 可用状态 */
	VMS_USED_FOR_PREPARE,/* 准备中的 */
	VMS_USED_FOR_CUR,/* 被占用的 */
}E_VideoMemState;

typedef enum {
	PS_BLANK = 0,/* 无图像数据 */
	PS_GENERATING,/* 准备中 */
	PS_GENERATED,/* 准备好了 */	
}E_PicState;


typedef struct VideoMem {
	int iID;/* 每一块内存都有一个ID */
	int bDevFrameBuffer;/* 标志是否是FrameBuffer */
	E_VideoMemState eVideoMemState;
	E_PicState ePicState;
	PixelDatas tPixelDatas;
	struct VideoMem *ptNext;
}T_VideoMem, *PT_VideoMem;


typedef struct DispOpr {
	char *name;
	int (*DeviceInit)(void);
	int (*DeviceExit)(void);
	int (*GetBuffer)(PDispBuff ptDispBuff);
	int (*FlushRegion)(PRegion ptRegion, PDispBuff ptDispBuff);
	int (*ShowPage)(PT_VideoMem ptVideoMem);
	struct DispOpr *ptNext;
}DispOpr, *PDispOpr;

void RegisterDisplay(PDispOpr ptDispOpr);

void DisplaySystemRegister(void);
int SelectDefaultDisplay(char *name);
int InitDefaultDisplay(void);
int PutPixel(int x, int y, unsigned int dwColor);
int FlushDisplayRegion(PRegion ptRegion, PDispBuff ptDispBuff);
PDispBuff GetDisplayBuffer(void);
int DrawFontBitMap(PFontBitMap ptFontBitMap, unsigned int dwColor);
void DrawRegion(PRegion ptRegion, unsigned int dwColor);
void DrawTextInRegionCentral(char *name, PRegion ptRegion, unsigned int dwColor);
void CleanScreen(unsigned int dwColor);
void ShowDispOpr(void);
void ExitDefaultDisplay(void);
PDispOpr GetDefaultDisplay(void);


int AllocVideoMem(int iNum);
PT_VideoMem GetVideoMem(int iID, int bCur);
void PutVideoMem(PT_VideoMem ptVideoMem);
void ClearVideoMem(PT_VideoMem ptVideoMem, unsigned int dwColor);
PT_VideoMem GetVideoMemDev();
//void ClearVideoMemRegion(PT_VideoMem ptVideoMem, PT_Layout ptLayout, unsigned int dwColor);
//void PicMergeRegion(PT_VideoMem ptVideoMem, PT_Layout ptLayout, unsigned int dwColor);



#endif










