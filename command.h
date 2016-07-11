/*************************************************************************
	> File Name: command.h
	> Author: 
	> Mail:  
	> Created Time: 2016年07月07日 星期四 13时54分57秒
 ************************************************************************/

#ifndef COMMAND_H
#define COMMAND_H

enum COMMAND_FACTORYTEST 
{
    FAC_ID_ENTER_FACTORY_MODE       = 0x01,
    FAC_ID_EXIT_FACTORY_MODE        = 0x02,
    FAC_ID_RESET_FACTORY_DEFAULT    = 0x03,

    FAC_ID_SET_ROBOT_SN             = 0x04,
    FAC_ID_GET_ROBOT_SN             = 0x05,
    
    FAC_ID_SET_WIFI_MAC             = 0x06,
    FAC_ID_GET_WIFI_MAC             = 0x07,
    FAC_ID_GET_WIFI_SIGNAL_STRENGTH = 0x08,
    FAC_ID_WIFI_SET_AP_ASSOCIATE    = 0x09,
    FAC_ID_WIFI_GET_AP_ASSOCIATE    = 0x0A,

    FAC_ID_GET_IP_ADDR              = 0x0B,
    FAC_ID_PING_ID_ADDR             = 0x0C,

    FAC_ID_BT_SET_ADDR              = 0x0D,
    FAC_ID_BT_GET_ADDR              = 0x0E,
    FAC_ID_BT_GET_SIGNAL_STRENGTH   = 0x0F,
    FAC_ID_BT_PAIRING               = 0x10,

    FAC_ID_ENTER_BURNING_MODE       = 0x11,
    FAC_ID_EXIT_BURNING_MODE        = 0x12,

    //FAC_ID_GET_FW_VERSION           = 0x13,
    FAC_ID_GET_FW_VERSION = 0x14,
    //FAC_ID_MIC_OPEN                 = 0x14,
    FAC_ID_MIC_START_RECORD         = 0x15,
    FAC_ID_MIC_STOP_RECORD          = 0x16,
    FAC_ID_MIC_PLAY_RECORD_FILE     = 0x17,
    FAC_ID_MIC_CLOSE                = 0x18,

    FAC_ID_KEY_STATUS               = 0x19,

    FAC_ID_RING_PLAY                = 0x1A,

    FAC_ID_LED_ON                   = 0x1B,
    FAC_ID_LED_OFF                  = 0x1C,

    FAC_ID_IR_SENSOR                = 0x1D,
    FAC_ID_ULTRASONIC_SENDOR        = 0x1E,

    FAC_ID_CHARGER_STATUS           = 0x1F,
    FAC_ID_USB_PLUGIN_STATUS        = 0x20,
    FAC_ID_CAMERA_OPEN              = 0x21,
    FAC_ID_CAMERA_TAKE_PICTURE      = 0x22,
    FAC_ID_CAMERA_VIEW_PICTURE      = 0x23,
    FAC_ID_CAMERA_CLOSE             = 0x24,
    FAC_ID_LCD_DISPLAY              = 0x25,
    FAC_ID_TOUCH_PANEL_TEST         = 0x26,
    FAC_ID_BATTERY_LEVEL            = 0x27,
    FAC_ID_TF_STATUS                = 0x28,
    FAC_ID_CAN_BUS_TEST             = 0x29,
    FAC_ID_FACIAL_EXPRESSION        = 0x2A,
    FAC_ID_MOTOR_MOTION             = 0x2B,
    FAC_ID_SPEAK_PATTERN            = 0x2C,
    FAC_ID_TOUCH_SENSOR             = 0x2D,

};

#endif /* COMMAND_H */
