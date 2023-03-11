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
#include <page_manager.h>
#include <picfmt_manager.h>



extern PicFileParser g_tBMPParser;
char Font;


int main(int argc,char **argv)
{
	int iError;
	Font = argv[1];
	
	/* 初始化显示设备 */
	DisplaySystemRegister();
	SelectDefaultDisplay("fb");
	InitDefaultDisplay();

	CleanScreen(0);

	/* 初始化字体模块 */
	FontSystemRegister();
	//SelectAndInitFont("ascii",NULL);

	SelectAndInitFont("freetype",argv[1]);

	/* 初始化字体操作 */
	iError = EncodingInit();
	if (iError)
	{
		printf("EncodingInit error!\n");
		return -1;
	}

	/* 初始化图片解析器 */
	PicFmtsInit();

	/* 申请空间 */
	AllocVideoMem(6);

	/* 初始化输入设备 */
	InputSystemRegister();
	IntpuDeviceInit();

	/* 页面初始化 */
	PagesInit();

	GetPages("main")->Run(NULL);

	return 0;
}


