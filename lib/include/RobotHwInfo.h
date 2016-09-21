#ifndef __ROBOT_HW_INFO_H_
#define __ROBOT_HW_INFO_H_

#include <stdio.h>
#include <unistd.h>

struct MacAddrStore
{
    uint8_t addr[6];
};

class RobotHwInfo
{
public:
    RobotHwInfo();
    ~RobotHwInfo();

    const char* getSN();
    int getWifiMacAddr(MacAddrStore& addr);
    int getEthMacAddr(MacAddrStore& addr);
    int getBtMacAddr(MacAddrStore& addr);

private:
    int openSNFlash();
    void closeSNFlash(int& fd);

    char mSN[32];
};

#endif
