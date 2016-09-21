#define LOG_TAG "factory_test"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "wifi/RWifiManager.h"
#include "factory_log.h"

int main(int argc, char** argv)
{
    int timeout = 0;
    bool delay = false;

    if (argc == 2)
        timeout = atoi(argv[1]);

    RWifiManager* manager = RWifiManager::getInstance();

    printf("openWifi...\n");
    int ret = manager->openWifi();
    if (ret < 0) return -1;

    printf("start connect wifi...\n");
    ret = manager->connectToWifi("302", "archermindx", 1);
    //ret = manager->connectToWifi("302");
    if (ret < 0)
        goto close_wifi;

    printf("wait connected wifi...\n");
    delay = timeout == 0 ? false : true;
    while (!manager->isConnected())
    {
        sleep(1);

        if (delay)
        {
            if (timeout-- > 0)
                continue;
            else
                break;
        }
    }

    if (delay && timeout <= 0)
    {
        ret = -1;
        goto close_wifi;
    }

    printf("connected wifi!!!\n");

    printf("SSID  : %s\n", manager->getSSID());
    printf("IP    : %s\n", manager->getIpAddr());
    printf("Signal: %d\n", manager->getWifiSignal());

    ret = manager->ping("www.baidu.com", 20);
    printf("ping sucess packets: %d\n", ret);

close_wifi:
    printf("close wifi...\n");
    manager->closeWifi();

    RWifiManager::releaseInstance();
    return ret;
}
