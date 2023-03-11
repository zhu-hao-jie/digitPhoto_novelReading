#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#include <disp_manager.h>
#include <font_manager.h>


int main(int argc, char **argv)
{
	PDispBuff ptBuffer;
	int error;

	FontBitMap tFontBitMap;
	char *str= "中";
	int i = 0;
	int lcd_x;
	int lcd_y;
	int font_size;
	unsigned int dwcode;
		

	if (argc != 5)
	{
		printf("Usage: %s <font_file> <lcd_x> <lcd_y> <font_size>\n", argv[0]);
		return -1;
	}

	lcd_x = strtol(argv[2], NULL, 0);
	lcd_y = strtol(argv[3], NULL, 0);
	
	font_size  = strtol(argv[4], NULL, 0);
	
		
	DisplaySystemRegister();

	SelectDefaultDisplay("fb");
	

	InitDefaultDisplay();

	ptBuffer = GetDisplayBuffer();

	FontSystemRegister();
	
	error = SelectAndInitFont("gbk", argv[1]);
	
	if (error)
	{
		printf("SelectAndInitFont err\n");
		return -1;
	}
	
	SetFontSize(font_size);
	CleanScreen(0xffffff);


	while (str[i])
	{
		/* get bitmap */
		tFontBitMap.iCurOriginX = lcd_x;
		tFontBitMap.iCurOriginY = lcd_y;
		dwcode = (str[i]) | (str[i+1]<<8);
		printf("this code : 0x%x 0x%x\n", str[i],str[i+1]);
		printf("this dwcode : 0x%x\n", dwcode);
		i++;
		error = GetFontBitMap(dwcode, &tFontBitMap);
		if (error)
		{
			printf("SelectAndInitFont err\n");
			return -1;
		}

		/* draw on buffer */		
		DrawFontBitMap(&tFontBitMap, 0xff0000);

		/* flush to lcd/web */		
		//FlushDisplayRegion(&tFontBitMap.tRegion, ptBuffer);
		

		lcd_x = tFontBitMap.iNextOriginX;
		lcd_y = tFontBitMap.iNextOriginY;	
		i++;
	}
	
	return 0;	
}


