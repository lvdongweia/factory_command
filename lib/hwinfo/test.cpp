#include "RobotHwInfo.h"
#include "RStorageManager.h"

#include <stdio.h>
#include <unistd.h>

static int hwinfo_main(int argc, char** argv)
{
    MacAddrStore store;

    RobotHwInfo tool;
    const char* sn = tool.getSN();
    if (sn && strlen(sn) != 0)
        printf("SN: %s\n", sn);
    else
        printf("SN is empty\n");

    int ret = tool.getWifiMacAddr(store);
    if (ret == 0)
        printf("Wifi Mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
                store.addr[0], store.addr[1], store.addr[2],
                store.addr[3], store.addr[4], store.addr[5]);
    else
        printf("Wifi Mac is empty\n");

    ret = tool.getEthMacAddr(store);
    if (ret == 0)
        printf("Eth Mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
                store.addr[0], store.addr[1], store.addr[2],
                store.addr[3], store.addr[4], store.addr[5]);
    else
        printf("Eth Mac is empty\n");

    ret = tool.getBtMacAddr(store);
    if (ret == 0)
        printf("BT Mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
                store.addr[0], store.addr[1], store.addr[2],
                store.addr[3], store.addr[4], store.addr[5]);
    else
        printf("BT Mac is empty\n");

    return 0;
}

static int tfcard_main(int argc, char** argv)
{
    RStorageManager* manager = RStorageManager::instance();

    bool isMounted = manager->isTFCardMounted();
    printf("TF Card Mounted State: %d\n", isMounted);

    RStorageManager::release();

    return 0;
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("Usage:\n");
        printf("%s hwinfo: show wifi/bt addr\n", argv[0]);
        printf("%s tfcard: show tfcard state\n", argv[0]);
        return 0;
    }

    if (strcmp(argv[0], "hwinfo") == 0)
        return hwinfo_main(argc, argv);
    else if (strcmp(argv[1], "tfcard") == 0)
        return tfcard_main(argc, argv);

    return 0;
}
