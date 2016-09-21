/*************************************************************************
	> File Name: errors.h
	> Author: 
	> Mail:  
	> Created Time: 2016年07月11日 星期一 13时34分39秒
 ************************************************************************/

#ifndef FAC_ERRORS_H
#define FAC_ERRORS_H


enum E_FAC_TEST {
    E_OK                = 0x01,  // success
    E_EXECUTED          = 0x02,  // command is excuting
    E_INVALID_NUMBER    = 0x03,  // invalid number
    E_PARAMETERS        = 0x04,  // invalid parameters
    E_CAN_COMMUNICATE   = 0x05,  // can communicate error
    E_DISCONNECTED      = 0x06,  // device is not connected
    E_NOT_FACTORY_MODE  = 0x07,  // not in factory mode
    E_FILE_NOT_FOUND    = 0x08,  // file not find

    E_OTHERS            = 0xFE,  // unknow failed reason
    E_FAILED            = 0xFF   // command excute failed
};




#define E_SET(e) (errs = e)
#define E_RESET() E_SET(E_OK)
extern volatile uint8_t errs; 

#endif /* FAC_ERRORS_H */
