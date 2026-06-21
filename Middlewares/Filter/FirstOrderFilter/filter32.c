/**
 ******************************************************************************
 * @file    filter32.c
 * @author  Wang Hongxi
 * @brief   First-order low-pass filter implementation (minimal)
 ******************************************************************************
 */
#include "filter32.h"

void First_Order_Filter_Init(First_Order_Filter_t *filter, float RC, float dt)
{
    filter->RC = RC;
    filter->K_RC = dt / (RC + dt);
    filter->alpha = 1.0f - filter->K_RC;
    filter->Input = 0;
    filter->Last_Input = 0;
    filter->Output = 0;
    filter->Last_Output = 0;
}

float First_Order_Filter_Calculate(First_Order_Filter_t *filter, float input, float dt)
{
    filter->K_RC = dt / (filter->RC + dt);
    filter->alpha = 1.0f - filter->K_RC;
    filter->Last_Input = filter->Input;
    filter->Input = input;
    filter->Last_Output = filter->Output;
    filter->Output = filter->Last_Output * filter->alpha + filter->Input * filter->K_RC;
    return filter->Output;
}
