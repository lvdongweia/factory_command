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
#include "factory_test_log.h"
#include "transport.h"

#define BUF_SIZE   255

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

static List<struct message*> qRecvList;

static void printRecvHex(const uint8_t *data, int len)
{
    if (len <= 0 || len > BUF_SIZE) return;

    char str[3*BUF_SIZE + 1] = {0};

    for (int i = 0; i < len; i++)
    {
        sprintf(str+3*i, "%02x ", data[i]);
    }

    str[3*len-1] = '\0';
    LOGD("###Recv data hex:[%s]", str);
}

static void pushToRecvList(struct message *msg)
{
    AutoMutex lock(mRecvLock);
    qRecvList.push_back(msg);
    LOGD("RecvList size:%d", qRecvList.size());
    if (msg->d_len > 0)
    {
        printRecvHex(msg->data, msg->d_len);
    }
    mRecvSignal.signal();
}

static int isValidData(const message *msg, uint8_t check)
{
    uint8_t checksum;

    checksum = msg->header ^ msg->type ^ msg->cmd ^ msg->d_len;
    if (msg->d_len > 0 && msg->data != NULL)
    {
        for (int i = 0; i < msg->d_len; i++)
            checksum ^= msg->data[i];
    }

    LOGD("calculate checksum is %x, origin is %x", checksum, check);

    return checksum == check ? 1 : 0;
}

static message* getPackage(uint8_t header, 
        uint8_t type, 
        uint8_t cmd, 
        uint8_t len,
        uint8_t *data)
{
    uint8_t *payload;
    struct message *package = (struct message*)malloc(sizeof(struct message));
    if (package == NULL) return NULL;

    memset(package, 0, sizeof(struct message));
    package->header = header;
    package->type   = type;
    package->cmd    = cmd;
    package->d_len  = len;

    if (len > 0 && data != NULL)
    {
        payload = (uint8_t*)malloc(sizeof(uint8_t) * len);
        if (payload == NULL) return NULL;
        memcpy(payload, data, len);
        package->data = payload;
    }

    return package;
}

static int read_data()
{
    int len;
    uint8_t buf[BUF_SIZE] = {0};
    struct message *msg;

    uint8_t *payload = NULL;
    uint8_t header, type, cmd, length, checksum; 

    if (!se_state) return -1;
    len = serial_read(se_fd, buf, sizeof(buf));
    LOGD("#####receive data len:%d", len);
    printRecvHex(buf, len);

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

#ifdef STRESS_TEST 
    /* for test serial */
    len = serial_write(se_fd, buf, len);
    LOGD("#####send success data len:%d", len);
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
    if (!(type & MSG_TYPE))
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
            LOGE("Invalid payload len:%d (%d)", pldLen, length);
            return 0;
        }
    }

    checksum = buf[len-1];

    /* construct msg */
    struct message *package = getPackage(header, type, cmd, length, payload);
    if (package == NULL) return 0;

    if (isValidData(package, checksum))
        pushToRecvList(package);
    else 
    {
        LOGE("checksum failed.");
        if (package->data != NULL) free(package->data);
        free(package);
    }

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

