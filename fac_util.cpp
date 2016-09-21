/*************************************************************************
	> File Name: fac_util.cpp
	> Author: 
	> Mail:  
	> Created Time: 2016年07月11日 星期一 17时13分03秒
 ************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include "fac_util.h"
#include "fac_errors.h"

volatile uint8_t errs = E_OK;



void printHex(const uint8_t *data, int len)
{
#ifdef LOG_TAG
    if (len <= 0 || len > BUF_SIZE) return;

    char str[3*BUF_SIZE + 1] = {0};

    for (int i = 0; i < len; i++)
    {
        sprintf(str+3*i, "%02x ", data[i]);
    }

    str[3*len-1] = '\0';
    LOGD("## Data:[%s] ##", str);
#endif
}

void printMacAddr(const uint8_t *data, int len)
{
#ifdef LOG_TAG
    char c[128] = {0};

    for (int i = 0; i < len; i++)
    {
        sprintf(c+3*i, "%02x:", data[i]);
    }
    c[3*len-1] = '\0';
    LOGD("MAC Addr: %s", c);
#endif
}


int str2Hex(uint8_t *dst, const char *src)
{
    int len = strlen(src);

    for (int i = 0; i < len; i++)
    {
        dst[i] = (uint8_t)src[i];
    }

    return len;
}

void Hex2Str(char *dst, const uint8_t *src, int len)
{
    for (int i = 0; i < len; i++)
    {
        dst[i] = (char)src[i];
    }

    dst[len] = '\0';
}

int isFileExist(const char *file)
{
    if (!access(file, F_OK))
        return 1;
    else
        return 0;
}

int createOutputDir()
{
    DIR *dir;
    dir = opendir(OUTPUT_PATH);
    if (dir == NULL)
    {
        LOGD("Create dir-2");
        if (mkdir(OUTPUT_PATH, 0777) < 0)
        {
            LOGE("Could not creat directory %s", OUTPUT_PATH);
            return -1;
        }
        chmod(OUTPUT_PATH, 0777);
    }

    return 0;
}
