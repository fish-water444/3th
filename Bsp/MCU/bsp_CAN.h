/**
 ******************************************************************************
 * @file     bsp_CAN.h
 * @author   Wang Hongxi
 * @version  V2.1
 * @brief    CAN通信初始化、接收解析与业务协议发送（精简版）
 ******************************************************************************
 */
#ifndef _BSP_CAN_H
#define _BSP_CAN_H

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include "can.h"

/* Exported macros -----------------------------------------------------------*/

#define CAN_RC_DATA_Frame_0 0x131
#define CAN_RC_DATA_Frame_1 0x132

/* Exported types ------------------------------------------------------------*/

typedef struct
{
    volatile uint32_t tx_timeout;
    volatile uint32_t tx_error;
    volatile uint32_t bus_off;
    volatile uint32_t last_error;
} CAN_ErrorInfo_t;

/* Exported variables ---------------------------------------------------------*/

extern CAN_ErrorInfo_t CAN1_ErrorInfo;
extern CAN_ErrorInfo_t CAN2_ErrorInfo;

/* Exported function declarations ---------------------------------------------*/

void CAN_Device_Init(void);

HAL_StatusTypeDef CAN_Send_Data(CAN_HandleTypeDef *_hcan, uint16_t std_id, uint8_t *data, uint8_t len);

void Send_RC_Data(CAN_HandleTypeDef *_hcan, uint8_t *rc_data);

#endif
