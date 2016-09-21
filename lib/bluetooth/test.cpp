#include "bluetooth/RBluetoothManager.h"

#include <stdio.h>
#include <unistd.h>

#define TIMEOUT_LEVEL1 5
#define TIMEOUT_LEVEL2 10
#define TIMEOUT_LEVEL3 15

int bt_main(int argc, char **argv)
{
    bool ret = false;

    int signal;
    int timeout_level1;
    int timeout_level2;
    int timeout_level3;

    //uint8_t tmp_addr[6] = {0x84, 0x11, 0x9E, 0x94, 0xB7, 0x3C};
    uint8_t tmp_addr[6] = {0xa8, 0x66, 0x7f, 0xe2, 0x1c, 0x51};
    RBluetoothAddr addr;
    memcpy(addr.addr, tmp_addr, 6);

    ////////////////////////// init/open ////////////////////
    RBluetoothManager *manager = RBluetoothManager::instance();
    printf("open bt...\n");
    ret = manager->openBt();
    if (!ret)
        printf("open bt failed\n");

    printf("wait opened...\n");
    timeout_level2 = TIMEOUT_LEVEL2;
    while (timeout_level2-- > 0 && !manager->isOpened())
        sleep(1);

    if (timeout_level2 <= 0)
    {
        printf("Error open Bluetooth\n");
        goto error_out;
    }

    ////////////////////////  discovery ////////////////////
    manager->startDiscovery();

    printf("wait found...\n");
    timeout_level3 = TIMEOUT_LEVEL3;
    while(timeout_level3-- > 0 && !manager->isFound(addr))
        sleep(1);

    if (timeout_level3 <= 0)
        printf("search device timeout\n");

    manager->cancelDiscovery();

    printf("wait cancel discovery...\n");
    timeout_level1 = TIMEOUT_LEVEL1;
    while(timeout_level1-- > 0 && manager->isDiscovering())
        sleep(1);

    if (timeout_level1 <= 0)
        printf("cancel discovery timeout\n");

    //Note: must be cancelDiscovery before pairing

    ///////////////////////// pairing /////////////////////////
    printf("pairing...\n");
error_out3:
    ret = manager->pairing(addr);
    if (!ret)
    {
        printf("pairing failed\n");
        goto error_out1;
    }

    printf("wait bond...\n");
    timeout_level3 = TIMEOUT_LEVEL3;
    while(timeout_level3-- > 0 && !manager->isBond())
        sleep(1);

    if (timeout_level3 <= 0)
    {
        printf("Error bond timeout\n");
        //TODO: retry bond
        goto error_out1;
    }
    printf("bond finished\n");

    if (manager->isFound())
    {
        printf("get rssi...\n");
        signal = manager->getSignalStrength();
        printf("signal streng: %d\n", signal);
    }

    ///////////////////////// cancel bond ////////////////////
error_out2:
    printf("remove bond...\n");
    ret = manager->cancelBond();
    if (!ret)
        printf("cancel bond failed\n");

    printf("wait cancel...\n");
    timeout_level1 = TIMEOUT_LEVEL1;
    while(timeout_level1-- > 0 && manager->isBond())
        sleep(1);

    if (timeout_level1 <= 0)
    {
        printf("cancel bond error[timeout]\n");
        printf("retry cancel\n");
        //TODO: retry cancel
    }

    ///////////////////////// close bt ////////////////////
error_out1:
    printf("close bt...\n");
    manager->closeBt();

    printf("wait close bt...\n");
    timeout_level1 = TIMEOUT_LEVEL1;
    while(timeout_level1-- > 0 && manager->isOpened())
        sleep(1);

    if (timeout_level1 <= 0)
    {
        printf("close bt error[timeout]\n");
        printf("retry close\n");
        //TODO: retry close
    }

error_out:
    printf("clean...\n");
    RBluetoothManager::release();
    return 0;
}

int main(int argc, char** argv)
{
    bt_main(argc, argv);

    printf("wait re-start...\n");
    sleep(3);

    bt_main(argc, argv);

    return 0;
}
