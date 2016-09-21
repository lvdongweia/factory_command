#define LOG_TAG "RWifiManager"

#include "RWifiManager.h"
#include "RWifiInfo.h"
#include "RWifiHalCmd.h"
#include "RNetworkUtils.h"
#include "RWifiSsidPasswd.h"
#include "RDhcpResults.h"
#include "RNetworkManagement.h"
#include "factory_log.h"

#include <stdlib.h>

RWifiManager* RWifiManager::mManager = NULL;
RWifiManager* RWifiManager::getInstance()
{
    if (mManager == NULL)
        mManager = new RWifiManager();

    return mManager;
}

void RWifiManager::releaseInstance()
{
    if (mManager)
        delete mManager;

    mManager = NULL;
}

RWifiManager::RWifiManager()
{
    mIsMonitoring = false;
    mIsWifiEnabled = false;
    mIsConnected = false;

    memset(&mEventHandle, 0, sizeof(pthread_t));

    mWifiCmd = new RWifiHalCmd();
    mNetUtils = new RNetworkUtils();
    mWifiInfo = new RWifiInfo(mWifiCmd);
    mSsidTool = new RWifiSsidPasswd();

    mNetd = RNetworkManagement::getInstance();
}

RWifiManager::~RWifiManager()
{
    delete mWifiCmd;
    delete mNetUtils;
    delete mWifiInfo;

    RNetworkManagement::releaseInstance();
}

bool RWifiManager::isP2pSupported()
{
    //device/rockchip/common/features/rk-core.xml
    //<feature name="android.hardware.wifi.direct" />
    //TODO: parse xml file, get feature list
    return true;
}

int RWifiManager::openWifi()
{
    if (mIsWifiEnabled) return 0;

    //mWifiCmd->killSupplicant(isP2pSupported());
    if (mWifiCmd->loadDriver())
    {
        //mNetd->wifiFirmwareReload();
        //mNetd->setInterfaceDown();
        if (mWifiCmd->startSupplicant(isP2pSupported()))
        {
            if (mWifiCmd->connectToSupplicant())
            {
                startMonitoring();
                return 0;
            }
            else
            {
                LOGE("connect to supplicant failed!");
                goto error_out1;
            }
        }
        else
        {
            LOGE("start supplicant failed!");
            goto error_out2;
        }
    }
    else
    {
        LOGE("load wifi driver failed!");
        goto error_out2;
    }

error_out1:
    mWifiCmd->killSupplicant(isP2pSupported());

error_out2:
    mWifiCmd->unloadDriver();
    return -1;
}

void RWifiManager::startMonitoring()
{
    mIsWifiEnabled = true;
    mIsMonitoring = true;

    pthread_create(&mEventHandle, NULL, processEvent, (void*)this);
    mNetd->startTethering();
}

void RWifiManager::closeWifi()
{
    if (!mIsWifiEnabled) return;

    mWifiCmd->closeSupplicantConnection();
    LOGD("closed supplicant connection...");

    stopMonitoring();
    mWifiCmd->killSupplicant(false);
    mNetUtils->disableInterface(RWifiHalCmd::mDefaultInterface);
    mWifiCmd->unloadDriver();
}

void RWifiManager::stopMonitoring()
{
    mIsMonitoring = false;
    pthread_join(mEventHandle, NULL);

    memset(&mEventHandle, 0, sizeof(pthread_t));
    mIsWifiEnabled = false;
    mNetd->stopTethering();
}

bool RWifiManager::isWifiEnabled()
{
    return mIsWifiEnabled;
}

bool RWifiManager::isConnected()
{
    return mIsConnected;
}

void RWifiManager::handleFailedIp()
{
    LOGE("IP configuration failed");

    /* DHCP times out after about 30 seconds, we do a
     * disconnect and an immediate reconnect to try again
     */
    mWifiCmd->disconnect();
    mWifiCmd->reconnect();
}

void RWifiManager::handleNetworkDisconnect()
{
    if (mIsConnected)
    {
        mNetUtils->stopDhcp(RWifiHalCmd::mDefaultInterface);
        mNetd->clearInterfaceAddresses();
        mIsConnected = false;
    }
}

void RWifiManager::handleNetworkConnect()
{
    if (!mNetUtils->stopDhcp(RWifiHalCmd::mDefaultInterface))
        LOGE("stop dncp error: %s", mNetUtils->getDhcpError());

    if (!mNetUtils->runDhcp(RWifiHalCmd::mDefaultInterface))
    {
        LOGE("dncp request error: %s", mNetUtils->getDhcpError());
        handleFailedIp();
    }
    else
    {
        //set dns
        const RDhcpResults* results = mNetUtils->getDhcpResults();
        mNetd->setDnsForwarders(results->mDns, results->getDnsNum());

        mIsConnected = true;
    }
}

//return true if the event was supplicant disconnection
bool RWifiManager::dispatchEvent(const char* event_str)
{
    bool ret = false;

    size_t event_prefix_len = strlen(RWifiHalCmd::EVENT_PREFIX_STR);
    if (strncmp(event_str, RWifiHalCmd::EVENT_PREFIX_STR, event_prefix_len) != 0)
    {
        if (strncmp(event_str, RWifiHalCmd::WPA_EVENT_PREFIX_STR, strlen(RWifiHalCmd::WPA_EVENT_PREFIX_STR)) == 0
                && strstr(event_str, RWifiHalCmd::PASSWORD_MAY_BE_INCORRECT_STR))
            LOGD("AUTHENTICATION_FAILURE_EVENT...");
        else if (strncmp(event_str, RWifiHalCmd::WPS_SUCCESS_STR, strlen(RWifiHalCmd::WPS_SUCCESS_STR)) == 0)
            LOGD("WPS_SUCCESS_EVENT...");
        else if (strncmp(event_str, RWifiHalCmd::WPS_FAIL_STR, strlen(RWifiHalCmd::WPS_FAIL_STR)) == 0)
            LOGD("WPS_FAIL_EVENT...");
        else if (strncmp(event_str, RWifiHalCmd::WPS_OVERLAP_STR, strlen(RWifiHalCmd::WPS_OVERLAP_STR)) == 0)
            LOGD("WPS_OVERLAP_EVENT...");
        else if (strncmp(event_str, RWifiHalCmd::WPS_TIMEOUT_STR, strlen(RWifiHalCmd::WPS_TIMEOUT_STR)) == 0)
            LOGD("WPS_TIMEOUT_EVENT...");
        else
            LOGD("don't handle %s", event_str);

        return false;
    }

    char* space = strchr(event_str, ' ');
    if (!space) return false;

    char eventName[64] = {0};
    memcpy(eventName, event_str + event_prefix_len, space - (event_str + event_prefix_len));

    if (strcmp(eventName, RWifiHalCmd::CONNECTED_STR) == 0)
    {
        LOGD("CONNECTED");
        handleNetworkConnect();
    }
    else if (strcmp(eventName, RWifiHalCmd::DISCONNECTED_STR) == 0)
    {
        LOGD("DISCONNECTED");
        handleNetworkDisconnect();
    }
    else if (strcmp(eventName, RWifiHalCmd::STATE_CHANGE_STR) == 0)
        LOGD("STATE_CHANGE");
    else if (strcmp(eventName, RWifiHalCmd::SCAN_RESULTS_STR) == 0)
        LOGD("SCAN_RESULTS");
    else if (strcmp(eventName, RWifiHalCmd::LINK_SPEED_STR) == 0)
        LOGD("LINK_SPEED");
    else if (strcmp(eventName, RWifiHalCmd::TERMINATING_STR) == 0)
    {
        LOGD("TERMINATING");
        handleNetworkDisconnect();
        ret = true;
    }
    else if (strcmp(eventName, RWifiHalCmd::DRIVER_STATE_STR) == 0)
        LOGD("DRIVER_STATE");
    else if (strcmp(eventName, RWifiHalCmd::EAP_FAILURE_STR) == 0)
        LOGD("EAP_FAILURE");
    else if (strcmp(eventName, RWifiHalCmd::ASSOC_REJECT_STR) == 0)
        LOGD("ASSOC_REJECT");
    else
        LOGD("Unknown event");

    //TODO: handle DRIVER_STATE/LINK_SPEED
    //TODO: handle STATE_CHANGE
    //TODO: handle TERMINATING
    //TODO: handle EAP_FAILURE/ASSOC_REJECT

    return ret;
}

void* RWifiManager::processEvent(void* arg)
{
    RWifiManager* manager = (RWifiManager*)arg;

    const char* event_str = NULL;
    while (manager->mIsMonitoring)
    {
        event_str = manager->mWifiCmd->waitForEvent();
        if (!event_str) continue;

        if (strstr(event_str, RWifiHalCmd::SCAN_RESULTS_STR))
            LOGD("Event [%s]", event_str);

        if (strncmp(event_str, RWifiHalCmd::mInterfacePrefix, strlen(RWifiHalCmd::mInterfacePrefix)) == 0)
        {
            //TODO: parse getting ifname("IFNAME=wlan0 ")
            event_str = strchr(event_str, ' ');
            event_str++;
        }

        if (manager->dispatchEvent(event_str)) break;
    }

    LOGI("exit wifi event process");
    pthread_exit((void*)"exit wifi event process");
    return (void*)NULL;
}

int RWifiManager::connectToWifi(const char* ssid, const char* pwd, int security)
{
    if (!ssid || strlen(ssid) == 0) return -1;

    bool ret = false;
    int netId = mWifiInfo->isNewWifi(ssid);
    if (netId < 0)
    {
        netId = mWifiCmd->addNetwork();
        if (netId < 0)
        {
            LOGE("add network failed!");
            return -1;
        }

        ret = setWifiSsid(netId, ssid);
        if (!ret)
            goto error_out;
    }

   ret = setWifiPasswd(netId, pwd, security);
   if (!ret)
       goto error_out;

   ret = mWifiCmd->enableNetwork(netId, false);
   if (!ret)
   {
       LOGE("enable %d net failed!", netId);
       goto error_out;
   }

   return 0;

error_out:
   mWifiCmd->removeNetwork(netId);
   return -1;
}

bool RWifiManager::setWifiSsid(int netId, const char* ssid)
{
    bool ret = mWifiCmd->setNetworkVariable(netId,
            RWifiHalCmd::mSsidVarName,
            mSsidTool->getSSIDFormat(ssid));

    if (!ret)
        LOGE("set ssid %s(%d) failed!", ssid, netId);

    return ret;
}

bool RWifiManager::setWifiPasswd(int netId, const char* pwd, int security)
{
    bool ret = false;
    if (!pwd || strlen(pwd) == 0)
    {
        ret = mWifiCmd->setNetworkVariable(netId,
                RWifiHalCmd::mKeyMgmtVarName,
                RWifiHalCmd::mKeyMgmtStr[0]);

        if (!ret)
            LOGE("set none passwd(%d) failed!", netId);
    }
    else
    {
        if (security == PASSWD_WPA_PSK)
        {
            ret = mWifiCmd->setNetworkVariable(netId,
                    RWifiHalCmd::mPskVarName,
                    mSsidTool->getPwdFormat(pwd));

            if (!ret)
                LOGE("set wpa psk passwd(%d) failed!", netId);
        }
        else if (security == PASSWD_WEB_KEY)
        {
            ret = mWifiCmd->setNetworkVariable(netId,
                    RWifiHalCmd::mKeyMgmtVarName,
                    RWifiHalCmd::mKeyMgmtStr[0]);

            if (!ret)
            {
                LOGE("set web key passwd-keymgt(%d) failed", netId);
                goto error_out;
            }

            char tmp[64] = {0};
            snprintf(tmp, sizeof(tmp), "%s %s", RWifiHalCmd::mAuthAlgStr[0], RWifiHalCmd::mAuthAlgStr[1]);
            ret = mWifiCmd->setNetworkVariable(netId,
                    RWifiHalCmd::mAuthAlgVarName,
                    mSsidTool->getPwdFormat(tmp));

            if (!ret)
            {
                LOGE("set web key passwd-auth_alg(%d) failed", netId);
                goto error_out;
            }

            ret = mWifiCmd->setNetworkVariable(netId,
                    RWifiHalCmd::mWebKeyVarNames[0],
                    mSsidTool->getPwdFormat(pwd));
            if (!ret)
                LOGE("set web key passwd(%d) failed", netId);
        }
    }

error_out:

return ret;
}

const char* RWifiManager::getSSID()
{
    return mWifiInfo->getCurrentSSID();
}

int RWifiManager::getWifiSignal()
{
    return mWifiInfo->getCurrentSignal();
}

const char* RWifiManager::getIpAddr()
{
    if (mIsConnected)
        return mNetUtils->getDhcpResults()->mIpAddr;
    else
        return NULL;
}

int RWifiManager::ping(const char* ip_host, int packet_num)
{
    if (!ip_host || strlen(ip_host) == 0 || packet_num <= 0) return -1;

    char ping_cmd[64] = {0};
    snprintf(ping_cmd, 64, "/system/bin/ping -c %d %s", packet_num, ip_host);

    FILE *stream = popen(ping_cmd, "r");
    if (stream == NULL)
    {
        LOGE("execute ping command failed: %s", strerror(errno));
        return -1;
    }

    int trans = -1, received = -1;

    char buf[1024] = {0};
    while (fgets(buf, 1024, stream) != NULL)
    {
        LOGD("%s", buf);

        char *finish_flag = strstr(buf, "transmitted");
        if (finish_flag)
        {
            //parse:
            //eg:4 packets transmitted, 4 received, 0% packet loss, time 2999ms
            char field[16] = {0};
            char *index = strchr(buf, ' ');
            if (index)
            {
                memcpy(field, buf, index - buf);
                trans = atoi(field);

                index = strchr(buf, ',');
                if (index)
                {
                    char *tmp = strchr(index + 2, ' ');
                    memset(field, 0, sizeof(field));
                    memcpy(field, index + 2, tmp - (index + 2));
                    received = atoi(field);
                }
            }
        }

        memset(buf, 0, 1024);
    }

    pclose(stream);

    if (trans != packet_num) return -2;

    return received;
}
