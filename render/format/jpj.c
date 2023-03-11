#include <Pic_operation.h>
#include <picfmt_manager.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <jpeglib.h>
#include <stdio.h>
#include <jconfig.h>
#include <jmorecfg.h>



static int isJPGFormat(PT_FileMap ptFileMap)
{
	struct jpeg_decompress_struct tDInfo;
	struct jpeg_error_mgr tJErr;
    int iRet;

	// 分配和初始化一个decompression结构体
	tDInfo.err = jpeg_std_error(&tJErr);
	jpeg_create_decompress(&tDInfo);

	// 用jpeg_read_header获得jpg信息
	jpeg_stdio_src(&tDInfo, ptFileMap->tFp);

    iRet = jpeg_read_header(&tDInfo, TRUE);
	jpeg_abort_decompress(&tDInfo);

    return (iRet == JPEG_HEADER_OK);
}

static int CovertOneLine(int iWidth, int iSrcBpp, int iDstBpp, unsigned char *pudSrcDatas, unsigned char *pudDstDatas)
{
	unsigned int dwRed;
	unsigned int dwGreen;
	unsigned int dwBlue;
	unsigned int dwColor;

	unsigned short *pwDstDatas16bpp = (unsigned short *)pudDstDatas;
	unsigned int   *pwDstDatas32bpp = (unsigned int *)pudDstDatas;

	int i;
	int pos = 0;

	if (iSrcBpp != 24)
	{
		return -1;
	}

	if (iDstBpp == 24)
	{
		memcpy(pudDstDatas, pudSrcDatas, iWidth*3);
	}
	else
	{
		for (i = 0; i < iWidth; i++)
		{
			dwRed   = pudSrcDatas[pos++];
			dwGreen = pudSrcDatas[pos++];
			dwBlue  = pudSrcDatas[pos++];
			if (iDstBpp == 32)
			{
				dwColor = (dwRed << 16) | (dwGreen << 8) | dwBlue;
				*pwDstDatas32bpp = dwColor;
				pwDstDatas32bpp++;
			}
			else if (iDstBpp == 16)
			{
				/* 565 */
				dwRed   = dwRed >> 3;
				dwGreen = dwGreen >> 2;
				dwBlue  = dwBlue >> 3;
				dwColor = (dwRed << 11) | (dwGreen << 5) | (dwBlue);
				*pwDstDatas16bpp = dwColor;
				pwDstDatas16bpp++;
			}
		}
	}
	return 0;
}


/*
 * ptPixelDatas->iBpp 是输入的参数, 它决定从JPG得到的数据要转换为该格式
 */
static int GetPixelDatasFrmJPG(PT_FileMap ptFileMap, PPixelDatas ptPixelDatas)
{
	struct jpeg_decompress_struct tDInfo;
	struct jpeg_error_mgr tJErr;
    int iRet;
    int iRowStride;
    unsigned char *aucLineBuffer;
    unsigned char *pucDest;
	if(iRet)
	{
		;
	}

    fseek(ptFileMap->tFp, 0, SEEK_SET);

	// 分配和初始化一个decompression结构体
	tDInfo.err = jpeg_std_error(&tJErr);
	jpeg_create_decompress(&tDInfo);

	// 用jpeg_read_header获得jpg信息
	jpeg_stdio_src(&tDInfo, ptFileMap->tFp);

    iRet = jpeg_read_header(&tDInfo, TRUE);

	// 设置解压参数,比如放大、缩小
    tDInfo.scale_num = tDInfo.scale_denom = 1;
    
	// 启动解压：jpeg_start_decompress	
	jpeg_start_decompress(&tDInfo);
    
	// 一行的数据长度
	iRowStride = tDInfo.output_width * tDInfo.output_components;
	aucLineBuffer = malloc(iRowStride);

    if (NULL == aucLineBuffer)
    {
        return -1;
    }

	ptPixelDatas->iWidth  = tDInfo.output_width;
	ptPixelDatas->iHeight = tDInfo.output_height;
	//ptPixelDatas->iBpp    = iBpp;
	ptPixelDatas->iLineBytes    = ptPixelDatas->iWidth * ptPixelDatas->iBpp / 8;
    ptPixelDatas->iTotalBytes   = ptPixelDatas->iHeight * ptPixelDatas->iLineBytes;
	ptPixelDatas->aucPixelDatas = malloc(ptPixelDatas->iTotalBytes);
	if (NULL == ptPixelDatas->aucPixelDatas)
	{
		return -1;
	}

    pucDest = ptPixelDatas->aucPixelDatas;

	// 循环调用jpeg_read_scanlines来一行一行地获得解压的数据
	while (tDInfo.output_scanline < tDInfo.output_height) 
	{
        /* 得到一行数据,里面的颜色格式为0xRR, 0xGG, 0xBB */
		(void) jpeg_read_scanlines(&tDInfo, &aucLineBuffer, 1);

		// 转到ptPixelDatas去
		CovertOneLine(ptPixelDatas->iWidth, 24, ptPixelDatas->iBpp, aucLineBuffer, pucDest);
		pucDest += ptPixelDatas->iLineBytes;
	}
	
	free(aucLineBuffer);
	jpeg_finish_decompress(&tDInfo);
	jpeg_destroy_decompress(&tDInfo);

    return 0;
}

static int FreePixelDatasForJPG(PPixelDatas ptPixelDatas)
{
	free(ptPixelDatas->aucPixelDatas);
	return 0;
}




static PicFileParser g_tJPGParser = {
	.name           = "jpg",
	.isSupport      = isJPGFormat,
	.GetPixelDatas  = GetPixelDatasFrmJPG,
	.FreePixelDatas = FreePixelDatasForJPG,	
};


void JPGParserInit(void)
{
	RegisterPicFileParser(&g_tJPGParser);
}


