/**
 ******************************************************************************
 * @file    filter32.h
 * @author  Wang Hongxi
 * @brief   First-order low-pass filter (minimal)
 ******************************************************************************
 */
#ifndef _FILTER32_H
#define _FILTER32_H

#include "stdint.h"

typedef struct
{
    float Input;
    float Last_Input;

    float Output;
    float Last_Output;

    float RC;    // RC = 1/omegac
    float K_RC;  // K_RC = dt / (RC + dt)
    float alpha; // 滤波系数
} First_Order_Filter_t;

void First_Order_Filter_Init(First_Order_Filter_t *filter, float RC, float dt);
float First_Order_Filter_Calculate(First_Order_Filter_t *filter, float input, float dt);

#endif
