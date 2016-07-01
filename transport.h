/*************************************************************************
	> File Name: transport.h
	> Author: 
	> Mail:  
	> Created Time: 2016年06月30日 星期四 14时13分54秒
 ************************************************************************/

#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <pthread.h>

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

