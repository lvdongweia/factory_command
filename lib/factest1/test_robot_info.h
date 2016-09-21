/*************************************************************************
	> File Name: fw_version.h
	> Author: 
	> Mail:  
	> Created Time: 2016年07月11日 星期一 16时00分30秒
 ************************************************************************/

#ifndef TEST_ROBOT_INFO_H 
#define TEST_ROBOT_INFO_H

#define FW_VER_ANDROID_OS   0x01
#define FW_VER_AI_CORE_LIB  0x02
#define FW_VER_SUBSYS_RM    0x03
#define FW_VER_SUBSYS_RC    0x04
#define FW_VER_SUBSYS_RP    0x05
#define FW_VER_SUBSYS_RF    0x06
#define FW_VER_SUBSYS_RBR   0x07
#define FW_VER_SUBSYS_RBL   0x08
#define FW_VER_SUBSYS_RBB   0x09

#define VERSION_VALUE_MAX   32
#define SERIAL_NUMBER_MAX   32

#define ROBOT_GENDER_BOY    0x10
#define ROBOT_GENDER_GIRL   0x20

#define CAN_MODULE_RM   0xF0
#define CAN_MODULE_RC   0xF1
#define CAN_MODULE_RP   0xF2
#define CAN_MODULE_RF   0xF3
#define CAN_MODULE_RBR  0xF4
#define CAN_MODULE_RBL  0xF5
#define CAN_MODULE_RBB  0xF6
#define CAN_MODULE_SUM  7

/* getfw_version: return the hex version data length,
** if error return -1.
** version:save the hex version data.
*/
int getfw_version(int id, uint8_t *version);

int get_serial_number(uint8_t *sn);

int set_gender(uint8_t sex);
int get_gender(uint8_t &sex);

int test_can_module(uint8_t m_id);

int set_expression(uint8_t id);

#endif /* TEST_ROBOT_INFO_H */
