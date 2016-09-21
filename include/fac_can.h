/*************************************************************************
	> File Name: fac_can.h
	> Author: 
	> Mail:  
	> Created Time: 2016年07月25日 星期一 09时56分35秒
 ************************************************************************/

#ifndef FAC_CAN_H
#define FAC_CAN_H


int init_can();
void uninit_can();

int send_can_data(uint8_t dst_id, const uint8_t *pdata, int len);


int read_can_data(uint8_t *pdata, int &len, uint8_t exp_s, 
        uint8_t exp_t, int exp_l, int timeout);


#endif /* FAC_CAN_H */
