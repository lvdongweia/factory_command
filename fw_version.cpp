/*************************************************************************
	> File Name: fw_version.cpp
	> Author: 
	> Mail:  
	> Created Time: 2016年07月11日 星期一 16时13分23秒
 ************************************************************************/


#include "libsysinfo/RobotSysInfo.h"

#include "fw_version.h"

using namespace android;

static int get_android_os_ver(char *ver, size_t len)
{
    return 0;
}


static int get_ai_core_lib_ver(char *ver, size_t len)
{
    return 0;
}

static int get_subsys_ver(int id, char *ver, size_t len)
{
    RobotSysInfo sysInfo;

    int realId = id - FW_VER_SUBSYS_RM;

    return sysInfo.GetSubSysVersion(realId, ver, len);
}

int getfw_version(int id, char *version, size_t len)
{
    if (id < FW_VER_ANDROID_OS || id > FW_VER_SUBSYS_RBB)
        return -1;

    if (id == FW_VER_ANDROID_OS)
        return get_android_os_ver(version, len);

    if (id == FW_VER_AI_CORE_LIB)
        return get_ai_core_lib_ver(version, len);

    return get_subsys_ver(id, version, len);
}
