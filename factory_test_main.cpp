/*************************************************************************
	> File Name: factory_test_main.c
	> Author: 
	> Mail:  
	> Created Time: 2016年06月29日 星期三 16时19分42秒
 ************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


#include "command.h"
#include "fac_errors.h"
#include "fac_util.h"
#include "transport.h"

#include "test_robot_info.h"
#include "test_sensors.h"
#include "test_battery.h"
#include "test_mic.h"
#include "RobotHwInfo.h"
#include "wifi/RWifiManager.h"
#include "robot_fac.h"
#include "bluetooth/RBluetoothCommon.h"
#include "bluetooth/RBluetoothManager.h"
#include "RStorageManager.h"
#include "camera/ActivityTestService.h"
#include "camera/pattern.h"


class TouchCallback;

const static char BURNING_SCRIPT_START[] = "/system/bin/motor_main.sh";
const static char BURNING_SCRIPT_STOP[] = "echo 0 > /data/motor_break";
const static char BURNING_SCRIPT_CONFIG[] = "/data/motor_break";

static bool g_IsFactoryMode = false;
static MOTOR_ID g_MOTOR_MOVING = INVALID_MOTOR_ID;

static sp<ActivityTestService> mService = NULL;

static bool mIsTouchTest = false;
static time_t mTouchStart;
static time_t mTouchEnd;

static int getMotionError(int err)
{
    switch(err)
    {
        case FAC_EXEC_NO_ERROR:
            return E_OK;
        case FAC_EXEC_FAILED:
            return E_FAILED;    
        case FAC_INVALID_ANGLE:
        case FAC_INVALID_TIME:
        case FAC_INVALID_MOTOR_ID:
        case FAC_INVALID_MOTOR_DIRECTION:
        case FAC_INVALID_RB_TYPE:
        case FAC_INVALID_MOTION_ID:
        case FAC_INVALID_POWER_MODE:
            return E_PARAMETERS;
    }

    return E_OTHERS;
}

static void get_robot_sn()
{
    uint8_t sn[PAYLOAD_SIZE] = {0};
    int len;

    RobotHwInfo rbtInfo;
    const char *c_sn =  rbtInfo.getSN();

    LOGD("Robot SN: %s", c_sn);
    len = str2Hex(sn, c_sn);
    responseEvent(MSG_TYPE_NACK, FAC_ID_GET_ROBOT_SN, sn, len);
}

static void get_wifi_addr()
{
    struct MacAddrStore wifi;
    int ret;

    RobotHwInfo rbtInfo;
    ret = rbtInfo.getEthMacAddr(wifi);
    if (ret < 0)
    {
        responseEvent(MSG_TYPE_NACK, FAC_ID_GET_WIFI_MAC, E_FAILED);
        return;
    }

    printMacAddr(wifi.addr, sizeof(wifi.addr));
    responseEvent(MSG_TYPE_NACK, FAC_ID_GET_WIFI_MAC, wifi.addr, sizeof(wifi.addr));
}

static void set_wifi_ap(uint8_t *data, int len)
{
    char ssid[PAYLOAD_SIZE+1] = {0};
    int count = 6;

    if (data == NULL)
    {
        responseEvent(MSG_TYPE_NACK, FAC_ID_WIFI_SET_AP_ASSOCIATE, E_PARAMETERS);
        return;
    }

    // get ssid
    Hex2Str(ssid, data, len); 

    RWifiManager *wifi = RWifiManager::getInstance();
    if (!wifi->isWifiEnabled())
    {
        LOGD("Opening wifi...");
        if (wifi->openWifi())
        {
            responseEvent(MSG_TYPE_NACK, FAC_ID_WIFI_SET_AP_ASSOCIATE, E_FAILED);
            return;
        }
        LOGD("Wifi open success");
    }

    LOGD("Connecting SSID: %s", ssid);
#if 1 
    int ret = wifi->connectToWifi(ssid);
#else
    sprintf(ssid, "TP-LINK_4E66");
    const char password[] = "avatarmindtest123";
    int ret = wifi->connectToWifi(ssid, password, RWifiManager::PASSWD_WPA_PSK);
#endif   
    if (ret < 0)
        ret = E_FAILED;
    else 
    {
        while (count > 0)
        {
            if (wifi->isConnected()) break;
            sleep(1);
            count--;
        }

        if (count > 0)
            ret = E_OK;
        else
            ret = E_FAILED;
    }

    LOGD("Connected end.");
    responseEvent(MSG_TYPE_NACK, FAC_ID_WIFI_SET_AP_ASSOCIATE, ret);
    //wifi->releaseInstance();
}

static void get_wifi_ap()
{
    uint8_t ap[PAYLOAD_SIZE] = {0};
    RWifiManager *wifi = RWifiManager::getInstance();

    if (!wifi->isConnected())
    {
        responseEvent(MSG_TYPE_NACK, FAC_ID_WIFI_GET_AP_ASSOCIATE, E_DISCONNECTED);
        return;
    }

    const char* ssid = wifi->getSSID();
    LOGD("Wifi SSID: %s", ssid);

    int len = str2Hex(ap, ssid);
    responseEvent(MSG_TYPE_NACK, FAC_ID_WIFI_GET_AP_ASSOCIATE, ap, len);
    //wifi->releaseInstance();
}
 
static void get_wifi_signal()
{
    RWifiManager *wifi = RWifiManager::getInstance();
    if (!wifi->isConnected())
    {
        responseEvent(MSG_TYPE_NACK, FAC_ID_GET_WIFI_SIGNAL_STRENGTH, E_DISCONNECTED);
        return;
    }

    int val = wifi->getWifiSignal();
    // conversion (-200,256) to (-120,-30)
    int r_val = (-120 + 90*(val-(-200)) / 456);
    LOGD("Wifi signal: %d, (%d)", val, r_val);

    uint8_t signal = 0x88 + (r_val + 120); 
    responseEvent(MSG_TYPE_NACK, FAC_ID_GET_WIFI_SIGNAL_STRENGTH, &signal, 1);
    //wifi->releaseInstance();
}

static void get_ip_addr()
{
    uint8_t addr[PAYLOAD_SIZE] = {0};
    RWifiManager *wifi = RWifiManager::getInstance();

    if (!wifi->isConnected())
    {
        responseEvent(MSG_TYPE_NACK, FAC_ID_GET_IP_ADDR, E_DISCONNECTED);
        return;
    }

    const char* ip = wifi->getIpAddr();
    LOGD("Wifi ip: %s", ip);

    int len = str2Hex(addr, ip);
    responseEvent(MSG_TYPE_NACK, FAC_ID_GET_IP_ADDR, addr, len);
    //wifi->releaseInstance();
}

static void ping_ip_addr(uint8_t *data, int len)
{
    uint8_t count = 0;
    if (data == NULL || len != 1)
    {
        responseEvent(MSG_TYPE_NACK, FAC_ID_PING_IP_ADDR, E_PARAMETERS);
        return;
    }

    int packets = data[0];
    RWifiManager *wifi = RWifiManager::getInstance();
    if (!wifi->isConnected())
    {
        responseEvent(MSG_TYPE_NACK, FAC_ID_PING_IP_ADDR, E_DISCONNECTED);
        return;
    }

    const char *ip = wifi->getIpAddr();
    char n_ip[16] = {0};
    memcpy(n_ip, ip, strlen(ip));
    char *p = strrchr(n_ip, '.');
    *(++p) = '1';
    *(++p) = '\0';
    LOGD("Start ping %s (%d packets)...", n_ip, packets);
    int ret = wifi->ping(n_ip, packets);
    if (ret < 0)
    {
        responseEvent(MSG_TYPE_NACK, FAC_ID_PING_IP_ADDR, E_FAILED);
    }
    else
    {
        count = (uint8_t)ret;
        responseEvent(MSG_TYPE_NACK, FAC_ID_PING_IP_ADDR, &count, 1);
    }
    //wifi->releaseInstance();

    LOGD("Ping end. (Rev:%d packets)", count);
}

static void get_bt_addr()
{
    struct MacAddrStore bt;
    int ret;

    RobotHwInfo rbtInfo;
    ret = rbtInfo.getBtMacAddr(bt);
    if (ret < 0)
    {
        responseEvent(MSG_TYPE_NACK, FAC_ID_BT_GET_ADDR, E_FAILED);
        return;
    }

    printMacAddr(bt.addr, sizeof(bt.addr));
    responseEvent(MSG_TYPE_NACK, FAC_ID_BT_GET_ADDR, bt.addr, sizeof(bt.addr));
}


static void bt_pairing(uint8_t *data, int len)
{
    if (data == NULL || len != 6)
    {
        responseEvent(MSG_TYPE_ACK, FAC_ID_BT_PAIRING, E_PARAMETERS);
        return;
    }

    int wait_time, cancel_time;
    int ret = E_OK;
    RBluetoothManager *bt = RBluetoothManager::instance();
    struct RBluetoothAddr bt_addr;
    memcpy(&bt_addr.addr, data, 6);
    printMacAddr(bt_addr.addr, sizeof(struct RBluetoothAddr));

    if (!bt->isOpened()) 
    {
        LOGD("BT: Opening bt...");
        if (!bt->openBt())
        {
            LOGD("BT: open failed");
            ret = E_DISCONNECTED;
            goto FAILED;
        }

        wait_time = 10;
        while (wait_time-- > 0 && !bt->isOpened())
        {
            sleep(1);
        }

        if (wait_time <= 0)
        {
            ret = E_DISCONNECTED;
            LOGD("BT: Open BT timeout");
            goto FAILED;
        }
    }
    LOGD("BT: Bt is opened");

    LOGD("BT: start discovery");
    bt->startDiscovery();
    wait_time = 20;
    while(wait_time-- > 0 && !bt->isFound(bt_addr))
    {
        sleep(1);
    }
    bt->cancelDiscovery();
    cancel_time = 10;
    while(cancel_time-- > 0 && bt->isDiscovering())
    {
        sleep(1);
    }

    if (wait_time <= 0)
    {
        LOGD("BT: found failed");
        goto FAILED;
    }
    LOGD("BT: Find device.");

    LOGD("BT: Pairing...");
    if (!bt->pairing(bt_addr))
    {
        LOGD("BT: Pair failed");
        ret = E_FAILED;
        goto FAILED;
    }

    wait_time = 15;
    while(wait_time-- > 0 && !bt->isBond())
    {
        sleep(1);
    }

    if (wait_time <= 0)
    {
        LOGD("BT: Pair timeout");
        ret = E_FAILED;
        goto FAILED;
    }
    LOGD("BT: Pair success");

FAILED:
    responseEvent(MSG_TYPE_ACK, FAC_ID_BT_PAIRING, ret);
    //bt->release();
    LOGD("BT: Pair end.");
}

static void get_bt_signal()
{
    RBluetoothManager *bt = RBluetoothManager::instance();
    
    if (!bt->isBond())
    {
        responseEvent(MSG_TYPE_ACK, FAC_ID_BT_GET_SIGNAL_STRENGTH, E_DISCONNECTED);
        return;
    }

     int val = bt->getSignalStrength();
    // conversion (-200,256) to (-120,-30)
    int r_val = (-120 + 90*(val-(-200)) / 456);
    LOGD("BT signal: %d, (%d)", val, r_val);

    uint8_t signal = 0x88 + (r_val + 120); 
    responseEvent(MSG_TYPE_ACK, FAC_ID_BT_GET_SIGNAL_STRENGTH, &signal, 1);
    //bt->release();
}

static void get_robot_cloud_sn()
{
    uint8_t sn[PAYLOAD_SIZE] = {0};
    int ret;

    ret = get_serial_number(sn);
    if (ret <= 0) {
        responseEvent(MSG_TYPE_NACK, FAC_ID_GET_ROBOT_CHECK_CLOUD_SN, E_FAILED);
    }
    else {
        responseEvent(MSG_TYPE_NACK, FAC_ID_GET_ROBOT_CHECK_CLOUD_SN, E_OK);
    }
}

static void enter_burning_mode()
{
    int ret;
    E_RESET();
 
    FILE *f = fopen(BURNING_SCRIPT_CONFIG, "r");
    if (f)
    {
        int execute = fgetc(f);
        if (execute == '1')
        {
            responseEvent(MSG_TYPE_ACK, FAC_ID_ENTER_BURNING_MODE, E_EXECUTED);
            return;
        }
    }

    ret = system(BURNING_SCRIPT_START);
    if (ret == -1) 
        E_SET(E_FAILED);

    responseEvent(MSG_TYPE_ACK, FAC_ID_ENTER_BURNING_MODE, errs);
}

static void exit_burning_mode()
{
    int ret;
    E_RESET();

    ret = system(BURNING_SCRIPT_STOP);
    if (ret == -1)
        E_SET(E_FAILED);

    responseEvent(MSG_TYPE_ACK, FAC_ID_EXIT_BURNING_MODE, errs);
}

static void get_fw_version(uint8_t *data, int len)
{
    uint8_t hexVer[PAYLOAD_SIZE] = {0};
    int ret, id;

    if (data == NULL || len != 1)
    {
        responseEvent(MSG_TYPE_NACK, FAC_ID_GET_FW_VERSION, E_PARAMETERS);
        return ;
    }

    id = data[0];
    ret = getfw_version(id, hexVer + 1);
    if (ret < 0)
    { 
        responseEvent(MSG_TYPE_NACK, FAC_ID_GET_FW_VERSION, errs);
    }
    else
    {
        hexVer[0] = id;
        responseEvent(MSG_TYPE_NACK, FAC_ID_GET_FW_VERSION, hexVer, ret+1);
    }
}

static void ring_play(uint8_t *data, int len)
{
    if (data == NULL || len != 2)
    {
        responseEvent(MSG_TYPE_ACK, FAC_ID_RING_PLAY_START, E_PARAMETERS);
        return;
    }

    PLAY_VOLUME ch = (PLAY_VOLUME)data[0];
    int level = (int)data[1];

    if (ch < L_ONLY || ch > STEREO || level < 0 || level > 100)
    {
        responseEvent(MSG_TYPE_ACK, FAC_ID_RING_PLAY_START, E_PARAMETERS);
        return;
    }

    level = (level * 15) / 100;

    ring_play_start(ch, level);
    responseEvent(MSG_TYPE_ACK, FAC_ID_RING_PLAY_START, errs);
}

static void get_sensor_distance(uint8_t cmd, uint8_t *data, int len)
{
    uint8_t hexSensor[PAYLOAD_SIZE] = {0};
    int ret, distance, type, id;

    if (data == NULL || len != 1)
    {
        responseEvent(MSG_TYPE_ACK, cmd, E_PARAMETERS);
        return ;
    }

    if (cmd == FAC_ID_IR_SENSOR)
        type = SENSOR_INFRARED;
    else
        type = SENSOR_ULTRASONIC;

    id = data[0];
    ret = enable_sensor(type, id);
    if (ret < 0)
    {
        responseEvent(MSG_TYPE_ACK, cmd, errs);
        return;
    }

    sleep(1);
    distance = get_distance();

    disable_sensor();

    hexSensor[0] = id;
    hexSensor[1] = (uint8_t)distance;
    responseEvent(MSG_TYPE_ACK, cmd, hexSensor, 2);
}

static void get_charger_status()
{
    uint8_t hexStatus[PAYLOAD_SIZE] = {0};
    int ret = get_battery_status();
    if (ret < 0)
    {
        responseEvent(MSG_TYPE_NACK, FAC_ID_CHARGER_STATUS, errs);
        return;
    }

    hexStatus[0] = ret;
    responseEvent(MSG_TYPE_NACK, FAC_ID_CHARGER_STATUS, hexStatus, 1);
}

static void get_battery_level()
{
    uint8_t hexLevel[PAYLOAD_SIZE] = {0};
    int ret = get_battery_capacity();
    if (ret < 0)
    {
        responseEvent(MSG_TYPE_NACK, FAC_ID_BATTERY_LEVEL, errs);
        return;
    }

    hexLevel[0] = ret;
    responseEvent(MSG_TYPE_NACK, FAC_ID_BATTERY_LEVEL, hexLevel, 1);
}

static void get_tf_status()
{
    RStorageManager *tf = RStorageManager::instance();

    uint8_t status;
    const uint8_t DETECT   = 0x01;
    const uint8_t UNDETECT = 0x02;
    if (tf->isTFCardMounted())
    {
        LOGD("TF Status: mounted");
        status = DETECT;
    }
    else
    {
        LOGD("TF Status: unmounted");
        status = UNDETECT;
    }

    responseEvent(MSG_TYPE_ACK, FAC_ID_TF_STATUS, &status, 1);
    tf->release();
}

static void test_can_bus()
{
    int ret, count = 0;
    uint8_t hexData[PAYLOAD_SIZE] = {0};
    uint8_t modules[CAN_MODULE_SUM] = {CAN_MODULE_RM,
        CAN_MODULE_RC, CAN_MODULE_RP, CAN_MODULE_RF, 
        CAN_MODULE_RBR, CAN_MODULE_RBL, CAN_MODULE_RBB};


    for (int i = 0; i < NELEM(modules); i++)
    {
        ret = test_can_module(modules[i]);
        if (ret < 0)
        {
            hexData[++count] = modules[i];
        }
    }

    if (count == 0)
    {
        responseEvent(MSG_TYPE_ACK, FAC_ID_CAN_BUS_TEST, E_OK);
    }
    else
    {
        hexData[0] = E_FAILED;
        responseEvent(MSG_TYPE_ACK, FAC_ID_CAN_BUS_TEST, hexData, count+1);
    }
}

static void test_facial_expression(uint8_t *data, int len)
{
    int exp_id;

    if (data == NULL || len != 1)
    {
        responseEvent(MSG_TYPE_ACK, FAC_ID_FACIAL_EXPRESSION, E_PARAMETERS);
        return;
    }
    
    exp_id = data[0];
    set_expression(exp_id);
    responseEvent(MSG_TYPE_ACK, FAC_ID_FACIAL_EXPRESSION, errs);
}

static void set_motor_motion(uint8_t *data, int len)
{
    uint8_t motion[PAYLOAD_SIZE] = {0};
    if (data == NULL || len != 1)
    {
        responseEvent(MSG_TYPE_ACK, FAC_ID_MOTOR_MOTION, E_PARAMETERS);
        return;
    }

    motion[0] = data[0];
    MOTION_ID id = (MOTION_ID)data[0];
    int ret = fac_motor_motion(id);

    motion[1] = getMotionError(ret);
    responseEvent(MSG_TYPE_ACK, FAC_ID_MOTOR_MOTION, motion, 2);
}

static void set_motor_calibration()
{
    fac_motor_calibration2(RB_RIGHT);
    fac_motor_calibration2(RB_LEFT);
    fac_motor_calibration2(RB_BODY);

    responseEvent(MSG_TYPE_ACK, FAC_ID_MOTOR_CALIBRATION, E_OK);
}

static void motor_move(uint8_t *data, int len)
{
    if (data == NULL || len != 4)
    {
        responseEvent(MSG_TYPE_ACK, FAC_ID_MOTOR_MOVE, E_PARAMETERS);
        return;
    }

    MOTOR_ID mid = (MOTOR_ID)data[0];
    MOTOR_DIRECTION direction = (MOTOR_DIRECTION)data[1];
    uint32_t angle = data[2];
    uint32_t time = data[3];

    int ret = fac_motor_move(mid, direction, angle, time);
    if (ret == FAC_EXEC_NO_ERROR)
    {
        g_MOTOR_MOVING = mid;
    }
    ret = getMotionError(ret);

    uint8_t resp = (uint8_t)ret;
    responseEvent(MSG_TYPE_ACK, FAC_ID_MOTOR_MOVE, &resp, 1);
}

static void motor_stop()
{
    int ret = E_OK;

    if (g_MOTOR_MOVING != INVALID_MOTOR_ID)
    {
        ret = fac_motor_stop(g_MOTOR_MOVING);

        ret = getMotionError(ret);
    }

    uint8_t resp = (uint8_t) ret;
    responseEvent(MSG_TYPE_ACK, FAC_ID_MOTOR_STOP, &resp, 1);
}

static void set_robot_gender(uint8_t *data, int len)
{
    uint8_t sex;

    if (data == NULL || len != 1)
    {
        responseEvent(MSG_TYPE_NACK, FAC_ID_SET_ROBOT_GENDER, E_PARAMETERS);
        return;
    }

    sex = data[0];
    set_gender(sex);
    responseEvent(MSG_TYPE_NACK, FAC_ID_SET_ROBOT_GENDER, errs);
}

static void get_robot_gender()
{
    int ret;
    uint8_t sex;

    ret = get_gender(sex);
    if (ret < 0)
        responseEvent(MSG_TYPE_NACK, FAC_ID_GET_ROBOT_GENDER, errs);
    else
        responseEvent(MSG_TYPE_NACK, FAC_ID_GET_ROBOT_GENDER, &sex, 1);
}

static void set_power_mode(uint8_t *data, int len)
{
    if (data == NULL || len != 1)
    {
        responseEvent(MSG_TYPE_NACK, FAC_ID_SET_POWER_MODE, E_PARAMETERS);
        return;
    }

    POWER_MODE mode = (POWER_MODE)data[0];
    int ret = fac_set_power_mode(mode);
    ret = getMotionError(ret);

    uint8_t resp = (uint8_t)ret;
    responseEvent(MSG_TYPE_NACK, FAC_ID_SET_POWER_MODE, &resp, 1);
}

static void camera_open()
{
    int ret = E_OK;
    int count = 3;
    if (mService == NULL)
    {
        LOGD("Camera: Service is not start");
        ret = E_DISCONNECTED;
        goto FAILED;
    }

    // open camera
    if (mService->startCameraActivity() < 0) 
    {
        LOGD("Camera: activity start failed");
        ret = E_FAILED;
        goto FAILED;
    }

    while(count-- > 0 && !mService->isCameraActivity())
    {
        sleep(1);
    }
    if (count <= 0)
    {
        LOGD("Camera: camera activity start timeout");
        ret = E_FAILED;
        goto FAILED;
    }

    if (mService->openCamera() < 0)
    {
        LOGD("Camera: open camera failed");
        ret = E_FAILED;
        goto FAILED;
    }
FAILED:
    responseEvent(MSG_TYPE_ACK, FAC_ID_CAMERA_OPEN, ret);
}

static void camera_preview()
{
    int ret = E_OK;
    if (mService == NULL)
    {
        LOGD("Camera: Service is not start");
        ret = E_DISCONNECTED;
        goto FAILED;
    }

    if (!mService->isCameraActivity())
    {
        LOGD("Camera: activity is not start");
        ret = E_DISCONNECTED;
        goto FAILED;
    }

    if (mService->preview() < 0) 
    {
        LOGD("Camera: preview failed");
        ret = E_FAILED;
        goto FAILED;
    }

FAILED:
    responseEvent(MSG_TYPE_ACK, FAC_ID_CAMERA_PREVIEW, ret);

}

static void camera_take_picture()
{
    int ret = E_OK;
    if (mService == NULL)
    {
        LOGD("Camera: Service is not start");
        ret = E_DISCONNECTED;
        goto FAILED;
    }

    if (!mService->isCameraActivity())
    {
        LOGD("Camera: activity is not start");
        ret = E_DISCONNECTED;
        goto FAILED;
    }

    if (mService->takePicture(1, String16("capture1.jpg")) < 0)
    {
        LOGD("Camera: take picture failed");
        ret = E_FAILED;
        goto FAILED;
    }
FAILED:
    responseEvent(MSG_TYPE_ACK, FAC_ID_CAMERA_TAKE_PICTURE, ret);
}

static void camera_view_picture()
{
    int ret = E_OK;
    if (mService == NULL)
    {
        LOGD("Camera: Service is not start");
        ret = E_DISCONNECTED;
        goto FAILED;
    } 

    if (!mService->isCameraActivity())
    {
        LOGD("Camera: activity is not start");
        ret = E_DISCONNECTED;
        goto FAILED;
    }

    if (mService->viewPicture(1, String16("capture1.jpg")) < 0)
    {
        LOGD("Camera: view picture failed");
        ret = E_FAILED;
        goto FAILED;
    }
FAILED:
    responseEvent(MSG_TYPE_ACK, FAC_ID_CAMERA_VIEW_PICTURE, ret);
}

static void camera_close()
{
    int ret = E_OK;
    if (mService == NULL)
    {
        LOGD("Camera: Service is not start");
        ret = E_DISCONNECTED;
        goto FAILED;
    }

    if (mService->isCameraActivity())
    {
        if (mService->closeCamera() < 0)
        {
            LOGD("Camera: camera close failed");
        }

        mService->stopCameraActivity();
    }

FAILED:
    responseEvent(MSG_TYPE_ACK, FAC_ID_CAMERA_CLOSE, ret);
}


static void lcd_display(uint8_t *data, int len)
{
    int ret = E_OK;
    int count = 3;
    int id;
    if (data == NULL || len != 1)
    {
        ret = E_PARAMETERS;
        goto FAILED;
    }

    if (mService == NULL)
    {
        LOGD("LCD: Service is not start");
        ret = E_DISCONNECTED;
        goto FAILED;
    }

    if (!mService->isLcdActivity())
    {
        if (mService->startLcdActivity() < 0)
        {
            LOGD("LCD: start activity failed");
            ret = E_FAILED;
            goto FAILED;
        }

        while (count-- > 0 && !mService->isLcdActivity())
        {
            sleep(1);
        }
        if (count <= 0)
        {
            LOGD("LCD: start activity timeout");
            ret = E_FAILED;
            goto FAILED;
        }
    }

    id = data[0];
    if (id < RED_PATTERN || id > FRAME_PATTERN)
    {
        ret = E_PARAMETERS;
        goto FAILED;
    }

    LOGD("LCD: display - %d", id);
    if (mService->display(id) < 0)
    {
        LOGD("LCD: set display(%d) failed.", id);
        ret = E_FAILED;
        goto FAILED;
    }

FAILED:
    responseEvent(MSG_TYPE_ACK, FAC_ID_LCD_DISPLAY, ret);
}

class TouchCallback : public ActivityTestCallback
{
public:
    virtual int touch(int num, const int* points)
    {
        if (!mIsTouchTest) return -1;
        mIsTouchTest = false;

        mTouchEnd = time(NULL);
        if ((mTouchEnd - mTouchStart) > 5) 
        {
            LOGD("Touch: Report touch timeout");
            return -1;
        }

        LOGD("Touch: num-%d", num);

        int x, y;
        int len = 0;
        uint8_t pt[PAYLOAD_SIZE] = {0};

        pt[len] = num;
        for (int i = 0; i < num; i++)
        {
            x = *(points + 2*i);
            y = *(points + 2*i + 1);
            pt[++len] = (x & 0xFF00) >> 8;
            pt[++len] = (x & 0x00FF);
            pt[++len] = (y & 0xFF00) >> 8;
            pt[++len] = (y & 0x00FF);

            LOGD("Touch: (%d, %d)", x, y);
        }

        responseEvent(MSG_TYPE_ACK, FAC_ID_TOUCH_PANEL_TEST, pt, len+1);
        return 0;
    }
};


static TouchCallback mTouchCallback;

static void touch_panel_test()
{
    int ret = E_OK;
    int count = 5;

    if (mService == NULL)
    {
        LOGD("TOUCH: Service is not start");
        ret = E_DISCONNECTED;
        goto FAILED;
    }

    if (!mService->isTouchActivity())
    {
        if (mService->startTouchActivity() < 0)
        {
            LOGD("TOUCH: startt activity timeout");
            ret = E_FAILED;
            goto FAILED;
        }

        while (count-- > 0 && !mService->isTouchActivity())
        {
            sleep(1);
        }
        if (count <= 0)
        {
            LOGD("TOUCH: start activity timeout");
            ret = E_FAILED;
            goto FAILED;
        }
    }

    mService->setCallback(&mTouchCallback);
    mIsTouchTest = true;

    mTouchStart = time(NULL);

    LOGD("Touch: please start touch test");
    return;

FAILED:
    responseEvent(MSG_TYPE_ACK, FAC_ID_TOUCH_PANEL_TEST, ret);
}


/* 
**handle all factory command.
*/
void handle_event(uint8_t type, uint8_t cmd, uint8_t *data, uint8_t len)
{
    int ret;
    uint8_t result[PAYLOAD_SIZE] = {0};

    if (g_IsFactoryMode == false && cmd != FAC_ID_ENTER_FACTORY_MODE)
    {
        responseEvent(MSG_TYPE_ACK, cmd, E_NOT_FACTORY_MODE);
        return;
    }

    LOGD("handle event: 0x%02x", cmd);
    switch(cmd)
    {
        case FAC_ID_ENTER_FACTORY_MODE:
            g_IsFactoryMode = true;

            // start app service
            mService = ActivityTestService::instance();
            responseEvent(MSG_TYPE_ACK, cmd, E_OK);
            break;
        case FAC_ID_EXIT_FACTORY_MODE:
            g_IsFactoryMode = false;
            // stop app service
            ActivityTestService::release();

            responseEvent(MSG_TYPE_ACK, cmd, E_OK);
            break;
        case FAC_ID_RESET_FACTORY_DEFAULT:
            responseEvent(MSG_TYPE_ACK, cmd, E_OK);
            break;

        case FAC_ID_SET_ROBOT_SN:
            responseEvent(MSG_TYPE_NACK, cmd, E_FAILED);
            break;
       case FAC_ID_GET_ROBOT_SN:
            get_robot_sn();
            break;

        case FAC_ID_SET_WIFI_MAC:
            responseEvent(MSG_TYPE_NACK, cmd, E_FAILED);
            break;
        case FAC_ID_GET_WIFI_MAC:
            get_wifi_addr();
            break;
        case FAC_ID_GET_WIFI_SIGNAL_STRENGTH:
            get_wifi_signal();
            break;
        case FAC_ID_WIFI_SET_AP_ASSOCIATE:
            set_wifi_ap(data, len);
            break;
        case FAC_ID_WIFI_GET_AP_ASSOCIATE:
            get_wifi_ap();
            break;
        case FAC_ID_GET_IP_ADDR:
            get_ip_addr();
            break;
        case FAC_ID_PING_IP_ADDR:
            ping_ip_addr(data, len);
            break;
        
        case FAC_ID_BT_SET_ADDR:
            responseEvent(MSG_TYPE_NACK, cmd, E_FAILED);
            break;
        case FAC_ID_BT_GET_ADDR:
            get_bt_addr();
            break;
        case FAC_ID_BT_GET_SIGNAL_STRENGTH:
            get_bt_signal();
            break;
        case FAC_ID_BT_PAIRING:
            bt_pairing(data, len);
            break;

        case FAC_ID_ENTER_BURNING_MODE:
            enter_burning_mode();
            break;
        case FAC_ID_EXIT_BURNING_MODE:
            exit_burning_mode();
            break;

        case FAC_ID_GET_FW_VERSION: 
            get_fw_version(data, len);
            break;

        case FAC_ID_MIC_OPEN:
            recorder_setup();
            responseEvent(MSG_TYPE_ACK, cmd, errs);
            break;
        case FAC_ID_MIC_START_RECORD:
            recorder_start();
            responseEvent(MSG_TYPE_ACK, cmd, errs);
            break;
        case FAC_ID_MIC_STOP_RECORD:
            recorder_stop();
            responseEvent(MSG_TYPE_ACK, cmd, errs);
            break;
        case FAC_ID_MIC_PLAY_RECORD_FILE:
            play_record();
            responseEvent(MSG_TYPE_ACK, cmd, errs);
            break;
        case FAC_ID_MIC_CLOSE:
            recorder_release();
            responseEvent(MSG_TYPE_ACK, cmd, errs);
            break;
        case FAC_ID_RING_PLAY_START:
            ring_play(data, len);
            break;
        case FAC_ID_RING_PLAY_STOP:
            ring_play_stop();
            responseEvent(MSG_TYPE_ACK, cmd, errs);
            break;
        case FAC_ID_SPEAK_PATTERN:
            responseEvent(MSG_TYPE_ACK, cmd, E_FAILED);
            break;

        //case FAC_ID_MIC_TEST:
        //    break;

        case FAC_ID_KEY_STATUS:
            responseEvent(MSG_TYPE_ACK, cmd, E_FAILED);
            break;

        //case FAC_ID_LED_ON:
        //    break;
        //case FAC_ID_LED_OFF:
        //    break;

        case FAC_ID_IR_SENSOR:
            get_sensor_distance(FAC_ID_IR_SENSOR , data, len);
            break;
        case FAC_ID_ULTRASONIC_SENSOR:
            get_sensor_distance(FAC_ID_ULTRASONIC_SENSOR, data, len);
            break;

        case FAC_ID_CHARGER_STATUS:
            get_charger_status();
            break;
        case FAC_ID_BATTERY_LEVEL:
            get_battery_level();
            break;
            
        case FAC_ID_USB_PLUGIN_STATUS:
            responseEvent(MSG_TYPE_ACK, cmd, E_FAILED);
            break;

        case FAC_ID_CAMERA_OPEN:
            camera_open();
            break;
        case FAC_ID_CAMERA_TAKE_PICTURE:
            camera_take_picture();
            break;
        case FAC_ID_CAMERA_VIEW_PICTURE:
            camera_view_picture();
            break;
        case FAC_ID_CAMERA_CLOSE:
            camera_close();
            break;
        case FAC_ID_CAMERA_PREVIEW:
            camera_preview();
            break;
        //case FAC_ID_CAMERA_IMG_COMPARE:
        //    break;

        case FAC_ID_LCD_DISPLAY:
            lcd_display(data, len);
            break;

        case FAC_ID_TOUCH_PANEL_TEST:
            touch_panel_test();
            break;
        
        case FAC_ID_TF_STATUS:
            get_tf_status();
            break;

        case FAC_ID_CAN_BUS_TEST:
            test_can_bus();
            break;
        case FAC_ID_FACIAL_EXPRESSION:
            test_facial_expression(data, len);
            break;

        case FAC_ID_MOTOR_MOTION:
            set_motor_motion(data, len);
            break;
        case FAC_ID_MOTOR_CALIBRATION:
            set_motor_calibration();
            break;
        case FAC_ID_MOTOR_MOVE:
            motor_move(data, len);
            break;
        case FAC_ID_MOTOR_STOP:
            motor_stop();
            break;

        //case FAC_ID_TOUCH_SENSOR:
        //    break;

        case FAC_ID_SET_ROBOT_GENDER:
            set_robot_gender(data, len);
            break;
        case FAC_ID_GET_ROBOT_GENDER:
            get_robot_gender();
            break;

        case FAC_ID_SET_POWER_MODE:
            set_power_mode(data, len);
            break;
        case FAC_ID_GET_ROBOT_CHECK_CLOUD_SN:
            get_robot_cloud_sn();
            break;

        default:
            LOGD("unhandle command: 0x%02x", cmd);
            break;
    }
}

static void event_loop()
{
    for (;;)
    {
        getNewEvent(handle_event);
    }
}


int main(int argc, char **argv)
{
    LOGD("Start factory_Test");

    if (createOutputDir() < 0)
        return -1;


    mService = ActivityTestService::instance();

    transport_init();
    event_loop();

    return 0;
}
