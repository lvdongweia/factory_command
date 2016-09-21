/*************************************************************************
	> File Name: test_battery.h
	> Author: 
	> Mail:  
	> Created Time: 2016年08月09日 星期二 17时04分40秒
 ************************************************************************/

#ifndef TEST_BATTERY_H
#define TEST_BATTERY_H

enum {
    CHARGE_IN  = 0x01,
    CHARGE_OUT = 0x02,
};


int get_battery_capacity();

int get_battery_status();



#endif /* TEST_BATTERY_H */
