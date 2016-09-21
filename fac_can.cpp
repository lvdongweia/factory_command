/*************************************************************************
	> File Name: fac_can.cpp
	> Author: 
	> Mail:  
	> Created Time: 2016年07月25日 星期一 09时57分01秒
 ************************************************************************/

#include <utils/Mutex.h>
#include <utils/Condition.h>

#include "RmCANClient_c.h"
#include "robot_common_data.h"

#include "fac_errors.h"
#include "fac_util.h"
#include "fac_can.h"

using namespace android;

#define FAC_TEST_MODULE         RM_FACTORY_TEST_ID
#define MS                      (1000 * 1000)

static Mutex sRecvMutex;
static Condition sRecvSignal;

struct TmpRecvData {
    int      s_id;
    uint8_t *data;
    int      len;
} RData; 

void can_server_died()
{
    LOGE("FACTORY_TEST - Can server died");
}

void recv_can_data(int priority, int s_id, const void *pdata, int len)
{
    AutoMutex lock(sRecvMutex);
    RData.s_id = s_id;
    RData.data = (uint8_t *)pdata;
    RData.len  = len;
    sRecvSignal.signal();
}

struct can_client_callback gClient = 
{
    .RmRecvCANData    = recv_can_data,
    .RmCANServiceDied = can_server_died
};

int send_can_data(uint8_t dst_id, const uint8_t *pdata, int len)
{
    return RmSendCANData(dst_id, pdata, len , ROBOT_CAN_LEVEL0);
}

int read_can_data(uint8_t *pdata, int &len, uint8_t exp_s, 
        uint8_t exp_t, int exp_l, int timeout)
{
    status_t ret;
    AutoMutex lock(sRecvMutex);
    ret = sRecvSignal.waitRelative(sRecvMutex, timeout * MS);
    if (ret != OK) return -1;

    if (exp_s != RData.s_id) return -1;
    if (exp_t != RData.data[0]) return -1;
    if (exp_l != RData.len) return -1;

    if (pdata != NULL)
        memcpy(pdata, RData.data, RData.len);
    len = RData.len;
    return 0;
}


int init_can()
{
    int ret = RmInitCANClient(FAC_TEST_MODULE, &gClient);
    if (ret < 0)
    {
        return -1;
    }
    return 0;
}

void uninit_can()
{
     RmDeinitCANClient();
}
