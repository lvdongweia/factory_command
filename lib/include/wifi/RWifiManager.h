#ifndef __RWIFI_MANAGER_H_
#define __RWIFI_MANAGER_H_

#include <utils/Mutex.h>

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

class RWifiHalCmd;
class RNetworkUtils;
class RWifiInfo;
class RWifiSsidPasswd;
class RNetworkManagement;

class RWifiManager
{
public:
    enum SecurityType
    {
        PASSWD_NONE = 0,
        PASSWD_WPA_PSK = 1,
        PASSWD_WEB_KEY = 2,
    };

public:
    static RWifiManager* getInstance();
    static void releaseInstance();

    int openWifi();
    void closeWifi();

    bool isWifiEnabled();
    bool isConnected();

    int connectToWifi(const char* ssid, const char* pwd = 0, int security = PASSWD_NONE);

    //get current connected info
    const char* getSSID();
    const char* getIpAddr();

    //-200~256
    int getWifiSignal();

    //return number of packets received or -1
    int ping(const char* ip_host, int packet_num);

private:
    RWifiManager();
    ~RWifiManager();

    bool isP2pSupported();
    void startMonitoring();
    void stopMonitoring();

    bool setWifiSsid(int netId, const char* ssid);
    bool setWifiPasswd(int netId, const char* pwd, int security);

    void handleFailedIp();
    void handleNetworkDisconnect();
    void handleNetworkConnect();

    bool dispatchEvent(const char* event_str);
    static void *processEvent(void* arg);

    bool mIsMonitoring;
    bool mIsWifiEnabled;
    bool mIsConnected;

    pthread_t mEventHandle;
    RWifiInfo* mWifiInfo;
    RWifiHalCmd* mWifiCmd;
    RNetworkUtils* mNetUtils;
    RWifiSsidPasswd* mSsidTool;
    RNetworkManagement* mNetd;

    android::Mutex mLock;
    static RWifiManager* mManager;
};

#endif
