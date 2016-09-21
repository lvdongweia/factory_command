#ifndef __RWIFI_HAL_CMD_H_
#define __RWIFI_HAL_CMD_H_

#include <stdio.h>
#include <unistd.h>

class RWifiHalCmd
{
public:
    //set value arg
    static const char* mSsidVarName;
    static const char* mPskVarName;

    static const char* mKeyMgmtVarName;
    static const char* mKeyMgmtStr[5];

    static const char* mAuthAlgVarName;
    static const char* mAuthAlgStr[3];

    static const char* mWebKeyVarNames[4];

public:
    static const char* mDefaultInterface;
    static const char* mInterfacePrefix;

    //wait for event str
    static const char* EVENT_PREFIX_STR;
    static const char* CONNECTED_STR;
    static const char* DISCONNECTED_STR;
    static const char* STATE_CHANGE_STR;
    static const char* SCAN_RESULTS_STR;
    static const char* LINK_SPEED_STR;
    static const char* TERMINATING_STR;
    static const char* WPA_EVENT_PREFIX_STR;
    static const char* PASSWORD_MAY_BE_INCORRECT_STR;
    static const char* WPS_SUCCESS_STR;
    static const char* WPS_FAIL_STR;
    static const char* WPS_OVERLAP_STR;
    static const char* WPS_TIMEOUT_STR;
    static const char* DRIVER_STATE_STR;
    static const char* EAP_FAILURE_STR;
    static const char* EAP_AUTH_FAILURE_STR;
    static const char* ASSOC_REJECT_STR;

    enum ScanType
    {
        SCAN_WITHOUT_CONNECTION_SETUP = 1,
        SCAN_WITH_CONNECTION_SETUP = 2,
    };

public:
    RWifiHalCmd();
    ~RWifiHalCmd();

    bool loadDriver();
    bool isDriverLoaded();
    bool unloadDriver();
    bool startSupplicant(bool p2pSupported);
    bool killSupplicant(bool p2pSupported);
    bool connectToSupplicant();
    void closeSupplicantConnection();
    const char* waitForEvent();

    bool ping();
    bool scan(int type);
    bool stopSupplicant();
    const char* listNetworks();
    int addNetwork();
    bool setNetworkVariable(int netId, const char* name, const char* value);
    const char* getNetworkVariable(int netId, const char* name);
    bool removeNetwork(int netId);
    bool enableNetwork(int netId, bool disableOthers);
    bool disableNetwork(int netId);
    bool reconnect();
    bool reassociate();
    bool disconnect();
    const char* status();
    const char* getMacAddress();
    const char* scanResults(int sid);
    bool startDriver();
    bool stopDriver();
    bool saveConfig();
    const char* signalPoll();

private:
    //wpa_supplicant's maximum size.
    static const int REPLY_BUF_SIZE = 4096;
    static const int EVENT_BUF_SIZE = 2048;

    static const int CMD_MAX_LEN = 256;

private:
    bool doBoolCommand(const char* command);
    int doIntCommand(const char* command);
    const char* doStringCommand(const char* command);

    bool doCommand(const char* cmd,char* reply, size_t reply_len);

    char mReplyBuf[REPLY_BUF_SIZE];
    char mEventBuf[EVENT_BUF_SIZE];
};

#endif
