/*************************************************
 Copyright (C), 2014-2016, Avatarmind. Co., Ltd.
 File name: robot_fac.h
 Author: wangkun
 Version: v1.0
 Date: 2016-08-17
 Description: This program defined api for factory.
 Others: None
 
 Function List: 
   1. fac_motor_move is to drive the motor.
   2. fac_motor_stop is to stop a motor.
   3. fac_motor_calibration is to calibrate a motor.
   4. fac_motor_calibration2 is to calibrate a board
      of motor.
   5. fac_motor_motion is to do a trajectory action.
 
 History:
   V1.0    2016-08-17    wangkun    init version
*************************************************/
#ifndef __ROBOT_FAC_H_
#define __ROBOT_FAC_H_

#ifdef __cplusplus
extern "C"{
#endif

enum RB_TYPE {
    RB_RIGHT = 0,
    RB_LEFT  = 1,
    RB_BODY  = 2,
    RB_MAX
};

enum MOTOR_ID {
    INVALID_MOTOR_ID        = 0x00,

    ARM_ROTATION_RIGHT      = 0x01,
    ARM_SWING_RIGHT         = 0x02,
    FORARM_ROTATION_RIGHT   = 0x03,
    FORARM_SWING_RIGHT      = 0x04,
    WRIST_RIGHT             = 0x05,
    THUMB_RIGHT             = 0x06,
    INDEXFINGER_RIGHT       = 0x07,
    MIDDLEFINGER_RIGHT      = 0x08,

    ARM_ROTATION_LEFT       = 0x09,
    ARM_SWING_LEFT          = 0x0A,
    FORARM_ROTATION_LEFT    = 0x0B,
    FORARM_SWING_LEFT       = 0x0C,
    WRIST_LEFT              = 0x0D,
    THUMB_LEFT              = 0x0E,
    INDEXFINGER_LEFT        = 0x0F,
    MIDDLEFINGER_LEFT       = 0x10,

    WHEEL_LEFT              = 0x11,
    WHEEL_RIGHT             = 0x12,
    WAIST_TILT              = 0x13,
    NECK_ROTATION           = 0x14,
    NECK_TILT               = 0x15,

    RINGFINGER_RIGHT        = 0x16,
    LITTLEFINGER_RIGHT      = 0x17,
    RINGFINGER_LEFT         = 0x18,
    LITTLEFINGER_LEFT       = 0x19,
    MOTOR_MAX
};

#define MODULE_DRIVE_PER_MOTOR    \
    0xFF,/*INVALID_MOTOR_ID*/     \
                                  \
    0x41,/*ARM_ROTATION_RIGHT*/   \
    0x41,/*ARM_SWING_RIGHT*/      \
    0x41,/*FORARM_ROTATION_RIGHT*/\
    0x41,/*FORARM_SWING_RIGHT*/   \
    0x41,/*WRIST_RIGHT*/          \
    0x41,/*THUMB_RIGHT*/          \
    0x41,/*INDEXFINGER_RIGHT*/    \
    0x41,/*MIDDLEFINGER_RIGHT*/   \
                                  \
    0x51,/*ARM_ROTATION_LEFT*/    \
    0x51,/*ARM_SWING_LEFT*/       \
    0x51,/*FORARM_ROTATION_LEFT*/ \
    0x51,/*FORARM_SWING_LEFT*/    \
    0x51,/*WRIST_LEFT*/           \
    0x51,/*THUMB_LEFT*/           \
    0x51,/*INDEXFINGER_LEFT*/     \
    0x51,/*MIDDLEFINGER_LEFT*/    \
                                  \
    0x61,/*WHEEL_LEFT*/           \
    0x61,/*WHEEL_RIGHT*/          \
    0x61,/*WAIST_TILT*/           \
    0x61,/*NECK_ROTATION*/        \
    0x61,/*NECK_TILT*/            \
                                  \
    0x41,/*RINGFINGER_RIGHT*/     \
    0x41,/*LITTLEFINGER_RIGHT*/   \
    0x51,/*RINGFINGER_LEFT*/      \
    0x51,/*LITTLEFINGER_LEFT*/

#define MODULE_CALIB_PER_MOTOR    \
    0xFF,/*INVALID_MOTOR_ID*/     \
                                  \
    0x40,/*ARM_ROTATION_RIGHT*/   \
    0x40,/*ARM_SWING_RIGHT*/      \
    0x40,/*FORARM_ROTATION_RIGHT*/\
    0x40,/*FORARM_SWING_RIGHT*/   \
    0x40,/*WRIST_RIGHT*/          \
    0x40,/*THUMB_RIGHT*/          \
    0x40,/*INDEXFINGER_RIGHT*/    \
    0x40,/*MIDDLEFINGER_RIGHT*/   \
                                  \
    0x50,/*ARM_ROTATION_LEFT*/    \
    0x50,/*ARM_SWING_LEFT*/       \
    0x50,/*FORARM_ROTATION_LEFT*/ \
    0x50,/*FORARM_SWING_LEFT*/    \
    0x50,/*WRIST_LEFT*/           \
    0x50,/*THUMB_LEFT*/           \
    0x50,/*INDEXFINGER_LEFT*/     \
    0x50,/*MIDDLEFINGER_LEFT*/    \
                                  \
    0x60,/*WHEEL_LEFT*/           \
    0x60,/*WHEEL_RIGHT*/          \
    0x60,/*WAIST_TILT*/           \
    0x60,/*NECK_ROTATION*/        \
    0x60,/*NECK_TILT*/            \
                                  \
    0x40,/*RINGFINGER_RIGHT*/     \
    0x40,/*LITTLEFINGER_RIGHT*/   \
    0x50,/*RINGFINGER_LEFT*/      \
    0x50,/*LITTLEFINGER_LEFT*/

#define MODULE_CALIB_PER_RB_TYPE  \
    0x40,/*RB_RIGHT*/             \
    0x50,/*RB_LEFT*/              \
    0x60,/*RB_BODYT*/

#define MIN_ANGLE_ARRAY_PER_MOTOR \
      0,/*INVALID_MOTOR_ID*/      \
                                  \
    -25,/*ARM_ROTATION_RIGHT*/    \
      0,/*ARM_SWING_RIGHT*/       \
    -80,/*FORARM_ROTATION_RIGHT*/ \
      0,/*FORARM_SWING_RIGHT*/    \
    -80,/*WRIST_RIGHT*/           \
      0,/*THUMB_RIGHT*/           \
      0,/*INDEXFINGER_RIGHT*/     \
      0,/*MIDDLEFINGER_RIGHT*/    \
                                  \
    -25,/*ARM_ROTATION_LEFT*/     \
      0,/*ARM_SWING_LEFT*/        \
    -80,/*FORARM_ROTATION_LEFT*/  \
      0,/*FORARM_SWING_LEFT*/     \
    -80,/*WRIST_LEFT*/            \
      0,/*THUMB_LEFT*/            \
      0,/*INDEXFINGER_LEFT*/      \
      0,/*MIDDLEFINGER_LEFT*/     \
                                  \
   -360,/*WHEEL_LEFT*/            \
   -360,/*WHEEL_RIGHT*/           \
    -10,/*WAIST_TILT*/            \
    -45,/*NECK_ROTATION*/         \
    -12,/*NECK_TILT*/             \
                                  \
      0,/*RINGFINGER_RIGHT*/      \
      0,/*LITTLEFINGER_RIGHT*/    \
      0,/*RINGFINGER_LEFT*/       \
      0,/*LITTLEFINGER_LEFT*/

#define MAX_ANGLE_ARRAY_PER_MOTOR \
      0,/*INVALID_MOTOR_ID*/      \
                                  \
    175,/*ARM_ROTATION_RIGHT*/    \
     65,/*ARM_SWING_RIGHT*/       \
     80,/*FORARM_ROTATION_RIGHT*/ \
     90,/*FORARM_SWING_RIGHT*/    \
     80,/*WRIST_RIGHT*/           \
      1,/*THUMB_RIGHT*/           \
      1,/*INDEXFINGER_RIGHT*/     \
      1,/*MIDDLEFINGER_RIGHT*/    \
                                  \
    175,/*ARM_ROTATION_LEFT*/     \
     65,/*ARM_SWING_LEFT*/        \
     80,/*FORARM_ROTATION_LEFT*/  \
     90,/*FORARM_SWING_LEFT*/     \
     80,/*WRIST_LEFT*/            \
      0,/*THUMB_LEFT*/            \
      0,/*INDEXFINGER_LEFT*/      \
      0,/*MIDDLEFINGER_LEFT*/     \
                                  \
    360,/*WHEEL_LEFT*/            \
    360,/*WHEEL_RIGHT*/           \
     10,/*WAIST_TILT*/            \
     45,/*NECK_ROTATION*/         \
     15,/*NECK_TILT*/             \
                                  \
      0,/*RINGFINGER_RIGHT*/      \
      0,/*LITTLEFINGER_RIGHT*/    \
      0,/*RINGFINGER_LEFT*/       \
      0,/*LITTLEFINGER_LEFT*/

enum MOTOR_DIRECTION {
    POSITIVE = 1,
    NEGATIVE = 2,
};

#define SLOW_STOP        0
#define EMERGENCY_STOP   1
#define NOT_STOP         2  

enum MOTION_ID {
    ALL_STOP                = 0x01,    ARMS_STOP               = 0x02,
    LEFT_ARM_STOP           = 0x03,    RIGHT_ARM_STOP          = 0x04, 
    WHEELS_STOP             = 0x05,    TRAJECTORY_STOP         = 0x06,
    ALL_RESET               = 0x07,    ARMS_RESET              = 0x08,
    LEFT_ARM_RESET          = 0x09,    RIGHT_ARM_RESET         = 0x0A,
    NECK_RESET              = 0x0B,    WAIST_RESET             = 0x0C,
    GO_500MM_SLOWLY         = 0x0D,    GO_500MM_NORMALLY       = 0x0E,
    GO_500MM_QUICKLY        = 0x0F,    GO_CONTINUED_SLOWLY     = 0x10,
    GO_CONTINUED_NORMALLY   = 0x11,    GO_CONTINUED_QUICKLY    = 0x12,
    BACK_500MM_SLOWLY       = 0x13,    BACK_500MM_NORMALLY     = 0x14,
    BACK_500MM_QUICKLY      = 0x15,    BACK_CONTINUED_SLOWLY   = 0x16,
    BACK_CONTINUED_NORMALLY = 0x17,    BACK_CONTINUED_QUICKLY  = 0x18,
    TURN_LEFT_90_SLOWLY     = 0x19,    TURN_LEFT_90_NORMALLY   = 0x1A,
    TURN_LEFT_90_QUICKLY    = 0x1B,    TURN_RIGHT_90_SLOWLY    = 0x1C,
    TURN_RIGHT_90_NORMALLY  = 0x1D,    TURN_RIGHT_90_QUICKLY   = 0x1E,
    RAN                     = 0x1F,    HANDSHAKE               = 0x20,
    WAVE                    = 0x21,    CHEER                   = 0x22,
    APPLAUD                 = 0x23,    AKIMBO                  = 0x24,
    SALUTE                  = 0x25,    FOLD_ARMS               = 0x26,
    BLOWING_KISSES          = 0x27,    HIGH_FIVES              = 0x28,
    HUGS                    = 0x29,    KISS                    = 0x2A,
    NOD                     = 0x2B,    SHAKING_HEAD            = 0x2C,
    VICTORY                 = 0x2D,    THANKS_FIRST            = 0x2E,
    THANKS_SECOND           = 0x2F,    THANKS_THIRD            = 0x30,
    LAUGH_OUT_LOUD          = 0x31,    YAWN                    = 0x32,
    LISTEN                  = 0x33,    LOOK_FIRST              = 0x34,
    LOOK_SECOND             = 0x35,    DISTRESSED              = 0x36,
    SHY_BY_HAND             = 0x37,    SPEECH                  = 0x38,
    ME                      = 0x39,    US                      = 0x3A,
    HIDDEN_HANDS            = 0x3B,    MOVE_RIGHT_HAND_FIRST   = 0x3C,
    MOVE_RIGHT_HAND_SECOND  = 0x3D,    MOVE_RIGHT_HAND_THIRD   = 0x3E,
    MOVE_RIGHT_HAND_FOURTH  = 0x3F,    MOVE_RIGHT_HAND_FIFTH   = 0x40,
    ROCK                    = 0x41,    SCISSORS                = 0x42,
    PAPER                   = 0x43,    HORIZONTAL_OFFSET       = 0x44,
    WAKE                    = 0x45,    FRIGHTENED              = 0x46,
    DONT_TICKLE_RIGHT       = 0x47,    DONT_TICKLE_LEFT        = 0x48,
    TWO_HANDS_PUSH_FORWARD  = 0x49,    INCOMING_CALL           = 0x4A,
    MJ_DANCE_POSE           = 0x4B,    BALLET_DANCE_POSE       = 0x4C,
    TAKE_PHOTO              = 0x4D,    UPGRADE                 = 0x4E,
    TURN_BOOK               = 0x4F,    STOP_TALKING            = 0x50,
    DONT_TOUCH_ME           = 0x51,    OH_YEAH                 = 0x52,
    READY_TO_FOLLOW         = 0x53,    FOLLOWING               = 0x54,
    WIPE_PERSPIRATION       = 0x55,    RAIN                    = 0x56,
    SNOW                    = 0x57,    SELF_PORTECTION         = 0x58,
    NARRATE                 = 0x59,    GO_MAZE                 = 0x5A,
    AVOID_OBSTACLE          = 0x5B,    MOTION_MAX
};

enum POWER_MODE {
    PM_ACTIVE    = 1,
    PM_STANDBY   = 2,
    PM_SLEEP     = 3,
    PM_POWEROFF  = 4,
};

enum ROBOT_FAC_ERROR {
    FAC_EXEC_NO_ERROR           =  0,
    FAC_EXEC_FAILED             = -1,
    FAC_INVALID_ANGLE           = -2,
    FAC_INVALID_TIME            = -3,
    FAC_INVALID_MOTOR_ID        = -4,
    FAC_INVALID_MOTOR_DIRECTION = -5,
    FAC_INVALID_RB_TYPE         = -6,
    FAC_INVALID_MOTION_ID       = -7,
    FAC_INVALID_POWER_MODE      = -8,
};

int fac_motor_move(enum MOTOR_ID id, enum MOTOR_DIRECTION direction,
    unsigned int angle, unsigned int run_time);
int fac_motor_stop(enum MOTOR_ID id);
int fac_motor_calibration(enum MOTOR_ID id);
int fac_motor_calibration2(enum RB_TYPE type);
int fac_motor_motion(enum MOTION_ID id);
int fac_set_power_mode(enum POWER_MODE mode);

#ifdef __cplusplus
}
#endif
#endif//__ROBOT_FAC_H_
