#ifndef _PIC_OPERATION_H
#define _PIC_OPERATION_H

#include <file.h>

typedef struct PixelDatas{
	int iWidth;
	int iHeight;
	int iBpp;
	int iLineBytes;
	int iTotalBytes;
	unsigned char *aucPixelDatas;
}PixelDatas,*PPixelDatas;

typedef struct PicFileParser{
	char *name;
	int (*isSupport)(PT_FileMap ptFileMap);
	int (*GetPixelDatas)(PT_FileMap ptFileMap, PPixelDatas ptPixelDatas);
	int (*FreePixelDatas)(PPixelDatas ptPixelDatas);
	struct PicFileParser  *PtNext;
}PicFileParser,*PPicFileParser;

#endif


