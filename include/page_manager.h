#ifndef _PAGE_MANAGER_H
#define _PAGE_MANAGER_H

#include <input_manager.h>
#include <disp_manager.h>

//#define ID(name)   (int(name[0]) + int(name[1]) + int(name[2]) + int(name[3]))

typedef struct PageParams {
    int iPageID;                  /* 页面的ID */
    char strCurPictureFile[256];  /* 要处理的第1个图片文件 */
}T_PageParams, *PT_PageParams;


typedef struct Layout {
	int iTopLeftX;
	int iTopLeftY;
	int iBotRightX;
	int iBotRightY;
	char *strIconName;
}T_Layout, *PT_Layout;

typedef struct PageLayout {
	int iTopLeftX;        /* 这个区域的左上角、右下角坐标 */
	int iTopLeftY;
	int iBotRightX;
	int iBotRightY;
	int iBpp;
	int iMaxTotalBytes;
	PT_Layout atLayout;  /* 这个区域分成好几个小区域 */
}T_PageLayout, *PT_PageLayout;

typedef struct PageCfg {
    int iIntervalSecond;
    char strSeletedDir[256];
}T_PageCfg, *PT_PageCfg;


typedef struct PageAction {
	char *name;
	void (*Run)(PT_PageParams ptParentPageParams);
	int (*GetTouchInputEvent)(PT_PageLayout ptPageLayout, PInputEvent ptInputEvent);
	int (*Prepare)(void);
	struct PageAction *ptNext;
}PageAction, *PPageAction;




void RegisterPage(PPageAction ptPage);

void ShowPages(void);
PPageAction GetPages(char *name);
void PagesInit(void);
int ID(char *strName);
int GeneratePage(PT_PageLayout ptPageLayout, PT_VideoMem ptVideoMem);
int GenerateGetInputEvent(PT_PageLayout ptPageLayout, PInputEvent ptInputEvent);
void GetPageCfg(PT_PageCfg ptPageCfg);



#endif


