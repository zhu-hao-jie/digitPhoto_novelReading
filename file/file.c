#include <file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>



int MapFile(PT_FileMap ptFileMap)
{
	int iFd;
    FILE *tFp;
	struct stat tStat;
	
	/* 打开文件 */
	tFp = fopen(ptFileMap->strFileName, "r+");
	if (tFp == NULL)
	{
		printf("can't open %s\n", ptFileMap->strFileName);
		return -1;
	}
	ptFileMap->tFp = tFp;
    iFd = fileno(tFp);

	fstat(iFd, &tStat);
	ptFileMap->iFileSize = tStat.st_size;
	ptFileMap->pucFileMapMem = (unsigned char *)mmap(NULL , tStat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, iFd, 0);
	if (ptFileMap->pucFileMapMem == (unsigned char *)-1)
	{
		printf("mmap error!\n");
		return -1;
	}
	return 0;

}

void UnMapFile(PT_FileMap ptFileMap)
{
	munmap(ptFileMap->pucFileMapMem, ptFileMap->iFileSize);
	fclose(ptFileMap->tFp);
}


static int isDir(char *strFilePath, char *strFileName)
{
    char strTmp[256];
    struct stat tStat;

    snprintf(strTmp, 256, "%s/%s", strFilePath, strFileName);
    strTmp[255] = '\0';

    if ((stat(strTmp, &tStat) == 0) && S_ISDIR(tStat.st_mode))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static int isRegFile(char *strFilePath, char *strFileName)
{
    char strTmp[256];
    struct stat tStat;

    snprintf(strTmp, 256, "%s/%s", strFilePath, strFileName);
    strTmp[255] = '\0';

    if ((stat(strTmp, &tStat) == 0) && S_ISREG(tStat.st_mode))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}



/* 把某目录下所含的顶层子目录、顶层目录下的文件都记录下来 */
int GetDirContents(char *strDirName, PT_DirContent **pptDirContents, int *piNumber)	
{
    PT_DirContent *aptDirContents;
	struct dirent **aptNameList;
	int iNumber;
	int i;
	int j;

	/* 扫描目录,结果按名字排序,存在aptNameList[0],aptNameList[1],... */
	iNumber = scandir(strDirName, &aptNameList, 0, alphasort);
	if (iNumber < 0)
	{
		printf("scandir error!\n");
		return -1;
	}

	/* 忽略".", ".."这两个目录 */
	aptDirContents = malloc(sizeof(PT_DirContent) * (iNumber - 2));
	if (NULL == aptDirContents)
	{
		printf("malloc error!\n");
		return -1;
	}
    *pptDirContents = aptDirContents;

	for (i = 0; i < iNumber - 2; i++)
	{
		aptDirContents[i] = malloc(sizeof(T_DirContent));
		if (NULL == aptDirContents)
		{
			printf("malloc error!\n");
			return -1;
		}
	}

	/* 先把目录挑出来存入aptDirContents */
	for (i = 0, j = 0; i < iNumber; i++)
	{
		/* 忽略".",".."这两个目录 */
		if ((0 == strcmp(aptNameList[i]->d_name, ".")) || (0 == strcmp(aptNameList[i]->d_name, "..")))
			continue;
        /* 并不是所有的文件系统都支持d_type, 所以不能直接判断d_type */
		/* if (aptNameList[i]->d_type == DT_DIR) */
        if (isDir(strDirName, aptNameList[i]->d_name))
		{
			strncpy(aptDirContents[j]->strName, aptNameList[i]->d_name, 256);
			aptDirContents[j]->strName[255] = '\0';
			aptDirContents[j]->eFileType    = FILETYPE_DIR;
            free(aptNameList[i]);
            aptNameList[i] = NULL;
			j++;
		}
	}

	/* 再把常规文件挑出来存入aptDirContents */
	for (i = 0; i < iNumber; i++)
	{
        if (aptNameList[i] == NULL)
            continue;
        
		/* 忽略".",".."这两个目录 */
		if ((0 == strcmp(aptNameList[i]->d_name, ".")) || (0 == strcmp(aptNameList[i]->d_name, "..")))
			continue;
        /* 并不是所有的文件系统都支持d_type, 所以不能直接判断d_type */
		/* if (aptNameList[i]->d_type == DT_REG) */
        if (isRegFile(strDirName, aptNameList[i]->d_name))
		{
			strncpy(aptDirContents[j]->strName, aptNameList[i]->d_name, 256);
			aptDirContents[j]->strName[255] = '\0';
			aptDirContents[j]->eFileType    = FILETYPE_FILE;
            free(aptNameList[i]);
            aptNameList[i] = NULL;
			j++;
		}
	}

	/* 释放aptDirContents中未使用的项 */
	for (i = j; i < iNumber - 2; i++)
	{
		free(aptDirContents[i]);
	}

	/* 释放scandir函数分配的内存 */
	for (i = 0; i < iNumber; i++)
	{
        if (aptNameList[i])
        {
    		free(aptNameList[i]);
        }
	}
	free(aptNameList);

	*piNumber = j;
	
	return 0;
}

void FreeDirContents(PT_DirContent *aptDirContents, int iNumber)
{
	int i;
	for (i = 0; i < iNumber; i++)
	{
		free(aptDirContents[i]);
	}
	free(aptDirContents);
}

static int isRegDir(char *strDirPath, char *strSubDirName)
{
    static const char *astrSpecailDirs[] = {"bin","boot","dev","etc","home","lib","lib32","libi2c.so","linuxrc","lost+found","media","mnt","opt","proc","root","run","sbin","selinux","sys","tmp","usr","var",NULL};
    int i = 0;
    
    /* 如果目录名含有"astrSpecailDirs"中的任意一个, 则返回0 */
    if (0 == strcmp(strDirPath, "/"))
    {
        while (astrSpecailDirs[i])
        {
            if (0 == strcmp(strSubDirName, astrSpecailDirs[i]))
                return 0;
            i++;
        }
    }
    return 1;    
}


int GetFilesIndir(char *strDirName, int *piStartNumberToRecord, int *piCurFileNumber, int *piFileCountHaveGet, int iFileCountTotal, char apstrFileNames[][256])
{
    int iError;
    PT_DirContent *aptDirContents;  /* 数组:存有目录下"顶层子目录","文件"的名字 */
    int iDirContentsNumber;         /* aptDirContents数组有多少项 */
    int i;
    char strSubDirName[257];

    /* 为避免访问的目录互相嵌套, 设置能访问的目录深度为10 */
#define MAX_DIR_DEEPNESS 10    
    static int iDirDeepness = 0;

    if (iDirDeepness > MAX_DIR_DEEPNESS)
    {
        return -1;
    }

    iDirDeepness++;    
    
    iError = GetDirContents(strDirName, &aptDirContents, &iDirContentsNumber);    
    if (iError)
    {
        printf("GetDirContents error!\n");
        iDirDeepness--;
        return -1;
    }

    /* 先记录文件 */
    for (i = 0; i < iDirContentsNumber; i++)
    {
        if (aptDirContents[i]->eFileType == FILETYPE_FILE)
        {
            if (*piCurFileNumber >= *piStartNumberToRecord)
            {
                snprintf(apstrFileNames[*piFileCountHaveGet], 256, "%s/%s", strDirName, aptDirContents[i]->strName);
                (*piFileCountHaveGet)++;
                (*piCurFileNumber)++;
                (*piStartNumberToRecord)++;
                if (*piFileCountHaveGet >= iFileCountTotal)
                {
                    FreeDirContents(aptDirContents, iDirContentsNumber);
                    iDirDeepness--;
                    return 0;
                }
            }
            else
            {
                (*piCurFileNumber)++;
            }
        }
    }

    /* 递归处理目录 */
    for (i = 0; i < iDirContentsNumber; i++)
    {
        if ((aptDirContents[i]->eFileType == FILETYPE_DIR) && isRegDir(strDirName, aptDirContents[i]->strName))
        {
            snprintf(strSubDirName, 256, "%s/%s", strDirName, aptDirContents[i]->strName);
            GetFilesIndir(strSubDirName, piStartNumberToRecord, piCurFileNumber, piFileCountHaveGet, iFileCountTotal, apstrFileNames);
            if (*piFileCountHaveGet >= iFileCountTotal)
            {
                FreeDirContents(aptDirContents, iDirContentsNumber);
                iDirDeepness--;
                return 0;
            }
        }
    }

    FreeDirContents(aptDirContents, iDirContentsNumber);
    iDirDeepness--;
    return 0;
}


