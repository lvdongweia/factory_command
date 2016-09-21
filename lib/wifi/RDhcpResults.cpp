#define LOG_TAG "RDhcpResults"

#include "RDhcpResults.h"
#include "factory_log.h"

#include <stdlib.h>
#include <string.h>

#define CLEAR_ARRAY(x) memset(x, 0, sizeof(x))

RDhcpResults::RDhcpResults()
{
    clear();
    mDns[0] = mDns1;
    mDns[1] = mDns2;
    mDns[2] = mDns3;
    mDns[3] = mDns4;
    mDns[4] = NULL;
}

RDhcpResults::~RDhcpResults()
{
}

void RDhcpResults::clear()
{
    CLEAR_ARRAY(mInterfaceName);
    CLEAR_ARRAY(mIpAddr);

    mPrefixLength = 0;

    CLEAR_ARRAY(mGateway);
    CLEAR_ARRAY(mDns1);
    CLEAR_ARRAY(mDns2);
    CLEAR_ARRAY(mDns3);
    CLEAR_ARRAY(mDns4);

    CLEAR_ARRAY(mServer);

    mLease = 0;

    CLEAR_ARRAY(mVendorInfo);
    CLEAR_ARRAY(mDomains);
    CLEAR_ARRAY(mMtu);
}

void RDhcpResults::dump() const
{
    LOGD("%s:\n"
            "IP: %s/%u\n"
            "Gateway: %s\n"
            "DNS1: %s\n"
            "DNS2: %s\n"
            "DNS3: %s\n"
            "DNS4: %s\n"
            "Server: %s\n"
            "Lease: %u\n"
            "Vendor: %s\n"
            "Domains: %s\n"
            "MTU: %s",
            mInterfaceName,
            mIpAddr, mPrefixLength,
            mGateway,
            mDns1, mDns2, mDns3, mDns4,
            mServer, mLease, mVendorInfo, mDomains, mMtu);
}

int RDhcpResults::getDnsNum() const
{
    int num = 0;
    for (num = 0; num < 4; num++)
        if (strlen(mDns[num]) == 0) break;

    return num;
}
