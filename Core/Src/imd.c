#include "imd.h"
#include "debug.h"
#include "GopherCAN.h"
#include "Gopher_Sense.h" 

extern CAN_HandleTypeDef hcan1;
extern GENERAL_PARAMETER param_list[NUM_CAN_PARAMS];

#define IMD_REQUEST_CAN_ID  0x18EAFF17   // J1939 Request PGN, to all, from 0x17

static uint8_t nextPgn = 0x02;

/* ---------------- Requests (BMS -> IMD) ---------------- */

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
/* ---------------- Forwarding (BMS -> logger via Gopher Sense) ---------------- */

// Mark one GopherCAN parameter so Gopher Sense sends its group
static void markSendNeeded(CAN_INFO_STRUCT* info)
{
    for (uint32_t i = 0; i < NUM_CAN_PARAMS; i++)
    {
        if (param_list[i].can_param == info)
        {
            param_list[i].status = SEND_NEEDED;
            return;
        }
    }
}

// Called by GopherCAN right after each IMD group is decoded.
static void sendImdGeneral(void)         { markSendNeeded(&isoCorrected.info); }   // group 0x37
static void sendImdIsolationDetail(void) { markSendNeeded(&isoNeg.info); }         // group 0x38
static void sendImdVoltage(void)         { markSendNeeded(&HVSystem.info); }       // group 0x39
static void sendImdItSystem(void)        { markSendNeeded(&capacityVal.info); }    // group 0x3A

/* ---------------- Task ---------------- */

void initImdTask(void)
{
    nextPgn = 0x02;

    attach_callback_std(0x37, sendImdGeneral);   // store it so it send every time group decoded now
    attach_callback_std(0x38, sendImdIsolationDetail);
    attach_callback_std(0x39, sendImdVoltage);
    attach_callback_std(0x3A, sendImdItSystem);
}

void runImdTask(void) {
    requestImd(nextPgn);

    if (nextPgn == 4) {
        nextPgn = 2;
    } else {
        nextPgn++;
    }            
}