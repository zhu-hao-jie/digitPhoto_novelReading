#include <Pic_operation.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int PicMerge(int iX, int iY, PPixelDatas ptSmallPic, PPixelDatas ptBigPic)
{
	int i;
	unsigned char *pucSrc;
	unsigned char *pucDest;

	if((ptSmallPic->iWidth > ptBigPic->iWidth) || (ptSmallPic->iHeight > ptBigPic->iHeight) || (ptSmallPic->iBpp != ptBigPic->iBpp))
	{
		printf("this is err\n");
		return -1;
	}

	pucSrc = ptSmallPic->aucPixelDatas;
	pucDest = ptBigPic->aucPixelDatas + iY*ptBigPic->iLineBytes + iX*ptBigPic->iBpp/8;

	for(i=0;i<ptSmallPic->iHeight;i++)
	{
		memcpy(pucDest,pucSrc,ptSmallPic->iLineBytes);
		pucSrc+=ptSmallPic->iLineBytes;
		pucDest+=ptBigPic->iLineBytes;
	}

	return 0;
}


int PicMergeRegion(int iStartXofNewPic, int iStartYofNewPic, int iStartXofOldPic, int iStartYofOldPic, int iWidth, int iHeight, PPixelDatas ptNewPic, PPixelDatas ptOldPic)
{
	int i;
	unsigned char *pucSrc;
	unsigned char *pucDst;
    int iLineBytesCpy = iWidth * ptNewPic->iBpp / 8;

    if ((iStartXofNewPic < 0 || iStartXofNewPic >= ptNewPic->iWidth) || \
        (iStartYofNewPic < 0 || iStartYofNewPic >= ptNewPic->iHeight) || \
        (iStartXofOldPic < 0 || iStartXofOldPic >= ptOldPic->iWidth) || \
        (iStartYofOldPic < 0 || iStartYofOldPic >= ptOldPic->iHeight))
    {
        return -1;
    }
	
	pucSrc = ptNewPic->aucPixelDatas + iStartYofNewPic * ptNewPic->iLineBytes + iStartXofNewPic * ptNewPic->iBpp / 8;
	pucDst = ptOldPic->aucPixelDatas + iStartYofOldPic * ptOldPic->iLineBytes + iStartXofOldPic * ptOldPic->iBpp / 8;
	for (i = 0; i < iHeight; i++)
	{
		memcpy(pucDst, pucSrc, iLineBytesCpy);
		pucSrc += ptNewPic->iLineBytes;
		pucDst += ptOldPic->iLineBytes;
	}
	return 0;
}



