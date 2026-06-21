/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "can.h"
#include "iwdg.h"
#include "remote_control.h"
#include "chassis_task.h"
#include "bsp_CAN.h"
#include "detect_task.h"
#include "ins_task.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
/* USER CODE END Variables */
osThreadId ChassisTaskHandle;
osThreadId DetectTaskHandle;
osThreadId INSTaskHandle;

/* Private function prototypes -----------------------------------------------*/
void StartChassisTask(void const *argument);      /* 底盘任务 */
void StartDetectTask(void const *argument);       /* 离线检测任务 */
void StartINSTask(void const *argument);          /* IMU姿态任务 */

void MX_FREERTOS_Init(void);

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize);

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize);

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* USER CODE BEGIN GET_TIMER_TASK_MEMORY */
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize)
{
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
  *ppxTimerTaskStackBuffer = &xTimerStack[0];
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
/* USER CODE END GET_TIMER_TASK_MEMORY */

void MX_FREERTOS_Init(void)
{
  /* definition and creation of INSTask */
  osThreadDef(INSTask, StartINSTask, osPriorityAboveNormal, 0, 512);
  INSTaskHandle = osThreadCreate(osThread(INSTask), NULL);

  /* definition and creation of DetectTask */
  osThreadDef(DetectTask, StartDetectTask, osPriorityBelowNormal, 0, 512);
  DetectTaskHandle = osThreadCreate(osThread(DetectTask), NULL);

  /* definition and creation of ChassisTask */
  osThreadDef(ChassisTask, StartChassisTask, osPriorityAboveNormal, 0, 512);
  ChassisTaskHandle = osThreadCreate(osThread(ChassisTask), NULL);
}

void StartINSTask(void const *argument)
{
  INS_Init();
  for (;;)
  {
    INS_Task();
    osDelay(INS_TASK_PERIOD);
  }
}

void StartDetectTask(void const *argument)
{
  Detect_Task_Init();
  for (;;)
  {
    Detect_Task();
    osDelay(DETECT_TASK_PERIOD);
  }
}

void StartChassisTask(void const *argument)
{
  osDelay(1000);
  Chassis_Init();

  for (;;)
  {
    Chassis_Control();
    osDelay(CHASSIS_TASK_PERIOD);
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
