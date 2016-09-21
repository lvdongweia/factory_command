/*************************************************************************
	> File Name: fw_version.cpp
	> Author: 
	> Mail:  
	> Created Time: 2016年07月11日 星期一 16时13分23秒
 ************************************************************************/

#include <stdio.h>
#include <cutils/properties.h>

#include "libsysinfo/RobotSysInfo.h"
#include "robot_common_data.h"
#include "rm_message_defs.h"

#include "fac_util.h"
#include "fac_errors.h"
#include "fac_can.h"
#include "ai_ver.h"
#include "test_robot_info.h"

using namespace android;

#define ANDROID_VER_LEN     3
#define AI_CORE_LIB_VER_LEN 8
#define SUBSYS_VER_LEN      9

#define TMOUT_DEFAULT    200 // 200ms

static int get_android_os_ver(uint8_t *ver)
{
    char buf[PROPERTY_VALUE_MAX] = {0};
    int ret;
    int ver_len = 0;

    ret = property_get("ro.build.version.release", buf, "");
    if (ret <= 0)
    {
        E_SET(E_FAILED);
        return -1;
    }

    LOGD("FW id:%02x Ver:%s", FW_VER_ANDROID_OS, buf);
    char *token = strtok(buf, ".");
    while (token != NULL && ver_len < ANDROID_VER_LEN)
    {
        ver[ver_len] = atoi(token);
        token = strtok(NULL, ".");

        ver_len++;
    }

    return ver_len;
}


static int get_ai_core_lib_ver(uint8_t *ver)
{
    char buf[PROPERTY_VALUE_MAX] = {0};
    int ret;
    const int build_data = 4;
    int ver_len = 0;
    int year=0, mouth=0, day=0;

    ret = get_nlu_version(buf, sizeof(buf));
    if (ret < 0)
    {
        E_SET(E_FAILED);
        return -1;
    }

    /* AI ver exmaple: R-2.7.0.0.20160101-Beta */
    char *begin = buf + 2;
    char *end = strrchr(begin, '-');
    if (end) *end = '\0';

    LOGD("FW id:%02x Ver:%s", FW_VER_AI_CORE_LIB, begin);
    char *token = strtok(begin, ".");
    /* get ai lib version:
       major, minor, revision1, revision2, 
       year, mouth ,day
    */
    while (token != NULL && ver_len < AI_CORE_LIB_VER_LEN)
    {
        if (ver_len == build_data)
        {
            if (strlen(token) != 8)
            {
                E_SET(E_OTHERS);
                return -1;
            }
            sscanf(token, "%4d%2d%2d", &year, &mouth, &day);
            ver[ver_len++] = (uint8_t)(year / 100);
            ver[ver_len++] = (uint8_t)(year % 100);
            ver[ver_len++] = (uint8_t)mouth;
            ver[ver_len++] = (uint8_t)day;
        }
        else
        {
            ver[ver_len++] = atoi(token);
        }

        token = strtok(NULL, ".");
    }

    return ver_len;
}

static int get_subsys_ver(int id, uint8_t *ver)
{
    RobotSysInfo sysInfo;
    int ret;
    int ver_len = 0;
    const int build_data = 4;
    char buf[VERSION_VALUE_MAX], tmp[3]; 
    int realId = id - FW_VER_SUBSYS_RM;
    int year=0, mouth=0, day=0;

    ret = sysInfo.GetSubSysVersion(realId, buf, sizeof(buf));
    if (ret < 0)
    {
        E_SET(E_FAILED);
        return -1;
    }

    LOGD("FW id:%02x Ver:%s", id, buf);
    char *token = strtok(buf, ".");
    /* get sub-system id, major version,
     * minor version, revision version,
     * building data, rom id.
     */
    while(token != NULL && ver_len < SUBSYS_VER_LEN)
    {
        if (ver_len == build_data) 
        {
            if (strlen(token) != 8)
            {
                E_SET(E_OTHERS);
                return -1;
            }

            sscanf(token, "%4d%2d%2d", &year, &mouth, &day);
            ver[ver_len++] = (uint8_t)(year / 100);
            ver[ver_len++] = (uint8_t)(year % 100);
            ver[ver_len++] = (uint8_t)mouth;
            ver[ver_len++] = (uint8_t)day;
        }
        else
        {
            ver[ver_len++] = atoi(token);
        }

        token = strtok(NULL, ".");
    }

    return ver_len;
}

int get_serial_number(uint8_t *sn)
{
    RobotSysInfo sysInfo;
    char serialNum[SERIAL_NUMBER_MAX] = {0};
    bool isValid;
    E_RESET();

    sysInfo.GetRobotSN(isValid, serialNum, sizeof(serialNum));
    if (!isValid)
    {
        E_SET(E_FAILED);
        return -1;
    }

    LOGD("Robot SN:%s", serialNum);
    return str2Hex(sn, serialNum);
}

int getfw_version(int id, uint8_t *version)
{
    E_RESET();
    if (id < FW_VER_ANDROID_OS || id > FW_VER_SUBSYS_RBB)
    {
        E_SET(E_INVALID_NUMBER);
        return -1;
    }

    if (version == NULL)
    {
        E_SET(E_OTHERS);
        return -1;
    }

    if (id == FW_VER_ANDROID_OS)
        return get_android_os_ver(version);

    if (id == FW_VER_AI_CORE_LIB)
        return get_ai_core_lib_ver(version);

    return get_subsys_ver(id, version);
}

int set_gender(uint8_t sex)
{
    RobotSysInfo sysInfo;
    bool isBoy;
    E_RESET();

    if (sex != ROBOT_GENDER_BOY && sex != ROBOT_GENDER_GIRL)
    {
        E_SET(E_INVALID_NUMBER);
        return -1;
    }

    isBoy = (sex == ROBOT_GENDER_BOY ? true : false);
    return sysInfo.SetRobotSex(isBoy);
}

int get_gender(uint8_t &sex)
{
    RobotSysInfo sysInfo;
    bool isBoy;
    int ret;
    E_RESET();

    ret = sysInfo.GetRobotSex(isBoy);
    if (ret < 0)
    {
        E_SET(E_FAILED);
        return -1;
    }

    sex = (isBoy ? ROBOT_GENDER_BOY : ROBOT_GENDER_GIRL);
    return 0;
}

int test_can_module(uint8_t m_id)
{
    int ret;
    uint8_t id;
    E_RESET();    
    RobotSysInfo sysInfo;
    RobotSysInfo::RobotSubSysState state;

    if (m_id < CAN_MODULE_RM || m_id > CAN_MODULE_RBB)
    {
        E_SET(E_PARAMETERS);
        return -1;
    }

    /* m_id << 4 get real module id */
    id = m_id & 0x0F;
    LOGD("id = %d", id);
    ret = sysInfo.GetSubSysState(id, state);
    if (ret < 0)
    {
        E_SET(E_FAILED);
        return -1;
    }

    if (state.is_active)
    {
        return 0;
    }

    E_SET(E_FAILED);
    return -1;
}

int set_expression(uint8_t id)
{
    int ret, d_len;
    uint8_t data[4] = {0};
    uint8_t pData[256] = {0};
    E_RESET();

    if (id < RF_EXPRESSION_SMILE_CMD || id > RF_EXPRESSION_EYEBINDONE)
    {
        E_SET(E_PARAMETERS);
        return -1;
    }

    ret = init_can();
    if (ret < 0)
    {
        E_SET(E_CAN_COMMUNICATE);
        return -1;
    }

    data[0] = RF_DISPLAY_CMD;
    data[1] = 0x04;
    data[2] = 0x00;
    data[3] = id;

    ret = send_can_data(RF_DISPLAY_ID, data, sizeof(data));
    if (ret < 0)
    {
        E_SET(E_CAN_COMMUNICATE);
        return -1;
    }

    ret = read_can_data(pData, d_len, RF_DISPLAY_ID, RF_DISPLAY_CMD_REP, 4, TMOUT_DEFAULT);
    if (ret < 0)
    {
        E_SET(E_FAILED);
        uninit_can();
        return -1;
    }

    if (pData[3] != 0x00)
    {
        E_SET(E_FAILED);
        uninit_can();
        return -1;
    }

    uninit_can();
    return 0;
}
