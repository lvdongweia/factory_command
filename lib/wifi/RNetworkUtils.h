#ifndef __RNETWORK_UTILS_H_
#define __RNETWORK_UTILS_H_

#include <stdio.h>

class RDhcpResults;

class RNetworkUtils
{
public:
    enum ResetMask{
        RESET_IPV4_ADDRESSES = 0x01,
        RESET_IPV6_ADDRESSES = 0x02,
        RESET_ALL_ADDRESSES = RESET_IPV4_ADDRESSES | RESET_IPV6_ADDRESSES,
    };

    RNetworkUtils();
    ~RNetworkUtils();
    const RDhcpResults* getDhcpResults();

    //interface opt
    int enableInterface(const char* ifname);
    int disableInterface(const char* ifname);
    int resetConnections(const char* ifname, int mask);

    //dhcp opt
    bool runDhcp(const char* ifname);
    bool runDhcpRenew(const char* ifname);
    bool stopDhcp(const char* ifname);
    bool releaseDhcpLease(const char* ifname);
    const char* getDhcpError();
    void markSocket(int socketfd, int mark);

private:
    bool runDhcpCommon(const char* ifname, bool renew);
    RDhcpResults* mResults;
};

#endif
