#ifndef _ENCODING_MANAGER_H
#define _ENCODING_MANAGER_H

#include <disp_manager.h>
#include <font_manager.h>

typedef struct EncodingOpr{
	char *name;
	int iHeadLen;
	PFontOpr ptFontOprSupportedHead;
	int (*isSupport)(unsigned char *pucBufHead);
	int (*GetCodeFrmBuf)(unsigned char *pucBufStart, unsigned char *pucBufEnd, unsigned int *pdwCode);
	struct EncodingOpr *ptNext;
}EncodingOpr,*PEncodingOpr;

void RegisterEncodingOpr(PEncodingOpr ptEncodingOpr);
void ShowEncodingOpr(void);
int AddFontOprForEncoding(PEncodingOpr ptEncodingOpr, PFontOpr ptFontOpr);
PEncodingOpr SelectDefaultEncodingOpr(unsigned char *pucFileBufHead);
PEncodingOpr SelectEncodingOprByName(char *name);


void AsciiEncodingInit(void);
void Utf8EncodingInit(void);
void Utf16beEncodingInit(void);
void Utf16leEncodingInit(void);
int EncodingInit(void);
int DelFontOprFrmEncoding(PEncodingOpr ptEncodingOpr, PFontOpr ptFontOpr);



#endif


