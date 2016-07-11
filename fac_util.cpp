/*************************************************************************
	> File Name: fac_util.cpp
	> Author: 
	> Mail:  
	> Created Time: 2016年07月11日 星期一 17时13分03秒
 ************************************************************************/

#include <stdio.h>
#include <ctype.h>

#include "fac_log.h"
#include "fac_util.h"

void printHex(const uint8_t *data, int len)
{
    if (len <= 0 || len > BUF_SIZE) return;

    char str[3*BUF_SIZE + 1] = {0};

    for (int i = 0; i < len; i++)
    {
        sprintf(str+3*i, "%02x ", data[i]);
    }

    str[3*len-1] = '\0';
    LOGD("#####Data hex:[%s]", str);
}


void str2Hex(uint8_t *dst, char *src, int len)
{
    for (int i = 0; i < len; i++)
    {
        dst[i] = (uint8_t)src[i];
    }
}
