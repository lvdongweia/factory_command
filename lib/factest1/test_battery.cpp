/*************************************************************************
	> File Name: test_battery.cpp
	> Author: 
	> Mail:  
	> Created Time: 2016年08月09日 星期二 17时10分20秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#include "fac_errors.h"
#include "fac_util.h"

#include "test_battery.h"


#define POWER_SUPPLY_SUBSYSTEM "power_supply"
#define POWER_SUPPLY_SYSFS_PATH "/sys/class/" POWER_SUPPLY_SUBSYSTEM

#define TMP_FAILURE_RETRY(exp) ({               \
        typeof (exp) _rc;                       \
        do {                                    \
        _rc = (exp);                            \
        } while(_rc == -1 && errno == EINTR);   \
        _rc;})

enum PowerSupplyType {
    ANDROID_POWER_SUPPLY_TYPE_UNKNOWN = 0,
    ANDROID_POWER_SUPPLY_TYPE_AC,
    ANDROID_POWER_SUPPLY_TYPE_USB,
    ANDROID_POWER_SUPPLY_TYPE_WIRELESS,
    ANDROID_POWER_SUPPLY_TYPE_BATTERY
};

enum {
    BATTERY_STATUS_UNKNOWN      = 1, // equals BatteryManager.BATTERY_STATUS_UNKNOWN constant
    BATTERY_STATUS_CHARGING     = 2, // equals BatteryManager.BATTERY_STATUS_CHARGING constant
    BATTERY_STATUS_DISCHARGING  = 3, // equals BatteryManager.BATTERY_STATUS_DISCHARGING constant
    BATTERY_STATUS_NOT_CHARGING = 4, // equals BatteryManager.BATTERY_STATUS_NOT_CHARGING constant
    BATTERY_STATUS_FULL         = 5, // equals BatteryManager.BATTERY_STATUS_FULL constant
};

struct StringEnumMap {
    const char *s;
    int val;
};

static int mapString(const char *str, struct StringEnumMap map[])
{
    for (int i = 0; map[i].s; i++)
    {
        if (!strcmp(str, map[i].s))
            return map[i].val;
    }

    return -1;
}

static int readFromFile(const char *path, char *buf, int size)
{
    char *cp = NULL;

    if (path == NULL) return -1;

    int fd = open(path, O_RDONLY, 0);
    if (fd < 0) {
        LOGE("Could not open %s", path);
        return -1;
    }

    int count = TMP_FAILURE_RETRY(read(fd, buf, size));
    if (count > 0)
        cp = (char *)memrchr(buf, '\n', count);

    if (cp)
        *cp = '\0';
    else
        buf[0] = '\0';

    close(fd);
    return count;
}


static int getIntField(const char *path)
{
    const int SIZE = 8;
    char buf[SIZE];

    int value = 0;
    if (readFromFile(path, buf, SIZE) > 0)
    {
        value = strtol(buf, NULL, 0);
    }

    return value;
}

static PowerSupplyType readPowerSupplyType(const char *path)
{
    const int SIZE = 32;
    char buf[SIZE];
    PowerSupplyType ret;

    struct StringEnumMap supplyTypeMap[] = {
        {"Unknow", ANDROID_POWER_SUPPLY_TYPE_UNKNOWN},
        {"Battery", ANDROID_POWER_SUPPLY_TYPE_BATTERY},
        { "Mains", ANDROID_POWER_SUPPLY_TYPE_AC },
        { "USB", ANDROID_POWER_SUPPLY_TYPE_USB },
        { "USB_DCP", ANDROID_POWER_SUPPLY_TYPE_AC },
        { "USB_CDP", ANDROID_POWER_SUPPLY_TYPE_AC },
        { "USB_ACA", ANDROID_POWER_SUPPLY_TYPE_AC },
        { "Wireless", ANDROID_POWER_SUPPLY_TYPE_WIRELESS },
        { NULL, 0 },
    };

    int len = readFromFile(path, buf, SIZE);
    if (len <= 0)
        return ANDROID_POWER_SUPPLY_TYPE_UNKNOWN;

    ret = (PowerSupplyType)mapString(buf, supplyTypeMap);
    if (ret < 0)
        ret = ANDROID_POWER_SUPPLY_TYPE_UNKNOWN; 

    return ret;
}

static int getBatteryStatus(const char *buf)
{
    int ret;

    struct StringEnumMap batteryStatusMap[] = {
        { "Unknown", BATTERY_STATUS_UNKNOWN },
        { "Charging", BATTERY_STATUS_CHARGING },
        { "Discharging", BATTERY_STATUS_DISCHARGING },
        { "Not charging", BATTERY_STATUS_NOT_CHARGING },
        { "Full", BATTERY_STATUS_FULL },
        { NULL, 0 },
    };

    return mapString(buf, batteryStatusMap);
}

static int getBatterySupplyPath(char *bat_path)
{
    char path[128] = "\0";

    DIR *dir = opendir(POWER_SUPPLY_SYSFS_PATH);
    if (dir == NULL)
    {
        E_SET(E_FAILED);
        LOGE("Could not open %s", POWER_SUPPLY_SYSFS_PATH);
        return -1;
    }

    struct dirent* entry;
    while((entry = readdir(dir)))
    {
        const char* name = entry->d_name;
        if (!strcmp(name, ".") || !strcmp(name, ".."))
            continue;

        memset(path, 0, sizeof(path));
        sprintf(path, "%s/%s/type", POWER_SUPPLY_SYSFS_PATH, name);

        if (readPowerSupplyType(path) != ANDROID_POWER_SUPPLY_TYPE_BATTERY)
            continue;

        sprintf(bat_path, "%s/%s", POWER_SUPPLY_SYSFS_PATH, name);
        closedir(dir);
        return 0;
    }

    closedir(dir);
    return -1;
}

int get_battery_capacity()
{
    int ret;
    char batteryCapacityPath[128] = {0};
    E_RESET();

    ret = getBatterySupplyPath(batteryCapacityPath);
    if (ret < 0)
    {
        E_SET(E_FAILED);
        return -1;
    }

    strcat(batteryCapacityPath, "/capacity");
    //LOGD("battery capacity: path=%s", batteryCapacityPath);

    ret = getIntField(batteryCapacityPath);
    LOGD("battery capacity: %d", ret);

    return ret;
}

int get_battery_status()
{
    const int SIZE = 128;
    int ret;
    char batteryStatusPath[SIZE] = {0};
    char buf[SIZE] = {0};
    E_RESET();

    ret = getBatterySupplyPath(batteryStatusPath);
    if (ret < 0)
    {
        E_SET(E_FAILED);
        return -1;
    }

    strcat(batteryStatusPath, "/status");
    //LOGD("battery status: path=%s", batteryStatusPath);

    ret = readFromFile(batteryStatusPath, buf, SIZE);
    if (ret < 0) 
    {
        E_SET(E_FAILED);
        return -1;
    }
    LOGD("battery status: %s", buf);

    ret = getBatteryStatus(buf);
    if (ret < 0)
    {
        E_SET(E_FAILED);
        return -1;
    }

    if (ret == BATTERY_STATUS_DISCHARGING)
        return CHARGE_OUT;
    else
        return CHARGE_IN;
}

