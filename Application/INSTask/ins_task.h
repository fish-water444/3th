/**
 ******************************************************************************
 * @file    ins_task.h
 * @brief   INS任务 — 从百零一移植(简化版，无PWM温控)
 ******************************************************************************
 */
#ifndef __INS_TASK_H
#define __INS_TASK_H

#include "stdint.h"
#include "main.h"

#define INS_TASK_PERIOD 1

#define INS_GetTimeline HAL_GetTick

enum
{
  X = 0,
  Y = 1,
  Z = 2,
};

void INS_Init(void);
void INS_Task(void);

#endif
