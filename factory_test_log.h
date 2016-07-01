/*************************************************************************
	> File Name: factory_test_log.h
	> Author: 
	> Mail:  
	> Created Time: 2016年06月29日 星期三 17时21分26秒
 ************************************************************************/

#ifndef FACTORY_TEST_LOG_H
#define FACTORY_TEST_LOG_H

#include <android/log.h>

#define LOG_TAG "FACTORY_COMMAND"

#define FT_LOGD
#define FT_LOGE



#ifdef FT_LOGD
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#else
#define LOGD(...) 
#endif

#ifdef FT_LOGE
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else
#define LOGE(...) 
#endif










#endif /* FACTORY_TEST_LOG_H */
