#ifndef __RWIFI_INFO_H_
#define __RWIFI_INFO_H_

#include <utils/KeyedVector.h>
#include <utils/String8.h>

#include <stdio.h>
#include <unistd.h>

class RWifiHalCmd;

class RWifiInfo
{
public:
    RWifiInfo(RWifiHalCmd* handle);
    ~RWifiInfo();

    const char* getCurrentSSID();
    int getCurrentSignal();

    void loadConfiguredNetworks();

    //if new, return -1, or return netId
    int isNewWifi(const char* ssid);

public:
    static const int MIN_RSSI = -200;
    static const int MAX_RSSI = 256;

public:
    class RWifiConfiguredInfo
    {
    public:
        enum RWifiStatus
        {
            CONFIG_CURRENT = 0,
            CONFIG_DISABLED = 1,
            CONFIG_ENABLED = 2,
        };

        RWifiConfiguredInfo();
        RWifiConfiguredInfo(const RWifiConfiguredInfo& other);

        RWifiConfiguredInfo& operator=(const RWifiConfiguredInfo& rightv);

    public:
        int mNetId;
        int mStatus;
        android::String8 mSSID;
    };

    class RWifiScanResultSInfo
    {
        //TODO:
    };

private:
    void readNetworkVariables(RWifiConfiguredInfo& config);
    void fetchRssiAndLinkSpeed(int& rssi, int& linkSpeed);

    //<netId, config>
    android::KeyedVector<int, RWifiConfiguredInfo> mConfiguredNetworks;

    RWifiHalCmd* mWifiCmd;
};

#endif
