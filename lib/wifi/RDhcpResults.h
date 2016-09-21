#ifndef __RDHCP_RESULTS_H_
#define __RDHCP_RESULTS_H_

#include <cutils/properties.h>
#include <stdio.h>
#include <unistd.h>

class RDhcpResults
{
public:
    RDhcpResults();
    ~RDhcpResults();

    void clear();
    void dump() const;
    int getDnsNum() const;

public:
    char mInterfaceName[16];

    char mIpAddr[PROPERTY_VALUE_MAX];
    uint32_t mPrefixLength;

    char mGateway[PROPERTY_VALUE_MAX];
    char    mDns1[PROPERTY_VALUE_MAX];
    char    mDns2[PROPERTY_VALUE_MAX];
    char    mDns3[PROPERTY_VALUE_MAX];
    char    mDns4[PROPERTY_VALUE_MAX];
    char    *mDns[5];

    char  mServer[PROPERTY_VALUE_MAX];

    uint32_t mLease;

    char mVendorInfo[PROPERTY_VALUE_MAX];
    char mDomains[PROPERTY_VALUE_MAX];
    char mMtu[PROPERTY_VALUE_MAX];

private:
};

#endif
