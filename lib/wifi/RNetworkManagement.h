#ifndef __RNETWORK_MANAGEMENT_H_
#define __RNETWORK_MANAGEMENT_H_

#include <stdio.h>
#include <unistd.h>

class RNetdConnector;

class RNetworkManagement
{
public:
    static RNetworkManagement* getInstance();
    static void releaseInstance();

    void startTethering();
    void stopTethering();

    void setDnsForwarders(const char* const * dns, int num);

    //default wlan0 STA
    void wifiFirmwareReload();
    void wifiFirmwareReload(const char* ifname, const char* mode);

    void setInterfaceUp(const char* ifname = NULL);
    void setInterfaceDown(const char* ifname = NULL);

    void clearInterfaceAddresses(const char* ifname = NULL);
    //TODO: other opt
private:
    RNetworkManagement();
    ~RNetworkManagement();

    void getInterfaceConfig(const char* ifname);
    void setInterfaceConfig(const char* ifname, const char* flag);

    int getCmdSN();

    int mSNBase;

    RNetdConnector* mConnector;
    static RNetworkManagement* mManagement;
};

#endif
