#define LOG_TAG "RobotHwInfo"

#include "RobotHwInfo.h"
#include "factory_log.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <sninfo.h>

RobotHwInfo::RobotHwInfo()
{
    memset(mSN, 0, sizeof(mSN));
}

RobotHwInfo::~RobotHwInfo()
{
}

int RobotHwInfo::openSNFlash()
{
    int fd = open("/dev/sninfo", O_RDONLY);
    if (fd < 0)
    {
        LOGE("open SN Sector failed: %s", strerror(errno));
        return -1;
    }

    return fd;
}

void RobotHwInfo::closeSNFlash(int& fd)
{
    if (fd > 0)
        close(fd);

    fd = -1;
}

const char* RobotHwInfo::getSN()
{
    const char* sn_str = NULL;

    int fd = openSNFlash();
    if (fd > 0)
    {
        struct sn_info tmp_info;
        memset(&tmp_info, 0, sizeof(struct sn_info));
        int ret = ioctl(fd, RNAND_GET_SN_INFO, &tmp_info);
        if (ret == 0)
        {
            if (tmp_info.size > 30)
                tmp_info.size = 30;

            memset(mSN, 0, sizeof(mSN));
            memcpy(mSN, tmp_info.sn, tmp_info.size);

            sn_str = mSN;
        }
        else
            LOGE("get sn info failed: %s", strerror(errno));
    }

    closeSNFlash(fd);
    return sn_str;
}

int RobotHwInfo::getWifiMacAddr(MacAddrStore& addr)
{
    int ret = -1;

    int fd = openSNFlash();
    if (fd > 0)
    {
        struct wifi_mac_addr tmp;
        memset(&tmp, 0, sizeof(struct wifi_mac_addr));
        ret = ioctl(fd, RNAND_GET_WIFI_ADDR, &tmp);
        if (ret == 0)
        {
            if (tmp.addr.size == 6)
            {
                memcpy(addr.addr, tmp.addr.addr, tmp.addr.size);
                ret = 0;
            }
            else
            {
                LOGW("wifi mac addr length error");
                ret = -1;
            }
        }
        else
            LOGE("get wifi mac addr failed: %s", strerror(errno));
    }

    closeSNFlash(fd);
    return ret;
}

int RobotHwInfo::getEthMacAddr(MacAddrStore& addr)
{
    int ret = -1;

    int fd = openSNFlash();
    if (fd > 0)
    {
        struct eth_mac_addr tmp;
        memset(&tmp, 0, sizeof(struct eth_mac_addr));
        ret = ioctl(fd, RNAND_GET_ETH_ADDR, &tmp);
        if (ret == 0)
        {
            if (tmp.addr.size == 6)
            {
                memcpy(addr.addr, tmp.addr.addr, tmp.addr.size);
                ret = 0;
            }
            else
            {
                LOGW("ethernet mac addr length error");
                ret = -1;
            }
        }
        else
            LOGE("get ehternet mac addr failed: %s", strerror(errno));
    }

    closeSNFlash(fd);
    return ret;
}

int RobotHwInfo::getBtMacAddr(MacAddrStore& addr)
{
    int ret = -1;

    int fd = openSNFlash();
    if (fd > 0)
    {
        struct bt_mac_addr tmp;
        memset(&tmp, 0, sizeof(struct bt_mac_addr));
        ret = ioctl(fd, RNAND_GET_BT_ADDR, &tmp);
        if (ret == 0)
        {
            if (tmp.addr.size == 6)
            {
                memcpy(addr.addr, tmp.addr.addr, tmp.addr.size);
                ret = 0;
            }
            else
            {
                LOGW("bluetooth mac addr length error");
                ret = -1;
            }
        }
        else
            LOGE("get bluetooth mac addr failed: %s", strerror(errno));
    }

    closeSNFlash(fd);
    return ret;
}
