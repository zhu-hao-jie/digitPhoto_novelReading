#ifndef _FONT_MANAGER_H
#define _FONT_MANAGER_H

#include <common.h>

typedef struct FontBitMap {
	int iXLeft;
	int iYTop;
	int iXMax;
	int iYMax;
	int iBpp;
	int iPitch;   /* 对于单色位图, 两行象素之间的跨度 */
	int iCurOriginX;
	int iCurOriginY;
	int iNextOriginX;
	int iNextOriginY;
	unsigned char *pucBuffer;
}FontBitMap, *PFontBitMap;


typedef struct FontOpr {
	char *name;
	int (*FontInit)(char *aFineName);
	int (*SetFontSize)(int iFontSize);
	int (*GetFontBitMap)(unsigned int dwCode, PFontBitMap ptFontBitMap);
	int (*GetStringRegionCar)(char *str, PRegionCartesian ptRegionCar);
	struct FontOpr *ptNext;
}FontOpr, *PFontOpr;

void RegisterFont(PFontOpr ptFontOpr);

void FontSystemRegister(void);

int SelectAndInitFont(char *aFontOprName, char *aFontFileName);
int SetFontSize(int iFontSize);
int GetFontBitMap(unsigned int dwCode, PFontBitMap ptFontBitMap);
int GetStringRegionCar(char *str, PRegionCartesian ptRegionCar);
void ShowFontOpr(void);
PFontOpr GetFontOpr(char *pcName);


#endif




