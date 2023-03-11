#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <draw.h>
#include <encoding_manager.h>
#include <font_manager.h>
#include <disp_manager.h>
#include <string.h>
#include <input_manager.h>
#include <debug_manager.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <render.h>


extern PicFileParser g_tBMPParser;


int main(int argc,char **argv)
{
	PDispBuff ptDispBuff;
	unsigned char *fBMPmem;
	int iFdBMP;
	struct stat tStat;
	int err;

	PixelDatas tPixelDatas;
	PixelDatas tPixelDatasSmall;
	PixelDatas tPixelDatasFb;

	if(argc!=2)
	{
		printf("Usage :%s <bmp_file>\n",argv[1]);
		return -1;
	}

	/* 初始化显示设备 */
	DisplaySystemRegister();
	SelectDefaultDisplay("fb");
	InitDefaultDisplay();

	ptDispBuff = GetDisplayBuffer();

	CleanScreen(0);

	/* 打开BMP文件 */
	iFdBMP = open(argv[1],O_RDWR);
	if(iFdBMP < 0)
	{
		printf("can not open %s\n",argv[1]);
		return -1;
	}

	if(fstat(iFdBMP, &tStat))
	{
		printf("can't get fstat\n");
		return -1;
	}	

	fBMPmem = (unsigned char *)mmap(NULL , tStat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, iFdBMP, 0);
	if (fBMPmem == (unsigned char *)-1)
	{
		printf("can't mmap for text file\n");
		return -1;
	}

	/* 提取BMP文件的RGB数据, 缩放, 在LCD上显示出来 */
	err = g_tBMPParser.isSupport(fBMPmem);
	if(err == 0)
	{
		printf("%s is not bmp file\n", argv[1]);
		return -1;
	}

	tPixelDatas.iBpp = ptDispBuff->iBpp;
	err = g_tBMPParser.GetPixelDatas(fBMPmem,&tPixelDatas);
	if(err!=0)
	{
		printf("GetPixelDatas err\n");
		return -1;
	}

	tPixelDatasFb.iWidth = ptDispBuff->iXres;
	tPixelDatasFb.iHeight = ptDispBuff->iYres;
	tPixelDatasFb.iBpp = ptDispBuff->iBpp;
	tPixelDatasFb.iLineBytes = ptDispBuff->iXres * ptDispBuff->iBpp/8;
	tPixelDatasFb.aucPixelDatas = ptDispBuff->buff;
	

	PicMerge(0,0,&tPixelDatas,&tPixelDatasFb);

	tPixelDatasSmall.iWidth  = tPixelDatas.iWidth/2;
	tPixelDatasSmall.iHeight = tPixelDatas.iHeight/2;
	tPixelDatasSmall.iBpp    = tPixelDatas.iBpp;
	tPixelDatasSmall.iLineBytes = tPixelDatasSmall.iWidth * tPixelDatasSmall.iBpp / 8;
	tPixelDatasSmall.aucPixelDatas = malloc(tPixelDatasSmall.iLineBytes * tPixelDatasSmall.iHeight);	

	PicZoom(&tPixelDatas,&tPixelDatasSmall);
	PicMerge(128,128,&tPixelDatasSmall,&tPixelDatasFb);

	g_tBMPParser.FreePixelDatas(&tPixelDatas);

	return 0;
}



