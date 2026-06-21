/**
 ******************************************************************************
 * @file    bsp_CAN.c
 * @author  Wang Hongxi
 * @version V2.1
 * @date    2026/06/17
 * @brief   CAN通信初始化、接收解析与业务协议发送（精简版：仅底盘电机+遥控器）
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/

#include "bsp_CAN.h"
#include "bsp_dwt.h"
#include "user_lib.h"
#include "chassis_task.h"
#include "remote_control.h"
#include "detect_task.h"

/* Private macros ------------------------------------------------------------*/

#define CAN_TX_WAIT_MAX 10000U

/* Private variables ---------------------------------------------------------*/

CAN_ErrorInfo_t CAN1_ErrorInfo = {0};
CAN_ErrorInfo_t CAN2_ErrorInfo = {0};

/* Private function declarations ---------------------------------------------*/

static HAL_StatusTypeDef CAN_Config_Filter(CAN_HandleTypeDef *_hcan, uint32_t filter_bank)
{
    CAN_FilterTypeDef can_filter_st = {0};

    can_filter_st.FilterActivation = ENABLE;
    can_filter_st.FilterMode = CAN_FILTERMODE_IDMASK;
    can_filter_st.FilterScale = CAN_FILTERSCALE_32BIT;
    can_filter_st.FilterIdHigh = 0x0000;
    can_filter_st.FilterIdLow = 0x0000;
    can_filter_st.FilterMaskIdHigh = 0x0000;
    can_filter_st.FilterMaskIdLow = 0x0000;
    can_filter_st.FilterBank = filter_bank;
    can_filter_st.FilterFIFOAssignment = CAN_RX_FIFO0;
    can_filter_st.SlaveStartFilterBank = 14;

    return HAL_CAN_ConfigFilter(_hcan, &can_filter_st);
}

static CAN_ErrorInfo_t *CAN_Get_ErrorInfo(CAN_HandleTypeDef *_hcan)
{
    if (_hcan == &hcan1)
        return &CAN1_ErrorInfo;
    if (_hcan == &hcan2)
        return &CAN2_ErrorInfo;
    return NULL;
}

/* Function prototypes -------------------------------------------------------*/

HAL_StatusTypeDef CAN_Send_Data(CAN_HandleTypeDef *_hcan, uint16_t std_id, uint8_t *data, uint8_t len)
{
    CAN_TxHeaderTypeDef tx_header = {0};
    uint32_t send_mail_box = 0;
    uint32_t count = 0;
    HAL_StatusTypeDef ret;
    CAN_ErrorInfo_t *err = CAN_Get_ErrorInfo(_hcan);

    if (_hcan == NULL || data == NULL || len > 8U)
        return HAL_ERROR;

    while (!((_hcan->State == HAL_CAN_STATE_READY) || (_hcan->State == HAL_CAN_STATE_LISTENING)))
    {
        if (++count > CAN_TX_WAIT_MAX)
        {
            if (err != NULL)
                err->tx_timeout++;
            return HAL_TIMEOUT;
        }
    }

    count = 0;
    while (HAL_CAN_GetTxMailboxesFreeLevel(_hcan) == 0U)
    {
        if (++count > CAN_TX_WAIT_MAX)
        {
            if (err != NULL)
            {
                err->tx_timeout++;
                err->last_error = HAL_CAN_GetError(_hcan);
            }
            HAL_CAN_AbortTxRequest(_hcan,
                                   CAN_TX_MAILBOX0 | CAN_TX_MAILBOX1 | CAN_TX_MAILBOX2);
            return HAL_TIMEOUT;
        }
    }

    tx_header.StdId = std_id;
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = len;

    ret = HAL_CAN_AddTxMessage(_hcan, &tx_header, data, &send_mail_box);
    if (ret != HAL_OK && err != NULL)
    {
        err->tx_error++;
        err->last_error = HAL_CAN_GetError(_hcan);
    }

    return ret;
}

void CAN_Device_Init(void)
{
    if (CAN_Config_Filter(&hcan1, 0) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_CAN_Start(&hcan1) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
    {
        Error_Handler();
    }

    if (CAN_Config_Filter(&hcan2, 14) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_CAN_Start(&hcan2) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief HAL库CAN接收FIFO0中断
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *_hcan)
{
    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];
    static uint8_t RC_Data_Buf[16];

    HAL_CAN_GetRxMessage(_hcan, CAN_RX_FIFO0, &rx_header, rx_data);

    if (_hcan == &hcan1)
    {
       
        if (rx_header.StdId >= 0x201 && rx_header.StdId <= 0x204)
        {
            if (Chassis.ChassisMotor[rx_header.StdId - 0x201].msg_cnt++ <= 50)
            {
                get_moto_offset(&Chassis.ChassisMotor[rx_header.StdId - 0x201], rx_data);
            }
            else
            {
                get_moto_info(&Chassis.ChassisMotor[rx_header.StdId - 0x201], rx_data);
            }
            switch (rx_header.StdId)
            {
            case 0x201: Detect_Hook(CHASSIS_MOTOR1_TOE); break;
            case 0x202: Detect_Hook(CHASSIS_MOTOR2_TOE); break;
            case 0x203: Detect_Hook(CHASSIS_MOTOR3_TOE); break;
            case 0x204: Detect_Hook(CHASSIS_MOTOR4_TOE); break;
            }
        }
    }

    if (_hcan == &hcan2)
    {
        switch (rx_header.StdId)
        {
        case CAN_RC_DATA_Frame_0:
            RC_Data_Buf[0] = rx_data[0];
            RC_Data_Buf[1] = rx_data[1];
            RC_Data_Buf[2] = rx_data[2];
            RC_Data_Buf[3] = rx_data[3];
            RC_Data_Buf[4] = rx_data[4];
            RC_Data_Buf[5] = rx_data[5];
            RC_Data_Buf[6] = rx_data[6];
            RC_Data_Buf[7] = rx_data[7];
            break;
        case CAN_RC_DATA_Frame_1:
            RC_Data_Buf[8] = rx_data[0];
            RC_Data_Buf[9] = rx_data[1];
            RC_Data_Buf[10] = rx_data[2];
            RC_Data_Buf[11] = rx_data[3];
            RC_Data_Buf[12] = rx_data[4];
            RC_Data_Buf[13] = rx_data[5];
            RC_Data_Buf[14] = rx_data[6];
            RC_Data_Buf[15] = rx_data[7];
            Callback_RC_Handle(&remote_control, RC_Data_Buf);
            break;
        }
    }
}

void Send_RC_Data(CAN_HandleTypeDef *_hcan, uint8_t *rc_data)
{
    uint8_t can_send_data[8];

    if (rc_data == NULL)
        return;

    for (uint8_t i = 0; i < 8U; i++)
        can_send_data[i] = rc_data[i];
    CAN_Send_Data(_hcan, CAN_RC_DATA_Frame_0, can_send_data, 8);

    for (uint8_t i = 0; i < 8U; i++)
        can_send_data[i] = rc_data[i + 8U];
    CAN_Send_Data(_hcan, CAN_RC_DATA_Frame_1, can_send_data, 8);
}
