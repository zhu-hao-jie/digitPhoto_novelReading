#include <Pic_operation.h>
#include <stdlib.h>
#include <string.h>


int PicZoom(PPixelDatas ptOriginPic, PPixelDatas ptZoomPic)
{
    unsigned long dwDstWidth = ptZoomPic->iWidth;
    unsigned long* pdwSrcXTable = malloc(sizeof(unsigned long) * dwDstWidth);
	unsigned long x;
	unsigned long y;
	unsigned long dwSrcY;
	unsigned char *pucDest;
	unsigned char *pucSrc;
	unsigned long dwPixelBytes = ptOriginPic->iBpp/8;

	if (ptOriginPic->iBpp != ptZoomPic->iBpp)
	{
		return -1;
	}
	
    for (x = 0; x < dwDstWidth; x++)//生成表 pdwSrcXTable
    {
        pdwSrcXTable[x]=(x*ptOriginPic->iWidth/ptZoomPic->iWidth);
    }

    for (y = 0; y < ptZoomPic->iHeight; y++)
    {			
        dwSrcY = (y * ptOriginPic->iHeight / ptZoomPic->iHeight);

		pucDest = ptZoomPic->aucPixelDatas + y*ptZoomPic->iLineBytes;
		pucSrc  = ptOriginPic->aucPixelDatas + dwSrcY*ptOriginPic->iLineBytes;
		
        for (x = 0; x <dwDstWidth; x++)
        {
            /* 原图座标: pdwSrcXTable[x]，srcy
             * 缩放座标: x, y
			 */
			 memcpy(pucDest+x*dwPixelBytes, pucSrc+pdwSrcXTable[x]*dwPixelBytes, dwPixelBytes);
        }
    }

    free(pdwSrcXTable);
	return 0;
}

