/*************************************************************************
	> File Name: transport.c
	> Author: 
	> Mail:  
	> Created Time: 2016年06月30日 星期四 13时55分59秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>
#include <dirent.h>
#include <utils/List.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>

#include "serial.h"
#include "fac_log.h"
#include "fac_util.h"

#include "transport.h"


#define ST_UNMOUNT 0
#define ST_OPEN    1

#define FD_READ    1
#define FD_WRITE   2
#define FD_TIMEOUT 3
#define FD_ERROR   4



using namespace android;

static int se_fd = -1;
static int se_state = ST_UNMOUNT;
static char se_name[32] = {0};

static pthread_t read_id;
static Mutex mRecvLock;
static Condition mRecvSignal;

#ifndef BUILD_FOR_JNI
static List<struct message*> qRecvList;
#else
static List<struct exmessage*> qRecvList;
#endif


#ifndef BUILD_FOR_JNI
static void pushToRecvList(struct message *msg)
{
    AutoMutex lock(mRecvLock);
    qRecvList.push_back(msg);
    LOGD("recv list size:%d", qRecvList.size());
    mRecvSignal.signal();
}

struct message* popFromRecvList()
{
    struct message *packet = NULL;

    AutoMutex lock(mRecvLock);
    if (qRecvList.empty()) mRecvSignal.wait(mRecvLock);

    List<struct message*>::iterator it = qRecvList.begin();
    packet = *it;
    qRecvList.erase(it);

    return packet;
}

static int getChecksum(const message *msg)
{
    uint8_t checksum;
    checksum = msg->header ^ msg->type ^ msg->cmd ^ msg->d_len;
    if (msg->d_len > 0 && msg->data != NULL)
    {
        for (int i = 0; i < msg->d_len; i++)
            checksum ^= msg->data[i];
    }
    return checksum;
}

static message* getPacket(uint8_t header, 
        uint8_t type, 
        uint8_t cmd, 
        uint8_t len,
        uint8_t *data)
{
    uint8_t *payload;
    struct message *packet = (struct message*)malloc(sizeof(struct message));
    if (packet == NULL) return NULL;

    memset(packet, 0, sizeof(struct message));
    packet->header = header;
    packet->type   = type;
    packet->cmd    = cmd;
    packet->d_len  = len;

    if (len > 0 && data != NULL)
    {
        payload = (uint8_t*)malloc(sizeof(uint8_t) * len);
        if (payload == NULL) return NULL;
        memcpy(payload, data, len);
        packet->data = payload;
    }

    return packet;
}

static void releasePacket(struct message *msg)
{
    if (msg == NULL) return;

    if (msg->data) free(msg->data);

    free(msg);
}

static int transport_write(struct message *packet)
{
    int datalen;
    if (!se_state) return -1;

    uint8_t data[BUF_SIZE] = {0};

    data[0] = packet->header;
    data[1] = packet->type;
    data[2] = packet->cmd;
    data[3] = datalen = packet->d_len;
    if (datalen < 0 || datalen > 250) return -1;
    if (datalen > 0 && packet->data == NULL) return -1;

    if (datalen > 0)
    {
        memcpy(data + 4, packet->data, datalen);
        data[4+datalen] = getChecksum(packet);
    }
    else
    {
        data[4] = getChecksum(packet);
    }

    return serial_write(se_fd, data, datalen+5);
}

int getNewEvent(EventCallback cbFun)
{
    /* if no event, it will block */
    struct message *msg = popFromRecvList();
    if (msg == NULL || cbFun == NULL) return -1;

    // callback to handle event
    if (cbFun)
       (*cbFun)(msg->type, msg->cmd, msg->data, msg->d_len);

    releasePacket(msg);
    return 0;
}

int responseEvent(uint8_t type, uint8_t cmd, uint8_t *data, uint8_t len)
{
    struct message *msg = getPacket(MSG_HEADER, type, cmd, len, data);

    // creat thread to write ?
    int ret = transport_write(msg);

    releasePacket(msg);
    return ret;
}


#else

static void pushToRecvList(struct exmessage *msg)
{
    AutoMutex lock(mRecvLock);
    qRecvList.push_back(msg);
    LOGD("RecvList size:%d", qRecvList.size()); 
    mRecvSignal.signal();
}

struct exmessage* popFromRecvList()
{
    struct exmessage *packet = NULL;

    AutoMutex lock(mRecvLock);
    if (qRecvList.empty()) mRecvSignal.wait(mRecvLock);

    List<struct exmessage*>::iterator it = qRecvList.begin();
    packet = *it;
    qRecvList.erase(it);

    return packet;
}

void getSerialData(uint8_t *data, int &len)
{
    struct exmessage *msg = popFromRecvList();

    memcpy(data, msg->data, msg->d_len);
    len = msg->d_len;
}
#endif

static int read_data()
{
    int len;
    uint8_t buf[BUF_SIZE] = {0};
   
    if (!se_state) return -1;
    len = serial_read(se_fd, buf, sizeof(buf));
    LOGD("#####receive data len:%d", len);
    printHex(buf, len);

    if (len < 0)
    {
        LOGE("read serial error.(%s)", strerror(errno));
        return 0;
    }
    if (len == 0) 
    {
        /* when serial device removed. read end of the file
         * and return 0.
         */
        LOGD("serial device removed.");
        return -1;
    }

#ifndef BUILD_FOR_JNI
    struct message *msg;

    uint8_t *payload = NULL;
    uint8_t header, type, cmd, length, checksum; 

#ifdef STRESS_TEST 
    /* for test serial */
    len = serial_write(se_fd, buf, len);
    LOGD("#####send success data len:%d", len);
    return 0;
#endif
 
    /* check msg protocol */
    if (len < 5 || len > 255) 
    {
        LOGE("Invalid data len:%d (5-255)", len);
        return 0;
    }

    header = buf[0];
    if (header != MSG_HEADER) 
    {
        LOGE("Invalid data header:%02x (%02x)", header, MSG_HEADER);
        return 0;
    }

    type = buf[1];
    if (type != MSG_TYPE_CMD && type != MSG_TYPE_ACK && type != MSG_TYPE_NACK)
    {
        LOGE("Invalid data type:%02x (%02x | %02x | %02x)", type, 
                MSG_TYPE_CMD, MSG_TYPE_ACK, MSG_TYPE_NACK);
        return 0;
    }

    cmd = buf[2];

    length = buf[3];
    if (length > 0)
    {
        payload = buf + 4;
        int pldLen = len - 5;
        if (length != pldLen)
        {
            LOGE("Invalid payload len:%d (%d)", length, pldLen);
            return 0;
        }
    }

    checksum = buf[len-1];

    /* construct msg */
    struct message *packet = getPacket(header, type, cmd, length, payload);
    if (packet == NULL) return 0;

    int ck = getChecksum(packet);
    if (checksum == ck)
        pushToRecvList(packet);
    else 
    {
        LOGE("checksum failed.(%02x / %02x)", ck, checksum);
        if (packet->data != NULL) free(packet->data);
        free(packet);
    }

#else

    struct exmessage* msg = (struct exmessage*)malloc(sizeof(struct exmessage));
    if (msg == NULL) return 0;
    memcpy(msg->data, buf, len);
    msg->d_len = len;

    pushToRecvList(msg);

#endif

    return 0;
}

static int wait_transport()
{
    int ret;
    fd_set rfd;
    struct timeval tm;

    tm.tv_sec = 0;
    tm.tv_usec = 1000 * 1000;

    FD_ZERO(&rfd);
    FD_SET(se_fd, &rfd);

    ret = select(se_fd+1, &rfd, NULL, NULL, NULL);
    if (ret > 0)
    {
        if (FD_ISSET(se_fd, &rfd))
            return FD_READ;
    }
    else if (ret == 0)
        return FD_TIMEOUT;

    return FD_ERROR;
}

static void* transport_thread(void *arg)
{
    int action;
    int ret;

    for (;;)
    {
        action = wait_transport();
        LOGD("%d event happen", action);
        if (action == FD_READ)
        {
            ret = read_data();
            if (ret < 0) break;
        }
    }

    LOGD("###################################transport thread exit");

    return 0;
}


static void open_device(char *dev)
{
    se_fd = serial_open(dev);
    if (se_fd > 0)
    {
        se_state = ST_OPEN;
        strcpy(se_name, dev);

        // start reading thread
        ft_pthread_create(&read_id, transport_thread, NULL);
    }
}

static void close_device()
{
    LOGD("##############to close device");
    se_state = ST_UNMOUNT;
    serial_close(se_fd);
    se_fd = -1;
    memset(se_name, 0, sizeof(se_name));

    pthread_join(read_id, NULL);
    LOGD("#############device closed");
    read_id = 0;
}
static int know_device(char *dev)
{
    if (!se_name[0])
        return 0;

    if (strcmp(se_name, dev))
        return 0;

    return 1;
}

static void find_serial_device(char *base) 
{
    DIR *devdir;
    struct dirent *de;
    char devname[32];
    int exist = 0;

    devdir = opendir(base);
    if (devdir == NULL) return;

    while ((de = readdir(devdir)) != NULL) 
    {
        if (strncmp("ttyUSB", de->d_name, 6) == 0)
        {
            sprintf(devname, "/dev/%s", de->d_name);
            exist = 1;
            break;
        }
    }

    if (exist)
    {
        // node exist
        if (se_state == ST_UNMOUNT)
        {
            open_device(devname);
        }
        else if (se_state == ST_OPEN && !know_device(devname))
        {
            close_device();
            open_device(devname);
        }
    }
    else
    {  
        // node is't exist
        if (se_state == ST_OPEN)
            close_device();
    }
}

static void* device_poll_thread(void *arg)
{
    LOGD("start device poll thread.");
    for(;;)
    {
        sleep(1);
        find_serial_device("/dev");
    }

    // never return
    return (NULL);
}

void transport_init()
{
    pthread_t tid;

    // starting serial check thread.
    if (ft_pthread_create(&tid, device_poll_thread, NULL)) 
    {
        LOGE("start device poll thread error.");
        exit(1);
    }

}

