/*************************************************************************
	> File Name: transport.h
	> Author: 
	> Mail:  
	> Created Time: 2016年06月30日 星期四 14时13分54秒
 ************************************************************************/

#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <pthread.h>

/* data protocol */
#define MSG_HEADER    0xC0
#define MSG_TYPE_CMD  0x01
#define MSG_TYPE_ACK  0x10
#define MSG_TYPE_NACK 0x80
#define MSG_TYPE (MSG_TYPE_CMD | MSG_TYPE_ACK | MSG_TYPE_NACK)

   
struct message
{
    uint8_t header;
    uint8_t type;
    uint8_t cmd;
    uint8_t d_len;
    uint8_t *data;
};

struct amessage
{
    message msg;
    uint8_t check;
};


typedef void* (*ft_thread_func_t)(void *arg);

static __inline__ int ft_pthread_create(pthread_t *pthread, ft_thread_func_t func, void *arg)
{
    pthread_attr_t attr;

    pthread_attr_init (&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    return pthread_create(pthread, &attr, func, arg);
}


void transport_init();





#endif /* TRANSPROT_H */

