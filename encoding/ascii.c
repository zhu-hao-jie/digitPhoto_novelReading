#include <encoding_manager.h>
#include <string.h>
#include <stdlib.h>


/* 判断是否是ascii.c编码格式 
*return 1表示是ascii
*/
static int isAsciiCoding(unsigned char *pucBufHead)
{
	const char aStrUtf8[]    = {0xEF, 0xBB, 0xBF, 0};
	const char aStrUtf16le[] = {0xFF, 0xFE, 0};
	const char aStrUtf16be[] = {0xFE, 0xFF, 0};
	
	if (strncmp((const char*)pucBufHead, aStrUtf8, 3) == 0)
	{
		/* UTF-8 */
		return 0;
	}
	else if (strncmp((const char*)pucBufHead, aStrUtf16le, 2) == 0)
	{
		/* UTF-16 little endian */
		return 0;
	}
	else if (strncmp((const char*)pucBufHead, aStrUtf16be, 2) == 0)
	{
		/* UTF-16 big endian */
		return 0;
	}
	else
	{
		return 1;
	}
}

static int AsciiGetCodeFrmBuf(unsigned char *pucBufStart, unsigned char *pucBufEnd, unsigned int *pdwCode)
{
	unsigned char *pucBuf = pucBufStart;
	unsigned char c = *pucBuf;
	
	if ((pucBuf < pucBufEnd) && (c < (unsigned char)0x80))
	{
		/* 返回ASCII码 */
		*pdwCode = (unsigned int)c;
		return 1;
	}

	if (((pucBuf + 1) < pucBufEnd) && (c >= (unsigned char)0x80))
	{
		/* 返回GBK码 */
		*pdwCode = pucBuf[0] + (((unsigned int)pucBuf[1])<<8);
		return 2;
	}

	if (pucBuf < pucBufEnd)
	{
		/* 可能文件有损坏, 但是还是返回一个码, 即使它是错误的 */
		*pdwCode = (unsigned int)c;
		return 1;
	}
	else
	{
		/* 文件处理完毕 */
		return 0;
	}
}


static EncodingOpr g_tAsciiEncodingOpr={
	.name          = "ascii",
	.iHeadLen      = 0,
	.isSupport     = isAsciiCoding,
	.GetCodeFrmBuf = AsciiGetCodeFrmBuf,	
};

/* 注册Ascii编码操作方法 */
void AsciiEncodingInit(void)
{
	AddFontOprForEncoding(&g_tAsciiEncodingOpr,GetFontOpr("freetype"));
	AddFontOprForEncoding(&g_tAsciiEncodingOpr, GetFontOpr("ascii"));
	AddFontOprForEncoding(&g_tAsciiEncodingOpr, GetFontOpr("gbk"));	
	RegisterEncodingOpr(&g_tAsciiEncodingOpr);
}





