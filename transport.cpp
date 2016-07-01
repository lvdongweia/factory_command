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

#include "serial.h"
#include "factory_test_log.h"
#include "transport.h"


#define ST_UNMOUNT 0
#define ST_OPEN    1

#define FD_READ    1
#define FD_WRITE   2
#define FD_TIMEOUT 3
#define FD_ERROR   4


static int se_fd = -1;
static int se_state = ST_UNMOUNT;
static char se_name[32] = {0};

static pthread_t read_id;

static void open_device(char *dev);
static void close_device();

static int read_data()
{
    char buf[256] = {0};
    int len;

    if (!se_state)
        return -1;

    len = serial_read(se_fd, buf, sizeof(buf));
    LOGD("read data: len=%d, data=%x", len, buf[0]);

    return len;
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

    ret = select(se_fd+1, &rfd, NULL, NULL, &tm);
    if (ret > 0)
    {
        if (FD_ISSET(se_fd, &rfd))
            return FD_READ;
    }
    else if (ret == 0)
        return FD_TIMEOUT;

    return FD_ERROR;
}

static void* input_thread(void *arg)
{
    int action;
    int len;
    uint8_t buf[2];
    buf[0] = 0x01;
    buf[1] = 0x02;

    while(se_state)
    {
        action = wait_transport();
        if (action == FD_READ)
        {
            len = read_data();
            if (len <= 0)
                break;
        }
    }

    LOGD("input_thread exit");

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
        ft_pthread_create(&read_id, input_thread, NULL);
    }
}

static void close_device()
{
    LOGD("to close device");
    se_state = ST_UNMOUNT;
    serial_close(se_fd);
    se_fd = -1;
    memset(se_name, 0, sizeof(se_name));

    pthread_join(read_id, NULL);
    LOGD("device closed");
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
        LOGD("node is't exit");
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

