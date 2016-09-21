/*************************************************************************
	> File Name: fac_util.h
	> Author: 
	> Mail:  
	> Created Time: 2016年07月11日 星期一 17时11分22秒
 ************************************************************************/

#ifndef FAC_UTIL_H
#define FAC_UTIL_H

#include <stdlib.h>
#include <android/log.h>

#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))

#define FACTORY_TEST_OUTPUT_PATH "/data/factory_test"
#define OUTPUT_PATH FACTORY_TEST_OUTPUT_PATH

#define PAYLOAD_SIZE 250
#define BUF_SIZE     (PAYLOAD_SIZE+5)



#undef LOG_TAG
#define LOG_TAG "FACTORY_COMMAND"

#define FAC_LOGD
#define FAC_LOGE

#ifdef FAC_LOGD
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#else
#define LOGD(...) 
#endif

#ifdef FAC_LOGE
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else
#define LOGE(...) 
#endif




int str2Hex(uint8_t *dst, const char *src);
void Hex2Str(char *dst, const uint8_t *src, int len);

void printHex(const uint8_t *data, int len);
void printMacAddr(const uint8_t *data, int len);

int isFileExist(const char *file);

int createOutputDir();


#endif /* FAC_UTIL_H */
