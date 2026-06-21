#include "chassis_task.h"
#include "detect_task.h"
#include "QuaternionAHRS.h"

Chassis_t Chassis = {0};
uint32_t Chassis_DWT_Count = 0;

float MAX_RPM = 8000;
float Vx_k = 2000;
float Vy_k = 2000;

static float dt = 0;

static void Chassis_Set_Mode(void);
static void Chassis_Get_CtrlValue(void);
static void Chassis_Set_Control(void);
static void Send_Chassis_Current(void);
static float Max_4(float num1, float num2, float num3, float num4);
static void Velocity_MAXLimit(void);

void Chassis_Init(void)
{
    Chassis.WheelRadius = 0.0765f;
    Chassis.WheelReductionRatio = (3591.0f / 187.0f);//减速比
    Chassis.VelocityRatio = VELOCITY_RATIO;
    Chassis.rcStickRotateRatio = RC_STICK_ROTATE_RATIO;//0

    // 电机速度环PID初始化
    for (uint8_t i = 0; i < 4; i++)
    {
        PID_Init(&Chassis.ChassisMotor[i].PID_Velocity, 16384, 2000, 0, 50, 0.1, 0, 65, 15, 0.005, 0, 1,
                 Integral_Limit | OutputFilter | ChangingIntegrationRate);
        Chassis.ChassisMotor[i].Max_Out = 16384.0f;
    }
}

void Chassis_Control(void)
{
    dt = DWT_GetDeltaT(&Chassis_DWT_Count);

    // 设置底盘运动模式
    Chassis_Set_Mode();
    // 处理来自摇杆与键盘的控制数据
    Chassis_Get_CtrlValue();
    // 底盘运动解算以及PID计算
    Chassis_Set_Control();
    // 发送底盘电机控制电流
    Send_Chassis_Current();
}

static void Chassis_Get_CtrlValue(void)
{
    static float Temp_Vx, Temp_Vy;
    static float tempVal;
    static float RC = 0.000001f;

    Temp_Vx = 0;
    Temp_Vy = 0;

    // 遥控器数值转化为车体速度
    Temp_Vx += (remote_control.ch3 * 2.64f / 660.0f);
    Temp_Vy += (remote_control.ch4 * 2.64f / 660.0f);

    // 缓启动 Vx
    if (fabsf(Temp_Vx) > 1e-3f)
    {
        if (Chassis.Vx * Temp_Vx < 0)
            Chassis.Vx = 0;
        tempVal = (Temp_Vx - Chassis.Vx) / (RC + dt);
        if (tempVal > Vx_k)
            tempVal = Vx_k;
        else if (tempVal < -Vx_k)
            tempVal = -Vx_k;
        Chassis.Vx += tempVal * dt;
    }
    else
    {
        tempVal = (Temp_Vx - Chassis.Vx) / (0.01f + dt);
        Chassis.Vx += tempVal * dt;
    }

    // 缓启动 Vy
    if (fabsf(Temp_Vy) > 1e-3f)
    {
        if (Chassis.Vy * Temp_Vy < 0)
            Chassis.Vy = 0;
        tempVal = (Temp_Vy - Chassis.Vy) / (RC + dt);
        if (tempVal > Vy_k)
            tempVal = Vy_k;
        else if (tempVal < -Vy_k)
            tempVal = -Vy_k;
        Chassis.Vy += 0.8f * tempVal * dt;
    }
    else
    {
        tempVal = (Temp_Vy - Chassis.Vy) / (0.01f + dt);
        Chassis.Vy += 0.8f * tempVal * dt;
    }

    Chassis.Vx = float_constrain(Chassis.Vx, -2.64f, 2.64f);
    Chassis.Vy = float_constrain(Chassis.Vy, -2.64f, 2.64f);
}

static void Chassis_Set_Mode(void)
{
    // 4个底盘电机全部离线 → 强制静止
    if (is_TOE_Error(CHASSIS_MOTOR1_TOE) &&
        is_TOE_Error(CHASSIS_MOTOR2_TOE) &&
        is_TOE_Error(CHASSIS_MOTOR3_TOE) &&
        is_TOE_Error(CHASSIS_MOTOR4_TOE))
    {
        Chassis.Mode = Silence_Mode;
    }
    else
    {
        // 左拨杆切换旋转模式（右拨杆已被云台占用控制枪管）
        Chassis.Mode = Follow_Mode;
    }

    Chassis.LastMode = Chassis.Mode;
}

static void Chassis_Set_Control(void)
{
    float yaw_rad; // IMU Yaw角(弧度) — 对标百零一 SpinningValidTheta

    switch (Chassis.Mode)
    {
    case Follow_Mode:
        // 左拨杆向下(Switch_Down=2)：自动旋转 + 摇杆平移
        if (remote_control.switch_left == Switch_Down)
        {
            Chassis.Vr = 50.0f; // 自动旋转角速度 50°/s
        }
        else
        {
            Chassis.Vr = 0; // 不自动旋转
        }

        // 使用IMU真实Yaw角(度→弧度) — 对标百零一 SpinningValidTheta
        yaw_rad = AHRS.Yaw * pi / 180.0f;

        // 对标百零一 Battle_MODE 坐标变换: sin(θ)*Vx - cos(θ)*Vy, cos(θ)*Vx + sin(θ)*Vy
        Chassis.VxTransfer =  user_sin(yaw_rad) * Chassis.Vx - user_cos(yaw_rad) * Chassis.Vy;
        Chassis.VyTransfer =  user_cos(yaw_rad) * Chassis.Vx + user_sin(yaw_rad) * Chassis.Vy;
        break;

    case Silence_Mode:
    default:
        Chassis.Vr = 0;
        Chassis.VxTransfer = 0;
        Chassis.VyTransfer = 0;
        break;
    }

    // 麦克纳姆轮逆运动学
    Chassis.V1 = ((Chassis.VxTransfer - Chassis.VyTransfer) * Chassis.VelocityRatio + (WHEEL_OPPOSITE * SQRT2) * Chassis.Vr) / (wheel_radius * 0.001f * SQRT2);
    Chassis.V2 = ((Chassis.VxTransfer + Chassis.VyTransfer) * Chassis.VelocityRatio + (WHEEL_OPPOSITE * SQRT2) * Chassis.Vr) / (wheel_radius * 0.001f * SQRT2);
    Chassis.V3 = ((-Chassis.VxTransfer + Chassis.VyTransfer) * Chassis.VelocityRatio + (WHEEL_OPPOSITE * SQRT2) * Chassis.Vr) / (wheel_radius * 0.001f * SQRT2);
    Chassis.V4 = ((-Chassis.VxTransfer - Chassis.VyTransfer) * Chassis.VelocityRatio + (WHEEL_OPPOSITE * SQRT2) * Chassis.Vr) / (wheel_radius * 0.001f * SQRT2);

    Velocity_MAXLimit();

    // 电机速度PID控制
    Motor_Speed_Calculate(&Chassis.ChassisMotor[0], Chassis.ChassisMotor[0].Velocity_RPM / Chassis.WheelReductionRatio, Chassis.V1);
    Motor_Speed_Calculate(&Chassis.ChassisMotor[1], Chassis.ChassisMotor[1].Velocity_RPM / Chassis.WheelReductionRatio, Chassis.V2);
    Motor_Speed_Calculate(&Chassis.ChassisMotor[2], Chassis.ChassisMotor[2].Velocity_RPM / Chassis.WheelReductionRatio, Chassis.V3);
    Motor_Speed_Calculate(&Chassis.ChassisMotor[3], Chassis.ChassisMotor[3].Velocity_RPM / Chassis.WheelReductionRatio, Chassis.V4);
}

static void Send_Chassis_Current(void)
{
    // 遥控器离线 → 发送0电流保护
    if (is_TOE_Error(RC_TOE))
    {
        Send_Motor_Current_1_4(&hcan1, 0, 0, 0, 0);
    }
    else
    {
        Send_Motor_Current_1_4(&hcan1,
                               (int16_t)Chassis.ChassisMotor[0].Output,
                               (int16_t)Chassis.ChassisMotor[1].Output,
                               (int16_t)Chassis.ChassisMotor[2].Output,
                               (int16_t)Chassis.ChassisMotor[3].Output);
    }
}

static float Max_4(float num1, float num2, float num3, float num4)
{
    static float max_num = 0;
    max_num = fabsf(num1);
    if (fabsf(num2) > max_num)
        max_num = fabsf(num2);
    if (fabsf(num3) > max_num)
        max_num = fabsf(num3);
    if (fabsf(num4) > max_num)
        max_num = fabsf(num4);
    return max_num;
}

static void Velocity_MAXLimit(void)
{
    static float TempMaxVelocity;
    if (Max_4(fabsf(Chassis.V1), fabsf(Chassis.V2), fabsf(Chassis.V3), fabsf(Chassis.V4)) > MAX_RPM)
    {
        TempMaxVelocity = Max_4(fabsf(Chassis.V1), fabsf(Chassis.V2), fabsf(Chassis.V3), fabsf(Chassis.V4));
        Chassis.V1 *= MAX_RPM / TempMaxVelocity;
        Chassis.V2 *= MAX_RPM / TempMaxVelocity;
        Chassis.V3 *= MAX_RPM / TempMaxVelocity;
        Chassis.V4 *= MAX_RPM / TempMaxVelocity;
    }
}
