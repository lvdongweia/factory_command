/*************************************************************************
	> File Name: factory_test_main.c
	> Author: 
	> Mail:  
	> Created Time: 2016年06月29日 星期三 16时19分42秒
 ************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


#include "command.h"
#include "errors.h"

#include "fac_util.h"
#include "transport.h"
#include "fw_version.h"
#include "fac_log.h"


static void get_fw_version(int id)
{
    char version[128] = {0};
    int ret, len;

    ret = getfw_version(id, version, sizeof(version));
    len = strlen(version);

    LOGD("ver:%s, len:%d", version, len);

    uint8_t hexVer[128] = {0};
    str2Hex(hexVer, version, len);

    responseEvent(MSG_TYPE_NACK, FAC_ID_GET_FW_VERSION, hexVer, len);
}

void handle_event(uint8_t type, uint8_t cmd, uint8_t *data, uint8_t len)
{
    LOGD("[handle_event]: type:%02x cmd:%02x", type, cmd);
    
    uint8_t result[PAYLOAD_SIZE] = {0};

    switch(cmd)
    {
        case FAC_ID_ENTER_FACTORY_MODE:
            result[0] = E_OK;
            responseEvent(MSG_TYPE_ACK, cmd, result, 1);
            break;

        case FAC_ID_EXIT_FACTORY_MODE:
            result[0] = E_OK;
            responseEvent(MSG_TYPE_ACK, cmd, result, 1);
            break;


        case FAC_ID_ENTER_BURNING_MODE:

            break;


        case FAC_ID_EXIT_BURNING_MODE:

            break;

        case FAC_ID_GET_FW_VERSION:
            get_fw_version(data[0]);
            break;


        default:
            LOGD("unhandle command:%02x", cmd);
            break;
    }
}

static void event_loop()
{
    for (;;)
    {
        LOGD("wait for event...");
        getNewEvent(handle_event);
    }
}


int main(int argc, char **argv)
{
    LOGD("Start factory_Test");
 
    transport_init();
    
    event_loop();

    return 0;
}
