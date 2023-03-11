#ifndef _DRAW_H
#define _DRAW_H

#include <font_manager.h>

typedef struct PageDesc {
	int iPage;
	unsigned char *pucLcdFirstPosAtFile;
	unsigned char *pucLcdNextPageFirstPosAtFile;
	struct PageDesc *ptPrePage;
	struct PageDesc *ptNextPage;
} T_PageDesc, *PT_PageDesc;


int OpenTextFile(char *pcFileName);
//int SetFontsDetail(char *pcHZKFile, char *pcFileFreetype, unsigned int dwFontSize);
int SelectAndInitDisplay(char *name);
void ExitDisplay();
int IncLcdX(int iX);
int IncLcdY(int iY);
int RelocateFontPos(PFontBitMap ptFontBitMap);
int ShowOnePage(unsigned char *pucTextFileMemCurPos);
void RecordPage(PT_PageDesc ptPageNew);
int ShowNextPage(void);
int ShowPrePage(void);
int isFiletxtSupported(char *strFileName);
int SetFontsDetail(unsigned int dwFontSize);





int DrawInit(void);


#endif


