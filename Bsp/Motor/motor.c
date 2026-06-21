#include "motor.h"
#include "bsp_CAN.h"
#include "user_lib.h"

float Motor_Torque_Calculate(Motor_t *motor, float torque, float target_torque)
{
    Feedforward_Calculate(&motor->FFC_Torque, target_torque);
    PID_Calculate(&motor->PID_Torque, torque, target_torque);

    if (motor->TorqueCtrl_User_Func_f != NULL)
        motor->TorqueCtrl_User_Func_f(motor);

    if (motor->Direction != NEGATIVE)
        motor->Output = motor->FFC_Torque.Output + motor->PID_Torque.Output + motor->Ke * motor->Velocity_RPM;
    else
        motor->Output = motor->FFC_Torque.Output + motor->PID_Torque.Output - motor->Ke * motor->Velocity_RPM;

    motor->Output = float_constrain(motor->Output, -motor->Max_Out, motor->Max_Out);
    return motor->Output;
}

float Motor_Speed_Calculate(Motor_t *motor, float velocity, float target_speed)
{
    Feedforward_Calculate(&motor->FFC_Velocity, target_speed);
    PID_Calculate(&motor->PID_Velocity, velocity, target_speed);

    if (motor->SpeedCtrl_User_Func_f != NULL)
        motor->SpeedCtrl_User_Func_f(motor);

    motor->Output = motor->FFC_Velocity.Output + motor->PID_Velocity.Output - motor->LDOB.Disturbance;
    motor->Output = float_constrain(motor->Output, -motor->Max_Out, motor->Max_Out);
    return motor->Output;
}

float Motor_Angle_Calculate(Motor_t *motor, float angle, float velocity, float target_angle)
{
    Feedforward_Calculate(&motor->FFC_Angle, target_angle);
    PID_Calculate(&motor->PID_Angle, angle, target_angle);

    if (motor->AngleCtrl_User_Func_f != NULL)
        motor->AngleCtrl_User_Func_f(motor);

    Motor_Speed_Calculate(motor, velocity, motor->FFC_Angle.Output + motor->PID_Angle.Output);
    return motor->Output;
}

void get_moto_info(Motor_t *ptr, uint8_t *aData)
{
    const float lpf = 2 * PI * 10 * 0.001 / (1 + 2 * PI * 10 * 0.001);

    if (ptr->Direction != NEGATIVE)
    {
        ptr->RawAngle = (uint16_t)(aData[0] << 8 | aData[1]);
        ptr->Velocity_RPM = (int16_t)(aData[2] << 8 | aData[3]);
    }
    else
    {
        ptr->RawAngle = 8191 - (uint16_t)(aData[0] << 8 | aData[1]);
        ptr->Velocity_RPM = -(int16_t)(aData[2] << 8 | aData[3]);
    }

    if (ptr->firstRun == 0)
    {
        ptr->firstRun = 1;
        ptr->last_Velocity_RPM = (float)(ptr->Velocity_RPM);
    }

    if (ptr->ReductionRatio > 1e-6f)
        ptr->OutputVel_RadPS = ptr->Velocity_RPM * 0.10471975511965f / ptr->ReductionRatio;

    ptr->Current = (int16_t)(aData[4] << 8 | aData[5]) * 20.0f / 16384.0f;
    ptr->Temperature = aData[6];
    ptr->Torque = TORQUE_CONSTANT * (1 / REDUCTION_RATIO) * ptr->Current;
    ptr->Velocity_Rad = ptr->Velocity_RPM / 9.55f;
    ptr->Velocity_RPM = lpf * (float)(ptr->Velocity_RPM) + (1.0 - lpf) * ptr->last_Velocity_RPM;

    if (ptr->RawAngle - ptr->last_angle > 4096)
        ptr->round_cnt--;
    else if (ptr->RawAngle - ptr->last_angle < -4096)
        ptr->round_cnt++;

    ptr->Angle = loop_float_constrain(ptr->RawAngle - ptr->zero_offset, -4095, 4096);
    ptr->AngleInDegree = ptr->Angle * 0.0439507f;
    ptr->total_angle = ptr->round_cnt * 8192 + ptr->RawAngle - ptr->offset_angle;
    ptr->last_angle = ptr->RawAngle;
    ptr->last_Velocity_RPM = (float)(ptr->Velocity_RPM);
}

void get_moto_offset(Motor_t *ptr, uint8_t *aData)
{
    ptr->RawAngle = (uint16_t)(aData[0] << 8 | aData[1]);
    ptr->offset_angle = ptr->RawAngle;
    ptr->last_angle = ptr->RawAngle;
}

static HAL_StatusTypeDef Send_Motor_Current(CAN_HandleTypeDef *_hcan, uint16_t std_id,
                                            int16_t c1, int16_t c2, int16_t c3, int16_t c4)
{
    uint8_t can_send_data[8];

    can_send_data[0] = (uint8_t)(c1 >> 8);
    can_send_data[1] = (uint8_t)c1;
    can_send_data[2] = (uint8_t)(c2 >> 8);
    can_send_data[3] = (uint8_t)c2;
    can_send_data[4] = (uint8_t)(c3 >> 8);
    can_send_data[5] = (uint8_t)c3;
    can_send_data[6] = (uint8_t)(c4 >> 8);
    can_send_data[7] = (uint8_t)c4;

    return CAN_Send_Data(_hcan, std_id, can_send_data, 8);
}

HAL_StatusTypeDef Send_Motor_Current_1_4(CAN_HandleTypeDef *_hcan,
                                         int16_t c1, int16_t c2, int16_t c3, int16_t c4)
{
    return Send_Motor_Current(_hcan, CAN_Transmit_1_4_ID, c1, c2, c3, c4);
}

HAL_StatusTypeDef Send_Motor_Current_5_8(CAN_HandleTypeDef *_hcan,
                                         int16_t c1, int16_t c2, int16_t c3, int16_t c4)
{
    return Send_Motor_Current(_hcan, CAN_Transmit_5_8_ID, c1, c2, c3, c4);
}
