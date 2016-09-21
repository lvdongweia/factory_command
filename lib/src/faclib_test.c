/*************************************************************************
FileName: faclib_test.c
Copy Right: Copyright (C), 2014-2016, Avatarmind. Co., Ltd.
System: RM
Module: FACTORY TEST
Author: wangkun
Create Date: 2016-08-17
**************************************************************************/
/*************************************************************************
Editor: wangkun
Version: v1.0 init version
Edit Date: 2016-08-17
**************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <poll.h>
#include <sys/utsname.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <netdb.h>
#include <time.h>

#define LOG_TAG "FACLIB_TEST"
#include <cutils/log.h>
#include "robot_fac.h"

int main()
{
    int ret = 0;

    ALOGD("FACLIB_TEST begin");
    ret = fac_motor_move(ARM_ROTATION_RIGHT, POSITIVE, 127, 30);
    if (ret < 0)
       ALOGE("Error:%d\n", ret);

    ret = fac_motor_stop(ARM_ROTATION_RIGHT);
    if (ret < 0)
       ALOGE("Error:%d\n", ret);

    ret = fac_motor_calibration(ARM_ROTATION_RIGHT);
    if (ret < 0)
       ALOGE("Error:%d\n", ret);

    ret = fac_motor_calibration2(RB_LEFT);
    if (ret < 0)
       ALOGE("Error:%d\n", ret);

    ret = fac_motor_motion(WAVE);
    if (ret < 0)
       ALOGE("Error:%d\n", ret);

    ret = fac_set_power_mode(PM_SLEEP);
    if (ret < 0)
       ALOGE("Error:%d\n", ret);

    ALOGD("FACLIB_TEST end");
    return 0;
}
