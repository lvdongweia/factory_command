#define LOG_TAG "RNetworkManagement"

#include "RNetworkManagement.h"
#include "RNetdConnector.h"
#include "RWifiHalCmd.h"
#include "factory_log.h"

static const char* DEFAULT_DHCP_RANGE[] = {
    "192.168.42.2", "192.168.42.254", "192.168.43.2", "192.168.43.254",
    "192.168.44.2", "192.168.44.254", "192.168.45.2", "192.168.45.254",
    "192.168.46.2", "192.168.46.254", "192.168.47.2", "192.168.47.254",
    "192.168.48.2", "192.168.48.254",
};

static const char* DNS_DEFAULT_SERVER[] = {"8.8.8.8", "8.8.4.4"};

RNetworkManagement* RNetworkManagement::mManagement = NULL;

//TODO: set connector listener

RNetworkManagement* RNetworkManagement::getInstance()
{
    if (mManagement == NULL)
        mManagement = new RNetworkManagement();
    return mManagement;
}

void RNetworkManagement::releaseInstance()
{
    if (mManagement)
        delete mManagement;

    mManagement = NULL;
}

RNetworkManagement::RNetworkManagement()
{
    mSNBase = 0;

    mConnector = new RNetdConnector();
    mConnector->startMonitoring();
}

RNetworkManagement::~RNetworkManagement()
{
    mConnector->stopMonitoring();
    delete mConnector;
}

int RNetworkManagement::getCmdSN()
{
    if (mSNBase > 1000)
        mSNBase = 1;
    else
        ++mSNBase;

    return mSNBase;
}

void RNetworkManagement::startTethering()
{
    RNetdConnector::RCommand command("tether", getCmdSN());
    command.appendArg("start");
    for (size_t i = 0; i < sizeof(DEFAULT_DHCP_RANGE)/sizeof(DEFAULT_DHCP_RANGE[0]); i++)
        command.appendArg(DEFAULT_DHCP_RANGE[i]);

    mConnector->doCommand(command.getArgNum(), command.getArgArray());
    setDnsForwarders(DNS_DEFAULT_SERVER, sizeof(DNS_DEFAULT_SERVER)/sizeof(DNS_DEFAULT_SERVER[0]));
}

void RNetworkManagement::stopTethering()
{
    RNetdConnector::RCommand command("tether", getCmdSN());
    command.appendArg("stop");
    mConnector->doCommand(command.getArgNum(), command.getArgArray());
}

void RNetworkManagement::setDnsForwarders(const char* const * dns, int num)
{
    RNetdConnector::RCommand command("tether", getCmdSN());
    command.appendArg("dns");
    command.appendArg("set");

    for (int i = 0; i < num; i++)
        if (command.appendArg(dns[i]) < 0)
            break;

    mConnector->doCommand(command.getArgNum(), command.getArgArray());
}

void RNetworkManagement::wifiFirmwareReload()
{
    wifiFirmwareReload(RWifiHalCmd::mDefaultInterface, "STA");
}

void RNetworkManagement::wifiFirmwareReload(const char* ifname, const char* mode)
{
    RNetdConnector::RCommand command("softap", getCmdSN());
    command.appendArg("fwreload");
    command.appendArg(ifname);
    command.appendArg(mode);
    mConnector->doCommand(command.getArgNum(), command.getArgArray());
}

void RNetworkManagement::setInterfaceUp(const char* ifname)
{
    if (ifname && strlen(ifname) != 0)
        setInterfaceConfig(ifname, "up");
    else
        setInterfaceConfig(RWifiHalCmd::mDefaultInterface, "up");
}

void RNetworkManagement::setInterfaceDown(const char* ifname)
{
    if (ifname && strlen(ifname) != 0)
        setInterfaceConfig(ifname, "down");
    else
        setInterfaceConfig(RWifiHalCmd::mDefaultInterface, "down");
}

void RNetworkManagement::getInterfaceConfig(const char* ifname)
{
    //TODO: parse respone message form netd connector, and get config info
}

void RNetworkManagement::setInterfaceConfig(const char* ifname, const char* flag)
{
    RNetdConnector::RCommand command("interface", getCmdSN());
    command.appendArg("setcfg");
    command.appendArg(ifname);

    //get config info by calling getInterfaceConfig
    command.appendArg("0.0.0.0");
    command.appendArg("0");

    command.appendArg(flag);
    mConnector->doCommand(command.getArgNum(), command.getArgArray());
}

void RNetworkManagement::clearInterfaceAddresses(const char* ifname)
{
    RNetdConnector::RCommand command("interface", getCmdSN());
    command.appendArg("clearaddrs");

    if (!ifname)
        command.appendArg(RWifiHalCmd::mDefaultInterface);
    else
        command.appendArg(ifname);

    mConnector->doCommand(command.getArgNum(), command.getArgArray());
}
