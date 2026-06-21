#ifndef _CHASSIS_TASK_H
#define _CHASSIS_TASK_H

#include <stdint.h>
#include "motor.h"
#include "bsp_CAN.h"
#include "bsp_dwt.h"
#include "remote_control.h"
#include <math.h>

#define CHASSIS_TASK_PERIOD 2

#define VELOCITY_RATIO 3//速度比例
#define RC_STICK_ROTATE_RATIO 0.72//旋转比例

#define wheel_radius 76.00f//车轮半径
#define pi 3.1415926f

#define SQRT2 1.4142135f
#define WHEEL_OPPOSITE 0.2675f

#ifdef ARM_MATH_DSP
#define user_cos arm_cos_f32
#define user_sin arm_sin_f32
#else
#define user_cos cosf
#define user_sin sinf
#endif

typedef struct _Chassis_t
{
    float Vx, Vy, Vr;
    float VxTransfer, VyTransfer;

    float VelocityRatio;
    float rcStickRotateRatio;

    float V1, V2, V3, V4;

    uint8_t Mode;
    uint8_t LastMode;

    Motor_t ChassisMotor[4];

    float WheelRadius;
    float WheelReductionRatio;
} Chassis_t;

enum
{
    Follow_Mode = 0,
    Silence_Mode,
};

enum
{
    FR = 0,
    FL = 1,
    HL = 2,
    HR = 3
};

void Chassis_Control(void);
void Chassis_Init(void);

extern Chassis_t Chassis;

#endif
