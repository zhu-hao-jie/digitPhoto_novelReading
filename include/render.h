#ifndef _RENDER_H
#define _RENDER_H

#include <Pic_operation.h>
#include <disp_manager.h>
#include <page_manager.h>
#include <font_manager.h>

#define ICON_PATH "/etc/digitpic/icons"

#define COLOR_BACKGROUND   0xE7DBB5
#define COLOR_FOREGROUND   0x514438




int PicZoom(PPixelDatas ptOriginPic, PPixelDatas ptZoomPic);
int PicMerge(int iX, int iY, PPixelDatas ptSmallPic, PPixelDatas ptBigPic);
int GetPixelDatasForIcon(char *strFileName, PPixelDatas ptPixelDatas);
void FreePixelDatasForIcon(PPixelDatas ptPixelDatas);
void FlushVideoMemToDev(PT_VideoMem ptVideoMem);

void ReleaseButton(PT_Layout ptLayout);
void PressButton(PT_Layout ptLayout);
void ClearRectangleInVideoMem(int iTopLeftX, int iTopLeftY, int iBotRightX, int iBotRightY, PT_VideoMem ptVideoMem, unsigned int dwColor);

int MergerStringToCenterOfRectangleInVideoMem(int iTopLeftX, int iTopLeftY, int iBotRightX, int iBotRightY, unsigned char *pucTextString, PT_VideoMem ptVideoMem);
int GetPixelDatasFrmFile(char *strFileName, PPixelDatas ptPixelDatas);
void FreePixelDatasFrmFile(PPixelDatas ptPixelDatas);
int isPictureFileSupported(char *strFileName);

int PicMergeRegion(int iStartXofNewPic, int iStartYofNewPic, int iStartXofOldPic, int iStartYofOldPic, int iWidth, int iHeight, PPixelDatas ptNewPic, PPixelDatas ptOldPic);

int isFiletxtSupported(char *strFileName);



#endif



