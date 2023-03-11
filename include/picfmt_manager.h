#ifndef _PIC_MANAGER_H
#define _PIC_MANAGER_H

#include <Pic_operation.h>
#include <page_manager.h>
#include <file.h>

void RegisterPicFileParser(PPicFileParser ptPicFileParser);

void ShowPicFmts(void);
void PicFmtsInit(void);
PPicFileParser Parser(char *pcName);
PPicFileParser GetParser(PT_FileMap ptFileMap);



#endif


