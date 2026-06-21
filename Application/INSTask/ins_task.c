/**
 ******************************************************************************
 * @file    ins_task.c
 * @brief   INS任务 — 从百零一移植(简化版，无PWM温控)
 ******************************************************************************
 */
#include "ins_task.h"
#include "QuaternionAHRS.h"
#include "BMI088driver.h"
#include "bsp_dwt.h"
#include "user_lib.h"
#include "GravityEstimateKF.h"
#include "cmsis_os.h"

#define USER_GetTick xTaskGetTickCount

uint32_t INS_DWT_Count = 0;
static float dt = 0, t = 0;

void INS_Init(void)
{
    // 重力估计卡尔曼滤波器初始化
    gEstimateKF_Init(0.01, 100000);
}

void INS_Task(void)
{
    dt = DWT_GetDeltaT(&INS_DWT_Count);
    t += dt;

    // 读取BMI088 IMU数据
    BMI088_Read(&BMI088);

    for (uint8_t i = 0; i < 3; i++)
    {
        AHRS.Accel[i] = BMI088.Accel[i];
        AHRS.Gyro[i] = BMI088.Gyro[i];
    }

    // 重力估计 + AHRS姿态解算
    gEstimateKF_Update(BMI088.Gyro[X], BMI088.Gyro[Y], BMI088.Gyro[Z],
                       BMI088.Accel[X], BMI088.Accel[Y], BMI088.Accel[Z], dt);
    Quaternion_AHRS_UpdateIMU(BMI088.Gyro[X], BMI088.Gyro[Y], BMI088.Gyro[Z],
                               gVec[X], gVec[Y], gVec[Z], dt);
    InsertQuaternionFrame(&QuaternionBuffer, AHRS.q, USER_GetTick() / 1000.0f);

    // 计算欧拉角 (AHRS.Yaw, AHRS.YawTotalAngle)
    Get_EulerAngle(AHRS.q);
}
