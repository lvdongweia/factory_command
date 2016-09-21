#define LOG_TAG "RWifiHalCmd"

#include "RWifiHalCmd.h"
#include "factory_log.h"

#include <hardware_legacy/wifi.h>
#include <stdlib.h>

const char* RWifiHalCmd::mSsidVarName = "ssid";
const char* RWifiHalCmd::mPskVarName = "psk";

const char* RWifiHalCmd::mKeyMgmtVarName = "key_mgmt";
const char* RWifiHalCmd::mKeyMgmtStr[5] = {"NONE", "WPA_PSK", "WPA_EAP", "IEEE8021X", "WPA2_PSK"};

const char* RWifiHalCmd::mAuthAlgVarName = "auth_alg";
const char* RWifiHalCmd::mAuthAlgStr[3] = {"OPEN", "SHARED", "LEAP"};

const char* RWifiHalCmd::mWebKeyVarNames[4] = {"wep_key0", "wep_key1", "wep_key2", "wep_key3"};

const char* RWifiHalCmd::mInterfacePrefix = "IFNAME=";
const char* RWifiHalCmd::mDefaultInterface = "wlan0";

const char* RWifiHalCmd::EVENT_PREFIX_STR = "CTRL-EVENT-";
const char* RWifiHalCmd::CONNECTED_STR = "CONNECTED";
const char* RWifiHalCmd::DISCONNECTED_STR = "DISCONNECTED";
const char* RWifiHalCmd::STATE_CHANGE_STR = "STATE-CHANGE";
const char* RWifiHalCmd::SCAN_RESULTS_STR = "SCAN-RESULTS";
const char* RWifiHalCmd::LINK_SPEED_STR = "LINK-SPEED";
const char* RWifiHalCmd::TERMINATING_STR = "TERMINATING";
const char* RWifiHalCmd::WPA_EVENT_PREFIX_STR = "WPA:";
const char* RWifiHalCmd::PASSWORD_MAY_BE_INCORRECT_STR = "pre-shared key may be incorrect";
const char* RWifiHalCmd::WPS_SUCCESS_STR = "WPS-SUCCESS";
const char* RWifiHalCmd::WPS_FAIL_STR = "WPS-FAIL";
const char* RWifiHalCmd::WPS_OVERLAP_STR = "WPS-OVERLAP-DETECTED";
const char* RWifiHalCmd::WPS_TIMEOUT_STR = "WPS-TIMEOUT";
const char* RWifiHalCmd::DRIVER_STATE_STR = "DRIVER-STATE";
const char* RWifiHalCmd::EAP_FAILURE_STR = "EAP-FAILURE";
const char* RWifiHalCmd::EAP_AUTH_FAILURE_STR = "EAP authentication failed";
const char* RWifiHalCmd::ASSOC_REJECT_STR = "ASSOC-REJECT";

RWifiHalCmd::RWifiHalCmd()
{
    memset(mEventBuf, 0, EVENT_BUF_SIZE);
    memset(mReplyBuf, 0, REPLY_BUF_SIZE);
}

RWifiHalCmd::~RWifiHalCmd()
{
}

bool RWifiHalCmd::doCommand(const char* cmd,char* reply, size_t reply_len)
{
    char cmd_str[CMD_MAX_LEN] = {0};
    snprintf(cmd_str, sizeof(cmd_str), "%s%s %s", mInterfacePrefix, mDefaultInterface, cmd);
    LOGD("Command string:%s", cmd_str);

    --reply_len; // Ensure we have room to add NUL termination.
    if (::wifi_command(cmd_str, reply, &reply_len) != 0)
        return false;

    // Strip off trailing newline.
    if (reply_len > 0 && reply[reply_len-1] == '\n')
        reply[reply_len-1] = '\0';
    else
        reply[reply_len] = '\0';

    return true;
}

bool RWifiHalCmd::doBoolCommand(const char* cmd)
{
    //TODO: sync lock
    char reply[REPLY_BUF_SIZE];
    if (!doCommand(cmd, reply, sizeof(reply))) {
        return false;
    }

    return (strcmp(reply, "OK") == 0);
}

int RWifiHalCmd::doIntCommand(const char* cmd)
{
    //TODO: sync lock
    char reply[REPLY_BUF_SIZE];
    if (!doCommand(cmd, reply, sizeof(reply)))
        return -1;

    return atoi(reply);
}

const char* RWifiHalCmd::doStringCommand(const char* cmd)
{
    //TODO: sync lock
    memset(mReplyBuf, 0, REPLY_BUF_SIZE);
    if (!doCommand(cmd, mReplyBuf, sizeof(mReplyBuf))) {
        return NULL;
    }

    return mReplyBuf;
}

bool RWifiHalCmd::loadDriver()
{
    return (::wifi_load_driver() == 0);
}

bool RWifiHalCmd::isDriverLoaded()
{
    return (::is_wifi_driver_loaded() == 1);
}

bool RWifiHalCmd::unloadDriver()
{
    return (::wifi_unload_driver() == 0);
}

bool RWifiHalCmd::startSupplicant(bool p2pSupported)
{
    return (::wifi_start_supplicant(p2pSupported) == 0);
}

bool RWifiHalCmd::killSupplicant(bool p2pSupported)
{
    return (::wifi_stop_supplicant(p2pSupported) == 0);
}

bool RWifiHalCmd::connectToSupplicant()
{
    return (::wifi_connect_to_supplicant() == 0);
}

void RWifiHalCmd::closeSupplicantConnection()
{
    ::wifi_close_supplicant_connection();
}

const char* RWifiHalCmd::waitForEvent()
{
    memset(mEventBuf, 0, EVENT_BUF_SIZE);
    int nread = ::wifi_wait_for_event(mEventBuf, sizeof(mEventBuf));
    if (nread > 0)
        return mEventBuf;
    else
        return NULL;
}

bool RWifiHalCmd::ping()
{
    const char* pong = doStringCommand("PING");
    return (pong && strcmp(pong, "PONG"));
}

bool RWifiHalCmd::scan(int type)
{
    if (type == SCAN_WITHOUT_CONNECTION_SETUP)
        return doBoolCommand("SCAN TYPE=ONLY");
    else if (type == SCAN_WITH_CONNECTION_SETUP)
        return doBoolCommand("SCAN");
    else
        LOGE("Invalid scan type");

    return false;
}

bool RWifiHalCmd::stopSupplicant()
{
    return doBoolCommand("TERMINATE");
}

const char* RWifiHalCmd::listNetworks()
{
    return doStringCommand("LIST_NETWORKS");
}

int RWifiHalCmd::addNetwork()
{
    return doIntCommand("ADD_NETWORK");
}

bool RWifiHalCmd::setNetworkVariable(int netId, const char* name, const char* value)
{
    char cmd_str[CMD_MAX_LEN] = {0};
    snprintf(cmd_str, sizeof(cmd_str), "SET_NETWORK %d %s %s", netId, name, value);
    return doBoolCommand(cmd_str);
}

const char* RWifiHalCmd::getNetworkVariable(int netId, const char* name)
{
    char cmd_str[CMD_MAX_LEN] = {0};
    snprintf(cmd_str, sizeof(cmd_str), "GET_NETWORK %d %s", netId, name);
    return doStringCommand(cmd_str);
}

bool RWifiHalCmd::removeNetwork(int netId)
{
    char cmd_str[CMD_MAX_LEN] = {0};
    snprintf(cmd_str, sizeof(cmd_str), "REMOVE_NETWORK %d", netId);
    return doBoolCommand(cmd_str);
}

bool RWifiHalCmd::enableNetwork(int netId, bool disableOthers)
{
    char cmd_str[CMD_MAX_LEN] = {0};
    if (disableOthers)
    {
        snprintf(cmd_str, sizeof(cmd_str), "SELECT_NETWORK %d", netId);
        return doBoolCommand(cmd_str);
    }
    else
    {
        snprintf(cmd_str, sizeof(cmd_str), "ENABLE_NETWORK %d", netId);
        return doBoolCommand(cmd_str);
    }
}

bool RWifiHalCmd::disableNetwork(int netId)
{
    char cmd_str[CMD_MAX_LEN] = {0};
    snprintf(cmd_str, sizeof(cmd_str), "DISABLE_NETWORK %d", netId);
    return doBoolCommand(cmd_str);
}

bool RWifiHalCmd::reconnect()
{
    return doBoolCommand("RECONNECT");
}

bool RWifiHalCmd::reassociate()
{
    return doBoolCommand("REASSOCIATE");
}

bool RWifiHalCmd::disconnect()
{
    return doBoolCommand("DISCONNECT");
}

const char* RWifiHalCmd::status()
{
    return doStringCommand("STATUS");
}

const char* RWifiHalCmd::getMacAddress()
{
    const char* ret = doStringCommand("DRIVER MACADDR");
    if (ret)
    {
        //Macaddr = XX.XX.XX.XX.XX.XX
        ret = strstr(ret, " = ");
        if (ret) return ret + 3;
    }

    return NULL;
}

/**
 * Format of results:
 * =================
 * id=1
 * bssid=68:7f:74:d7:1b:6e
 * freq=2412
 * level=-43
 * tsf=1344621975160944
 * age=2623
 * flags=[WPA2-PSK-CCMP][WPS][ESS]
 * ssid=zubyb
 * ====
 *
 * RANGE=ALL gets all scan results
 * RANGE=ID- gets results from ID
 * MASK=<N> see wpa_supplicant/src/common/wpa_ctrl.h for details
 */
const char* RWifiHalCmd::scanResults(int sid)
{
    char cmd_str[CMD_MAX_LEN] = {0};
    snprintf(cmd_str, sizeof(cmd_str), "BSS RANGE=%d- MASK=0x21987", sid);
    return doStringCommand(cmd_str);
}

bool RWifiHalCmd::startDriver()
{
    return doBoolCommand("DRIVER START");
}

bool RWifiHalCmd::stopDriver()
{
    return doBoolCommand("DRIVER STOP");
}

bool RWifiHalCmd::saveConfig()
{
    return doBoolCommand("SAVE_CONFIG");
}

/** Example output:
 * RSSI=-65
 * LINKSPEED=48
 * NOISE=9999
 * FREQUENCY=0
 */
const char* RWifiHalCmd::signalPoll()
{
    return doStringCommand("SIGNAL_POLL");
}
