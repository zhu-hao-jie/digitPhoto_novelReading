#include <picfmt_manager.h>
#include <string.h>
#include <stdio.h>

static PPicFileParser g_tFileParser = NULL;


void RegisterPicFileParser(PPicFileParser ptPicFileParser)
{
	ptPicFileParser->PtNext = g_tFileParser;
	g_tFileParser = ptPicFileParser;
}


/* 显示图片解析器 */
void ShowPicFmts(void)
{
	int i = 0;
	PPicFileParser Ptmp = g_tFileParser;
	while(Ptmp)
	{
		printf("%02d %s\n", i++, Ptmp->name);
		Ptmp = Ptmp->PtNext;
	}
}


/* 初始化图片解析器 */
void PicFmtsInit(void)
{
	extern void BMPParserInit(void);
	BMPParserInit();
	extern void JPGParserInit(void);
	JPGParserInit();
}

/* 通过名字找到图片解析器 */
PPicFileParser Parser(char *pcName)
{
	PPicFileParser Ptmp = g_tFileParser;
	while(Ptmp)
	{
		if(strcmp(Ptmp->name,pcName)==0)
		{
			return Ptmp;
		}
		Ptmp = Ptmp->PtNext;
	}
	return NULL;
}

/* 得到 */
PPicFileParser GetParser(PT_FileMap ptFileMap)
{
	PPicFileParser ptTmp = g_tFileParser;
	
	while (ptTmp)
	{
		if (ptTmp->isSupport(ptFileMap))
		{
			return ptTmp;
		}
		ptTmp = ptTmp->PtNext;
	}
	return NULL;
}






