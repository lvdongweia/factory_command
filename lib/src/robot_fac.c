/*************************************************************************
FileName: robot_fac.c
Copy Right: Copyright (C), 2014-2016, Avatarmind. Co., Ltd.
System: ROBOT
Module: FACTORY TEST LIB
Author: wangkun
Create Date: 2016-08-17
**************************************************************************/
/*************************************************************************
Editor: wangkun
Version: v1.0 init version
Edit Date: 2016-08-17
**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <errno.h>

#define LOG_TAG "ROBOT_FAC"
#include <cutils/log.h>
#include "robot_fac.h"

unsigned int module_drive[MOTOR_MAX] = {
    MODULE_DRIVE_PER_MOTOR
};

unsigned int module_calib[MOTOR_MAX] = {
    MODULE_CALIB_PER_MOTOR
};

unsigned int module_calib2[RB_MAX] = {
    MODULE_CALIB_PER_RB_TYPE
};

int min_angle[MOTOR_MAX] = {
    MIN_ANGLE_ARRAY_PER_MOTOR
};

int max_angle[MOTOR_MAX] = {
    MAX_ANGLE_ARRAY_PER_MOTOR
};

int fac_motor_move(enum MOTOR_ID id, enum MOTOR_DIRECTION direction,
    unsigned int angle, unsigned int run_time)
{
    char cmd[1024] = {0};
    unsigned char task_id;
    unsigned int flag;
    int s_angle;

    if (id >= MOTOR_MAX || id <= INVALID_MOTOR_ID)
        return FAC_INVALID_MOTOR_ID;

    if (direction != POSITIVE && direction != NEGATIVE)
        return FAC_INVALID_MOTOR_DIRECTION;

    if (direction == POSITIVE)
        s_angle = angle;
    else
        s_angle = -angle;

    if (s_angle < min_angle[id] || s_angle > max_angle[id])
        return FAC_INVALID_ANGLE;

    if (run_time > 255)
        return FAC_INVALID_TIME;

    task_id = time(NULL) & 0xff;
    flag = SLOW_STOP;

    sprintf(cmd, "rsc_cli send_to_can %02x 8008%02x%02x%02x%02x%02x%02x 8",
        module_drive[id], task_id, id, angle & 0xff, (angle >> 8) & 0xff, flag,
        run_time & 0xff);

    system(cmd);

    return FAC_EXEC_NO_ERROR;
}

int fac_motor_stop(enum MOTOR_ID id)
{
    char cmd[1024] = {0};
    unsigned char task_id;

    if (id >= MOTOR_MAX || id <= INVALID_MOTOR_ID)
        return FAC_INVALID_MOTOR_ID;

    task_id = time(NULL) & 0xff;

    sprintf(cmd, "rsc_cli send_to_can %02x 8104%02x%02x 4", module_drive[id],
        task_id, id);

    system(cmd);

    return FAC_EXEC_NO_ERROR;
}

int fac_motor_calibration(enum MOTOR_ID id)
{
    char cmd[1024] = {0};
    unsigned char task_id;

    if (id >= MOTOR_MAX || id <= INVALID_MOTOR_ID)
        return FAC_INVALID_MOTOR_ID;

    task_id = time(NULL) & 0xff;

    sprintf(cmd, "rsc_cli send_to_can %02x F003%02x 3", module_calib[id], id);

    system(cmd);

    return FAC_EXEC_NO_ERROR;
}

int fac_motor_calibration2(enum RB_TYPE type)
{
    char cmd[1024] = {0};
    unsigned char task_id;

    if (type != RB_RIGHT && type != RB_LEFT && type != RB_BODY)
        return FAC_INVALID_RB_TYPE;

    task_id = time(NULL) & 0xff;

    sprintf(cmd, "rsc_cli send_to_can %02x F002 2", module_calib2[type]);

    system(cmd);

    return FAC_EXEC_NO_ERROR;
}

#define SPEED_FAST   3
#define SPEED_MEDIUM 2
#define SPEED_SLOW   1

#define PARTS_ALL        0x00
#define PARTS_ARMS       0x01
#define PARTS_LEFT_ARM   0x02
#define PARTS_RIGHT_ARM  0x03
#define PARTS_WHEELS     0x04
#define PARTS_NECK       0x04
#define PARTS_WAIST      0x05
#define PARTS_TRAJECTORY 0x11

#define HORIZONTAL_OFFSET_LEFT  1
#define HORIZONTAL_OFFSET_RIGHT 0

#define MAZE_ACTION_BEGIN 0x01
#define MAZE_ACTION_END   0x00

#define AVOID_OBSTACLE_ENABLE  0x01
#define AVOID_OBSTACLE_DISABLE 0x00

int fac_motor_motion(enum MOTION_ID id)
{
    char cmd[1024] = {0};
    unsigned char task_id;
    unsigned int speed;
    unsigned int where;
    unsigned int distance;
    unsigned int angle;
    unsigned int action;
    unsigned int direction;

    if (id < ALL_STOP || id >= MOTION_MAX)
        return FAC_INVALID_MOTION_ID;

    task_id = time(NULL) & 0xff;
    speed = SPEED_FAST;

    switch(id)
    {
        case ALL_STOP                :
            where = PARTS_ALL;
            sprintf(cmd, "rsc_cli send_to_can 12 FF05%02x%02x%02x 5",
                task_id, where, speed);
            break;
        case ARMS_STOP               :
            where = PARTS_ARMS;
            sprintf(cmd, "rsc_cli send_to_can 12 FF05%02x%02x%02x 5",
                task_id, where, speed);
            break;
        case LEFT_ARM_STOP           :
            where = PARTS_LEFT_ARM;
            sprintf(cmd, "rsc_cli send_to_can 12 FF05%02x%02x%02x 5",
                task_id, where, speed);
            break;
        case RIGHT_ARM_STOP          :
            where = PARTS_RIGHT_ARM;
            sprintf(cmd, "rsc_cli send_to_can 12 FF05%02x%02x%02x 5",
                task_id, where, speed);
            break;
        case WHEELS_STOP             :
            where = PARTS_RIGHT_ARM;
            sprintf(cmd, "rsc_cli send_to_can 12 FF05%02x%02x%02x 5",
                task_id, where, speed);
            break;
        case TRAJECTORY_STOP         :
            where = PARTS_TRAJECTORY;
            sprintf(cmd, "rsc_cli send_to_can 12 FF05%02x%02x%02x 5",
                task_id, where, speed);
            break;
        case ALL_RESET               :
            where = PARTS_ALL;
            sprintf(cmd, "rsc_cli send_to_can 12 FE05%02x%02x%02x 5",
                task_id, where, speed);
            break;
        case ARMS_RESET              :
            where = PARTS_ARMS;
            sprintf(cmd, "rsc_cli send_to_can 12 FE05%02x%02x%02x 5",
                task_id, where, speed);
            break;
        case LEFT_ARM_RESET          :
            where = PARTS_LEFT_ARM;
            sprintf(cmd, "rsc_cli send_to_can 12 FE05%02x%02x%02x 5",
                task_id, where, speed);
            break;
        case RIGHT_ARM_RESET         :
            where = PARTS_RIGHT_ARM;
            sprintf(cmd, "rsc_cli send_to_can 12 FE05%02x%02x%02x 5",
                task_id, where, speed);
            break;
        case NECK_RESET              :
            where = PARTS_NECK;
            sprintf(cmd, "rsc_cli send_to_can 12 FE05%02x%02x%02x 5",
                task_id, where, speed);
            break;
        case WAIST_RESET             :
            where = PARTS_WAIST;
            sprintf(cmd, "rsc_cli send_to_can 12 FE05%02x%02x%02x 5",
                task_id, where, speed);
            break;
        case GO_500MM_SLOWLY         :
            distance = 500;
            speed = SPEED_SLOW;
            sprintf(cmd, "rsc_cli send_to_can 12 9707%02x01%02x%02x%02x 7",
                task_id, distance & 0xff, (distance >> 8) & 0xff, speed);
            break;
        case GO_500MM_NORMALLY       :
            distance = 500;
            speed = SPEED_MEDIUM;
            sprintf(cmd, "rsc_cli send_to_can 12 9707%02x01%02x%02x%02x 7",
                task_id, distance & 0xff, (distance >> 8) & 0xff, speed);
            break;
        case GO_500MM_QUICKLY        :
            distance = 500;
            speed = SPEED_FAST;
            sprintf(cmd, "rsc_cli send_to_can 12 9707%02x01%02x%02x%02x 7",
                task_id, distance & 0xff, (distance >> 8) & 0xff, speed);
            break;
        case GO_CONTINUED_SLOWLY     :
            distance = 0xAAAA;
            speed = SPEED_SLOW;
            sprintf(cmd, "rsc_cli send_to_can 12 9707%02x01%02x%02x%02x 7",
                task_id, distance & 0xff, (distance >> 8) & 0xff, speed);
            break;
        case GO_CONTINUED_NORMALLY   :
            distance = 0xAAAA;
            speed = SPEED_MEDIUM;
            sprintf(cmd, "rsc_cli send_to_can 12 9707%02x01%02x%02x%02x 7",
                task_id, distance & 0xff, (distance >> 8) & 0xff, speed);
            break;
        case GO_CONTINUED_QUICKLY    :
            distance = 0xAAAA;
            speed = SPEED_FAST;
            sprintf(cmd, "rsc_cli send_to_can 12 9707%02x01%02x%02x%02x 7",
                task_id, distance & 0xff, (distance >> 8) & 0xff, speed);
            break;
        case BACK_500MM_SLOWLY       :
            distance = 500;
            speed = SPEED_SLOW;
            sprintf(cmd, "rsc_cli send_to_can 12 9707%02x00%02x%02x%02x 7",
                task_id, distance & 0xff, (distance >> 8) & 0xff, speed);
            break;
        case BACK_500MM_NORMALLY     :
            distance = 500;
            speed = SPEED_MEDIUM;
            sprintf(cmd, "rsc_cli send_to_can 12 9707%02x00%02x%02x%02x 7",
                task_id, distance & 0xff, (distance >> 8) & 0xff, speed);
            break;
        case BACK_500MM_QUICKLY      :
            distance = 500;
            speed = SPEED_FAST;
            sprintf(cmd, "rsc_cli send_to_can 12 9707%02x00%02x%02x%02x 7",
                task_id, distance & 0xff, (distance >> 8) & 0xff, speed);
            break;
        case BACK_CONTINUED_SLOWLY   :
            distance = 0xAAAA;
            speed = SPEED_SLOW;
            sprintf(cmd, "rsc_cli send_to_can 12 9707%02x00%02x%02x%02x 7",
                task_id, distance & 0xff, (distance >> 8) & 0xff, speed);
            break;
        case BACK_CONTINUED_NORMALLY :
            distance = 0xAAAA;
            speed = SPEED_MEDIUM;
            sprintf(cmd, "rsc_cli send_to_can 12 9707%02x00%02x%02x%02x 7",
                task_id, distance & 0xff, (distance >> 8) & 0xff, speed);
            break;
        case BACK_CONTINUED_QUICKLY  :
            distance = 0xAAAA;
            speed = SPEED_FAST;
            sprintf(cmd, "rsc_cli send_to_can 12 9707%02x00%02x%02x%02x 7",
                task_id, distance & 0xff, (distance >> 8) & 0xff, speed);
            break;
        case TURN_LEFT_90_SLOWLY     :
            angle = 90;
            speed = SPEED_SLOW;
            sprintf(cmd, "rsc_cli send_to_can 12 9807%02x01%02x%02x%02x 7",
                task_id, angle & 0xff, (angle >> 8) & 0xff, speed);
            break;
        case TURN_LEFT_90_NORMALLY   :
            angle = 90;
            speed = SPEED_MEDIUM;
            sprintf(cmd, "rsc_cli send_to_can 12 9807%02x01%02x%02x%02x 7",
                task_id, angle & 0xff, (angle >> 8) & 0xff, speed);
            break;
        case TURN_LEFT_90_QUICKLY    :
            angle = 90;
            speed = SPEED_FAST;
            sprintf(cmd, "rsc_cli send_to_can 12 9807%02x01%02x%02x%02x 7",
                task_id, angle & 0xff, (angle >> 8) & 0xff, speed);
            break;
        case TURN_RIGHT_90_SLOWLY    :
            angle = 90;
            speed = SPEED_SLOW;
            sprintf(cmd, "rsc_cli send_to_can 12 9807%02x00%02x%02x%02x 7",
                task_id, angle & 0xff, (angle >> 8) & 0xff, speed);
            break;
        case TURN_RIGHT_90_NORMALLY  :
            angle = 90;
            speed = SPEED_MEDIUM;
            sprintf(cmd, "rsc_cli send_to_can 12 9807%02x00%02x%02x%02x 7",
                task_id, angle & 0xff, (angle >> 8) & 0xff, speed);
            break;
        case TURN_RIGHT_90_QUICKLY   :
            angle = 90;
            speed = SPEED_FAST;
            sprintf(cmd, "rsc_cli send_to_can 12 9807%02x00%02x%02x%02x 7",
                task_id, angle & 0xff, (angle >> 8) & 0xff, speed);
            break;
        case RAN                     :
            sprintf(cmd, "rsc_cli send_to_can 12 9C04%02x%02x 4", task_id,
                speed);
            system(cmd);
	    usleep(100000);
            memset(cmd, 0, sizeof(cmd));
            distance = 100;
            sprintf(cmd, "rsc_cli send_to_can 12 9706%02x01%02x%02x 6",
                task_id + 1, distance, speed);
            break;
        case HANDSHAKE               :
            sprintf(cmd, "rsc_cli send_to_can 12 9904%02x%02x 4", task_id,
                speed);
            break;
        case WAVE                    :
            sprintf(cmd, "rsc_cli send_to_can 12 9A04%02x%02x 4", task_id,
                speed);
            break;
        case CHEER                   :
            sprintf(cmd, "rsc_cli send_to_can 12 9B04%02x%02x 4", task_id,
                speed);
            break;
        case APPLAUD                 :
            sprintf(cmd, "rsc_cli send_to_can 12 9D04%02x%02x 4", task_id,
                speed);
            break;
        case AKIMBO                  :
            sprintf(cmd, "rsc_cli send_to_can 12 9E04%02x%02x 4", task_id,
                speed);
            break;
        case SALUTE                  :
            sprintf(cmd, "rsc_cli send_to_can 12 9F04%02x%02x 4", task_id,
                speed);
            break;
        case FOLD_ARMS               :
            sprintf(cmd, "rsc_cli send_to_can 12 A004%02x%02x 4", task_id,
                speed);
            break;
        case BLOWING_KISSES          :
            sprintf(cmd, "rsc_cli send_to_can 12 A104%02x%02x 4", task_id,
                speed);
            break;
        case HIGH_FIVES              :
            sprintf(cmd, "rsc_cli send_to_can 12 A204%02x%02x 4", task_id,
                speed);
            break;
        case HUGS                    :
            sprintf(cmd, "rsc_cli send_to_can 12 A304%02x%02x 4", task_id,
                speed);
            break;
        case KISS                    :
            sprintf(cmd, "rsc_cli send_to_can 12 A404%02x%02x 4", task_id,
                speed);
            break;
        case NOD                     :
            sprintf(cmd, "rsc_cli send_to_can 12 A504%02x%02x 4", task_id,
                speed);
            break;
        case SHAKING_HEAD            :
            sprintf(cmd, "rsc_cli send_to_can 12 A604%02x%02x 4", task_id,
                speed);
            break;
        case VICTORY                 :
            sprintf(cmd, "rsc_cli send_to_can 12 A704%02x%02x 4", task_id,
                speed);
            break;
        case THANKS_FIRST            :
            sprintf(cmd, "rsc_cli send_to_can 12 A804%02x%02x 4", task_id,
                speed);
            break;
        case THANKS_SECOND           :
            sprintf(cmd, "rsc_cli send_to_can 12 A904%02x%02x 4", task_id,
                speed);
            break;
        case THANKS_THIRD            :
            sprintf(cmd, "rsc_cli send_to_can 12 AA04%02x%02x 4", task_id,
                speed);
            break;
        case LAUGH_OUT_LOUD          :
            sprintf(cmd, "rsc_cli send_to_can 12 AB04%02x%02x 4", task_id,
                speed);
            break;
        case YAWN                    :
            sprintf(cmd, "rsc_cli send_to_can 12 AC04%02x%02x 4", task_id,
                speed);
            break;
        case LISTEN                  :
            sprintf(cmd, "rsc_cli send_to_can 12 AD04%02x%02x 4", task_id,
                speed);
            break;
        case LOOK_FIRST              :
            sprintf(cmd, "rsc_cli send_to_can 12 AE04%02x%02x 4", task_id,
                speed);
            break;
        case LOOK_SECOND             :
            sprintf(cmd, "rsc_cli send_to_can 12 AF04%02x%02x 4", task_id,
                speed);
            break;
        case DISTRESSED              :
            sprintf(cmd, "rsc_cli send_to_can 12 B004%02x%02x 4", task_id,
                speed);
            break;
        case SHY_BY_HAND             :
            sprintf(cmd, "rsc_cli send_to_can 12 B104%02x%02x 4", task_id,
                speed);
            break;
        case SPEECH                  :
            sprintf(cmd, "rsc_cli send_to_can 12 B204%02x%02x 4", task_id,
                speed);
            break;
        case ME                      :
            sprintf(cmd, "rsc_cli send_to_can 12 B304%02x%02x 4", task_id,
                speed);
            break;
        case US                      :
            sprintf(cmd, "rsc_cli send_to_can 12 B404%02x%02x 4", task_id,
                speed);
            break;
        case HIDDEN_HANDS            :
            sprintf(cmd, "rsc_cli send_to_can 12 B504%02x%02x 4", task_id,
                speed);
            break;
        case MOVE_RIGHT_HAND_FIRST   :
            sprintf(cmd, "rsc_cli send_to_can 12 B604%02x%02x 4", task_id,
                speed);
            break;
        case MOVE_RIGHT_HAND_SECOND  :
            sprintf(cmd, "rsc_cli send_to_can 12 B704%02x%02x 4", task_id,
                speed);
            break;
        case MOVE_RIGHT_HAND_THIRD   :
            sprintf(cmd, "rsc_cli send_to_can 12 B804%02x%02x 4", task_id,
                speed);
            break;
        case MOVE_RIGHT_HAND_FOURTH  :
            sprintf(cmd, "rsc_cli send_to_can 12 B904%02x%02x 4", task_id,
                speed);
            break;
        case MOVE_RIGHT_HAND_FIFTH   :
            sprintf(cmd, "rsc_cli send_to_can 12 BA04%02x%02x 4", task_id,
                speed);
            break;
        case ROCK                    :
            sprintf(cmd, "rsc_cli send_to_can 12 BB04%02x%02x 4", task_id,
                speed);
            break;
        case SCISSORS                :
            sprintf(cmd, "rsc_cli send_to_can 12 BC04%02x%02x 4", task_id,
                speed);
            break;
        case PAPER                   :
            sprintf(cmd, "rsc_cli send_to_can 12 BD04%02x%02x 4", task_id,
                speed);
            break;
        case HORIZONTAL_OFFSET       :
            direction = HORIZONTAL_OFFSET_LEFT;
            distance = 500;
            sprintf(cmd, "rsc_cli send_to_can 12 BE07%02x%02x%02x%02x%02x 7",
                task_id, direction, distance & 0xff, (distance >> 8) & 0xff,
                speed);
            break;
        case WAKE                    :
            sprintf(cmd, "rsc_cli send_to_can 12 BF04%02x%02x 4", task_id,
                speed);
            break;
        case FRIGHTENED              :
            sprintf(cmd, "rsc_cli send_to_can 12 C004%02x%02x 4", task_id,
                speed);
            break;
        case DONT_TICKLE_RIGHT       :
            sprintf(cmd, "rsc_cli send_to_can 12 C104%02x%02x 4", task_id,
                speed);
            break;
        case DONT_TICKLE_LEFT        :
            sprintf(cmd, "rsc_cli send_to_can 12 C204%02x%02x 4", task_id,
                speed);
            break;
        case TWO_HANDS_PUSH_FORWARD  :
            sprintf(cmd, "rsc_cli send_to_can 12 C304%02x%02x 4", task_id,
                speed);
            break;
        case INCOMING_CALL           :
            sprintf(cmd, "rsc_cli send_to_can 12 C404%02x%02x 4", task_id,
                speed);
            break;
        case MJ_DANCE_POSE           :
            sprintf(cmd, "rsc_cli send_to_can 12 C504%02x%02x 4", task_id,
                speed);
            break;
        case BALLET_DANCE_POSE       :
            sprintf(cmd, "rsc_cli send_to_can 12 C604%02x%02x 4", task_id,
                speed);
            break;
        case TAKE_PHOTO              :
            sprintf(cmd, "rsc_cli send_to_can 12 C704%02x%02x 4", task_id,
                speed);
            break;
        case UPGRADE                 :
            sprintf(cmd, "rsc_cli send_to_can 12 C804%02x%02x 4", task_id,
                speed);
            break;
        case TURN_BOOK               :
            sprintf(cmd, "rsc_cli send_to_can 12 C904%02x%02x 4", task_id,
                speed);
            break;
        case STOP_TALKING            :
            sprintf(cmd, "rsc_cli send_to_can 12 CA04%02x%02x 4", task_id,
                speed);
            break;
        case DONT_TOUCH_ME           :
            sprintf(cmd, "rsc_cli send_to_can 12 CB04%02x%02x 4", task_id,
                speed);
            break;
        case OH_YEAH                 :
            sprintf(cmd, "rsc_cli send_to_can 12 CC04%02x%02x 4", task_id,
                speed);
            break;
        case READY_TO_FOLLOW         :
            sprintf(cmd, "rsc_cli send_to_can 12 CD04%02x%02x 4", task_id,
                speed);
            break;
        case FOLLOWING               :
            sprintf(cmd, "rsc_cli send_to_can 12 CE04%02x%02x 4", task_id,
                speed);
            break;
        case WIPE_PERSPIRATION       :
            sprintf(cmd, "rsc_cli send_to_can 12 CF04%02x%02x 4", task_id,
                speed);
            break;
        case RAIN                    :
            sprintf(cmd, "rsc_cli send_to_can 12 D004%02x%02x 4", task_id,
                speed);
            break;
        case SNOW                    :
            sprintf(cmd, "rsc_cli send_to_can 12 D104%02x%02x 4", task_id,
                speed);
            break;
        case SELF_PORTECTION         :
            sprintf(cmd, "rsc_cli send_to_can 12 D204%02x%02x 4", task_id,
                speed);
            break;
        case NARRATE                 :
            sprintf(cmd, "rsc_cli send_to_can 12 D304%02x%02x 4", task_id,
                speed);
            break;
        case GO_MAZE                 :
            action = MAZE_ACTION_BEGIN;
            sprintf(cmd, "rsc_cli send_to_can 13 FA04%02x%02x 4", task_id,
                action);
            break;
        case AVOID_OBSTACLE          :
            action = AVOID_OBSTACLE_ENABLE;
            sprintf(cmd, "rsc_cli send_to_can 13 FB04%02x%02x 4", task_id,
                action);
            break;
        default:
            break;
    }

    system(cmd);

    return FAC_EXEC_NO_ERROR;
}

int fac_set_power_mode(enum POWER_MODE mode)
{
    char cmd[1024] = {0};

    if (mode != PM_ACTIVE && mode != PM_STANDBY &&
        mode != PM_SLEEP && mode != PM_POWEROFF)
        return FAC_INVALID_POWER_MODE;

    switch (mode)
    {
        case PM_ACTIVE:
            /*Wakeup and do some actions*/
            sprintf(cmd, "rsc_cli receive_from_can 06 0502 2");//wakeup
            system(cmd);
            usleep(200000);
            fac_motor_motion(ALL_RESET);
            usleep(1500000);
            fac_motor_motion(WAVE);//Use wave temporarily
            break;
        case PM_STANDBY:
            /*Wakeup and just keep idle*/
            sprintf(cmd, "rsc_cli receive_from_can 06 0502 2");//wakeup
            system(cmd);
            usleep(200000);
            fac_motor_motion(ALL_RESET);
            break;
        case PM_SLEEP:
            /*Sleep*/
            fac_motor_motion(ALL_RESET);
            usleep(1500000);
            fac_motor_motion(ALL_STOP);
            usleep(100000);
            sprintf(cmd, "rsc_cli receive_from_can 06 030300 3");//sleep
            system(cmd);
            break;
        case PM_POWEROFF:
            /*Power off*/
            strcpy(cmd, "rsc_cli send_to_can 20 41048000 4");//power off all
            system(cmd);
            break;
        default:
            break;
    }

    return FAC_EXEC_NO_ERROR;
}

