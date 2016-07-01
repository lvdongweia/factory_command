/*************************************************************************
	> File Name: factory_test_main.c
	> Author: 
	> Mail:  
	> Created Time: 2016年06月29日 星期三 16时19分42秒
 ************************************************************************/

#include <stdio.h>
#include <unistd.h>

#include "factory_test_log.h"
#include "transport.h"



static void event_loop()
{
    for (;;)
    {
        sleep(5);

    }
}


int main(int argc, char **argv)
{
    LOGD("Start factory_Test");
 
    transport_init();
    
    event_loop();

    return 0;
}
