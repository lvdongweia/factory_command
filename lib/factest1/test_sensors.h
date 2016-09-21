/*************************************************************************
	> File Name: test_sensors.h
	> Author: 
	> Mail:  
	> Created Time: 2016年07月25日 星期一 10时41分16秒
 ************************************************************************/

#ifndef TEST_SENSORS
#define TEST_SENSORS

enum SENSOR_TYPE {
    SENSOR_INFRARED   = 0,
    SENSOR_ULTRASONIC = 1,
};

enum IR_SENSORS_ID {
    IR_SENSOR_FRONT_LEFT        = 1,
    IR_SENSOR_FRONT_RIGHT       = 2,
    IR_SENSOR_BACK_CENTER       = 3,
    IR_SENSOR_MAX
};

enum ULTRASONIC_SENSORS_ID {
    US_SENSOR_FRONT_BELLY       = 1,
    US_SENSOR_BACK_WAIST        = 2,
    US_SENSOR_LEFT_CHASSIS      = 3,
    US_SENSOR_RIGHT_CHASSIS     = 4,
    US_SENSOR_UP_CHASSIS        = 5,
    US_SENSOR_MAX
};


int enable_sensor(int type, int id);

void disable_sensor();

int get_distance();

#endif /* TEST_SENSORS */
