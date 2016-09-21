#define LOG_TAG "RNetworkUtils"

#include "RNetworkUtils.h"
#include "RDhcpResults.h"
#include "factory_log.h"

#include <netutils/ifc.h>
#include <netutils/dhcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

RNetworkUtils::RNetworkUtils()
{
    mResults = new RDhcpResults();
}

RNetworkUtils::~RNetworkUtils()
{
    delete mResults;
}

int RNetworkUtils::enableInterface(const char* interfaceName)
{
    return ::ifc_enable(interfaceName);
}

int RNetworkUtils::disableInterface(const char* interfaceName)
{
    return ::ifc_disable(interfaceName);
}

int RNetworkUtils::resetConnections(const char* ifname, int mask)
{
    return ::ifc_reset_connections(ifname, mask);
}

bool RNetworkUtils::runDhcp(const char* ifname)
{
    return runDhcpCommon(ifname, false);
}

bool RNetworkUtils::runDhcpRenew(const char* ifname)
{
    return runDhcpCommon(ifname, true);
}

bool RNetworkUtils::stopDhcp(const char* ifname)
{
    mResults->clear();

    int result = ::dhcp_stop(ifname);
    if (result)
        LOGE("dhcp_stop %s failed: %d", ifname, result);
    return (result == 0);
}

bool RNetworkUtils::releaseDhcpLease(const char* ifname)
{
    int result = ::dhcp_release_lease(ifname);
    return (result == 0);
}

const char* RNetworkUtils::getDhcpError()
{
    return ::dhcp_get_errmsg();
}

void RNetworkUtils::markSocket(int socketfd, int mark)
{
    if (setsockopt(socketfd, SOL_SOCKET, SO_MARK, &mark, sizeof(mark)) < 0)
        LOGE("Error marking socket");
}

bool RNetworkUtils::runDhcpCommon(const char* ifname, bool renew)
{
    int result;

    mResults->clear();

    if (ifname == NULL) return false;
    strcpy(mResults->mInterfaceName, ifname);

    if (renew)
    {
        result = ::dhcp_do_request_renew(ifname, mResults->mIpAddr, mResults->mGateway,
                &(mResults->mPrefixLength), mResults->mDns, mResults->mServer,
                &mResults->mLease, mResults->mVendorInfo, mResults->mDomains,
                mResults->mMtu);
    }
    else
    {
        result = ::dhcp_do_request(ifname, mResults->mIpAddr, mResults->mGateway,
                &(mResults->mPrefixLength), mResults->mDns, mResults->mServer,
                &mResults->mLease, mResults->mVendorInfo, mResults->mDomains,
                mResults->mMtu);
    }

    if (result != 0)
    {
        LOGE("dhcp_do_request failed : %s (%s)", ifname, renew ? "renew" : "new");
        mResults->clear();
        return false;
    }

    if (!result)
    {
        mResults->dump();
        return true;
    }

    return false;
}

const RDhcpResults* RNetworkUtils::getDhcpResults()
{
    return mResults;
}
