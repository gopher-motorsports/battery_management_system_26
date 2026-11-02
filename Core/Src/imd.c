#include "imd.h"
#include "debug.h"
#include "GopherCAN.h"

extern CAN_HandleTypeDef hcan1;

#define IMD_REQUEST_CAN_ID  0x18EAFF17   // J1939 Request PGN, to all, from 0x17

static uint8_t nextPgn = 0x02;

static void requestImd(uint8_t pgnLow)
{
    CAN_TxHeaderTypeDef TxHeader = {0};
    uint8_t TxData[3] = { pgnLow, 0xFF, 0x00 };   // PGN 0x00FF0x, low byte first
    uint32_t TxMailbox;

    TxHeader.ExtId = IMD_REQUEST_CAN_ID;
    TxHeader.IDE = CAN_ID_EXT;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.DLC = 3;
    TxHeader.TransmitGlobalTime = DISABLE;

    // keep GopherCAN's TX interrupt from grabbing a mailbox at the same moment
    HAL_CAN_DeactivateNotification(&hcan1, CAN_IT_TX_MAILBOX_EMPTY);
    HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_TX_MAILBOX_EMPTY);

    if (status != HAL_OK)
    {
        Debug("CAN1 IMD request failed: status=%lu err=0x%08lX free=%lu\r\n",
              (uint32_t)status, HAL_CAN_GetError(&hcan1),
              HAL_CAN_GetTxMailboxesFreeLevel(&hcan1));
    }
}

void initImdTask(void) {
    nextPgn = 0x02;
}

void runImdTask(void) {
    requestImd(nextPgn);

    if (nextPgn == 4) {
        nextPgn = 2;
    } else {
        nextPgn++;
    }            
}