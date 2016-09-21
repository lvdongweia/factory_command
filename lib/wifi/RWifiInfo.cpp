#define LOG_TAG "RWifiInfo"

#include "RWifiInfo.h"
#include "RWifiHalCmd.h"
#include "RWifiSsidPasswd.h"
#include "factory_log.h"

#include <stdlib.h>

RWifiInfo::RWifiConfiguredInfo::RWifiConfiguredInfo()
{
    mNetId = -1;
}

RWifiInfo::RWifiConfiguredInfo::RWifiConfiguredInfo(const RWifiInfo::RWifiConfiguredInfo& other)
{
    mNetId = other.mNetId;
    mStatus = other.mStatus;
    mSSID = other.mSSID;
}

RWifiInfo::RWifiConfiguredInfo&
RWifiInfo::RWifiConfiguredInfo::operator=(const RWifiInfo::RWifiConfiguredInfo& rightv)
{
    mNetId = rightv.mNetId;
    mStatus = rightv.mStatus;
    mSSID = rightv.mSSID;

    return *this;
}

RWifiInfo::RWifiInfo(RWifiHalCmd* handle)
{
    mWifiCmd = handle;
}

RWifiInfo::~RWifiInfo()
{
}

int RWifiInfo::isNewWifi(const char* ssid)
{
    loadConfiguredNetworks();

    for (size_t i = 0; i < mConfiguredNetworks.size(); i++)
    {
        if (strcmp(mConfiguredNetworks[i].mSSID.string(), ssid) == 0)
            return mConfiguredNetworks[i].mNetId;
    }

    return -1;
}

const char* RWifiInfo::getCurrentSSID()
{
    loadConfiguredNetworks();

    for (size_t i = 0; i < mConfiguredNetworks.size(); i++)
    {
        if (mConfiguredNetworks[i].mStatus == RWifiConfiguredInfo::CONFIG_CURRENT
                && !mConfiguredNetworks[i].mSSID.isEmpty())
            return mConfiguredNetworks[i].mSSID.string();
    }

    return NULL;
}

int RWifiInfo::getCurrentSignal()
{
    int rssi, linkSpeed;
    fetchRssiAndLinkSpeed(rssi, linkSpeed);

    return rssi;
}

//TODO: optimize parsing
void RWifiInfo::loadConfiguredNetworks()
{
    mConfiguredNetworks.clear();

    const char* list_networks = mWifiCmd->listNetworks();
    if (!list_networks || !strlen(list_networks) || !strchr(list_networks, '\n'))
    {
        LOGE("listNetwors failed or no network in config");
        return;
    }

    LOGD("list_network: %s", list_networks);
    size_t len = strlen(list_networks) + 1;
    char* networks_str = new char[len];
    if (!networks_str) return;

    memset(networks_str, 0, len);
    memcpy(networks_str, list_networks, len);

    // network-id | ssid | bssid | flags
    // Skip the first line, which is a header
    char* start_ptr = strchr(networks_str, '\n');
    start_ptr++;

    char* flag_ptr;
    while (start_ptr && *start_ptr != '\0')
    {
        flag_ptr = strchr(start_ptr, '\n');
        if (flag_ptr != NULL) *flag_ptr = '\0';

        int field_counts = 0;
        char* fields[4];
        char* tab_ptr = start_ptr;
        while (tab_ptr && *tab_ptr != '\0')
        {
            fields[field_counts++] = tab_ptr;
            if ((tab_ptr = strchr(tab_ptr, '\t')) != NULL)
            {
                *tab_ptr = '\0';
                tab_ptr++;
            }
            else
                break;
        }

        for (int fi = 0; fi < field_counts; fi++)
            LOGD("fiedls[%d] = %s", fi, fields[fi]);

        RWifiConfiguredInfo config;
        config.mNetId = atoi(fields[0]);

        if (field_counts > 3 )
        {
            if (strstr(fields[3], "[CURRENT]"))
                config.mStatus = RWifiConfiguredInfo::CONFIG_CURRENT;
            else if (strstr(fields[3], "[DISABLED]"))
                config.mStatus = RWifiConfiguredInfo::CONFIG_DISABLED;
            else
                config.mStatus = RWifiConfiguredInfo::CONFIG_ENABLED;
        }
        else
            config.mStatus = RWifiConfiguredInfo::CONFIG_ENABLED;

        //readNetworkVariables(config);
        config.mSSID = fields[1];
        mConfiguredNetworks.add(config.mNetId, config);

        if (flag_ptr)
            start_ptr = ++flag_ptr;
        else
            break;
    }

    delete networks_str;
}

void RWifiInfo::readNetworkVariables(RWifiInfo::RWifiConfiguredInfo& config)
{
    int netId = config.mNetId;
    if (netId < 0) return;

    //value = "\"302\"";
    const char* value = mWifiCmd->getNetworkVariable(netId, RWifiHalCmd::mSsidVarName);
    if (value)
        config.mSSID = value;
    else
    {
        config.mSSID = NULL;
        LOGW("read network ssid failed!");
    }

    //TODO: if ssid[0] == '"', ssid string is not UTF-8 decodable
    //convert to "\...\"

    //TODO: get other info
}

//TODO: optimize parsing
void RWifiInfo::fetchRssiAndLinkSpeed(int& rssi, int& linkSpeed)
{
    rssi = MIN_RSSI;
    linkSpeed = -1;

    const char* value = mWifiCmd->signalPoll();
    if (!value) return;

    size_t len = strlen(value) + 1;
    char* signal_str = new char[len];
    if (!signal_str) return;

    memset(signal_str, 0, len);
    memcpy(signal_str, value, len);

    char* start_ptr = signal_str;
    char* flag_ptr;
    while (start_ptr && *start_ptr != '\0')
    {
        flag_ptr = strchr(start_ptr, '\n');
        if (flag_ptr != NULL) *flag_ptr = '\0';

        char* tag = strchr(start_ptr, '=');
        if (tag)
        {
            *tag = '\0';
            if (strcmp(start_ptr, "RSSI") == 0)
                rssi = atoi(tag + 1);
            else if (strcmp(start_ptr, "LINKSPEED") == 0)
                linkSpeed = atoi(tag + 1);
        }

        if (flag_ptr)
            start_ptr = ++flag_ptr;
        else
            break;
    }

    if (MIN_RSSI < rssi && rssi < MAX_RSSI)
    {
        /* some implementations avoid negative values by adding 256
         * so we need to adjust for that here.
         */
        if (rssi > 0) rssi -= 256;
    }

    delete signal_str;
}
