/*************************************************************************
	> File Name: test_sensors.cpp
	> Author: 
	> Mail:  
	> Created Time: 2016年07月25日 星期一 10时41分56秒
 ************************************************************************/

#include <unistd.h>
#include <android/sensor.h>
#include <gui/Sensor.h>
#include <android/looper.h>
#include <pthread.h>

#include "fac_util.h"
#include "fac_errors.h"
#include "test_sensors.h"

using namespace android;

/* sensor handle sync with /hardware/rk30/sensor/st/nusensors.h */
#define HANDLE_IRDA_BOTTOM_FRONT_LEFT           6
#define HANDLE_IRDA_BOTTOM_FRONT_RIGHT          7
#define HANDLE_IRDA_BOTTOM_BACK_CENTER          10

#define HANDLE_ULTRASONIC_FRONT_BELLY           17
#define HANDLE_ULTRASONIC_BACK_WAIST            18
#define HANDLE_ULTRASONIC_LEFT_ANKLE_OUTSIDE    19
#define HANDLE_ULTRASONIC_RIGHT_ANKLE_OUTSIDE   20
#define HANDLE_ULTRASONIC_CHASSIS               21


#define GYROSCOPE_SAMPLE_RATE   200
#define LOOPER_ID_SENSOR        2
#define LOOPER_TIMEOUT          1000/GYROSCOPE_SAMPLE_RATE

#define SENSOR_TYPE_MAX         SENSOR_TYPE_ULTRASONIC_DISTANCE


static ASensorManager *g_manager = NULL;
static ASensorRef g_sensor = NULL;
static ASensorEventQueue *g_queue = NULL;

static int sensor_type = -1;
static int sensor_id   = -1;

static pthread_t g_tid = 0;
static int g_sensor_num = 0;
static int g_init_flag = 0;
static volatile float g_distance = 0.0f;

static struct SENSORS_MAP {
    int id;
    int handle;
} ir_sensor_map[] = {
    {IR_SENSOR_FRONT_LEFT,     HANDLE_IRDA_BOTTOM_FRONT_LEFT},
    {IR_SENSOR_FRONT_RIGHT,    HANDLE_IRDA_BOTTOM_FRONT_RIGHT}, 
    {IR_SENSOR_BACK_CENTER,    HANDLE_IRDA_BOTTOM_BACK_CENTER},
}, us_sensor_map[] = {
    {US_SENSOR_FRONT_BELLY,   HANDLE_ULTRASONIC_FRONT_BELLY},
    {US_SENSOR_BACK_WAIST,    HANDLE_ULTRASONIC_BACK_WAIST},
    {US_SENSOR_LEFT_CHASSIS,  HANDLE_ULTRASONIC_LEFT_ANKLE_OUTSIDE},
    {US_SENSOR_RIGHT_CHASSIS, HANDLE_ULTRASONIC_RIGHT_ANKLE_OUTSIDE},
    {US_SENSOR_UP_CHASSIS,    HANDLE_ULTRASONIC_CHASSIS},
};

int get_ir_sensor_handle(int id)
{
    uint32_t i;
    for (i = 0; i < NELEM(ir_sensor_map); i++) {
        if (ir_sensor_map[i].id == id)
            return ir_sensor_map[i].handle;
    }

    return -1;
}

int get_us_sensor_handle(int id)
{
    uint32_t i;
    for (i = 0; i < NELEM(us_sensor_map); i++) {
        if (us_sensor_map[i].id == id)
            return us_sensor_map[i].handle;
    }

    return -1;
}

static void sensor_handle_event(ASensorEvent *event, int count)
{
    int i;
    for (i = 0; i < count; i++)
    {
        switch (event[i].type)
        {
            case SENSOR_TYPE_IR_DISTANCE:
                g_distance = event[i].data[0];
                LOGD("IR_DISTANCE sensor: %f", g_distance);
                break;
            case SENSOR_TYPE_ULTRASONIC_DISTANCE:
                g_distance = event[i].data[0];
                LOGD("ULTRASONIC_DISTANCE sensor: %f", g_distance);
                break;

            default:
                LOGE("UnHandle sensor data type:%d", event[i].type);
                break;
        } 
    }
}

static void *sensor_poll_thread(void *arg)
{
    ALooper *looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    g_queue = ASensorManager_createEventQueue(g_manager, looper, LOOPER_ID_SENSOR, NULL, NULL);

    while (g_init_flag)
    {
        int ident;
        int events;
        struct android_poll_source* source;

        while((ident = ALooper_pollAll(LOOPER_TIMEOUT, NULL, &events, (void**)&source)) >= 0)
        {
            if(ident == LOOPER_ID_SENSOR)
            {
                int num_event;
                ASensorEvent event[SENSOR_TYPE_MAX];
                while((num_event = ASensorEventQueue_getEvents(g_queue, event, g_sensor_num)) > 0)
                {
                    sensor_handle_event(event, num_event);
                }
            }
        }
    }

    ASensorManager_destroyEventQueue(g_manager, g_queue);
    g_queue = NULL;
    pthread_exit(NULL);
    return NULL;
}

static int init_sensor_grab(int32_t handle)
{
    if (g_init_flag > 0)
        return 0;

    int sensor_num;
    ASensorList sensor_list;
    Sensor *c_se;

    g_manager = ASensorManager_getInstance();
    g_sensor_num = ASensorManager_getSensorList(g_manager, &sensor_list);

    g_init_flag = 1;
    pthread_create(&g_tid, NULL, sensor_poll_thread, NULL);

    for (int i = 0; i < g_sensor_num; i++)
    {
        c_se = (Sensor*)(sensor_list[i]);
        if (c_se->getHandle() == handle) 
        {
            g_sensor = sensor_list[i];
            return 0;
        }
    }

    return -1;
}

static void uninit_sensor_grab()
{
    if (g_init_flag > 0)
        g_init_flag = 0;

    pthread_join(g_tid, NULL);

    g_queue = NULL;
    g_manager = NULL;
    g_distance = 0;
}


int enable_sensor(int type, int id)
{
    int handle, ret;
    E_RESET();

    if (sensor_type == type && sensor_id == id)
        return 0;

    if (sensor_id != -1)
        disable_sensor();

    if (type == SENSOR_INFRARED)
        handle = get_ir_sensor_handle(id);
    else
        handle = get_us_sensor_handle(id);

    if (handle < 0)
    {
        E_SET(E_INVALID_NUMBER);
        return -1;
    }

    ret = init_sensor_grab(handle);
    if (ret < 0)
    {
        E_SET(E_FAILED);
        return -1;
    }

    while (!g_queue)
        usleep(20000);

    ret = ASensorEventQueue_enableSensor(g_queue, g_sensor);
    if (ret < 0)
    {
        E_SET(E_FAILED);

        uninit_sensor_grab();
        return -1;
    }

    sensor_type = type;
    sensor_id = id;

    return 0;
}

void disable_sensor()
{
    if (sensor_id != -1)
    {
        ASensorEventQueue_disableSensor(g_queue, g_sensor);
        g_sensor = NULL;

        uninit_sensor_grab();

        sensor_id = -1;
    }
}

int get_distance()
{
    return (int)g_distance;
}

