/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */
#include "main.h"
#include "debug.h"
#include "charger.h"
#include <math.h>

/* ==================================================================== */
/* ======================= EXTERNAL VARIABLES ========================= */
/* ==================================================================== */

extern CAN_HandleTypeDef hcan1;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

/*!
  @brief   Send a CAN message to the charger
  @param   voltageRequest - Charger Voltage Request
  @param   currentRequest - Charger Current Request
  @param   enable - Enable/Disable Request
*/
// void sendChargerMessage(float voltageRequest, float currentRequest, bool enable)
// {
//     // Create template for EXT frame CAN message 
//     CAN_TxHeaderTypeDef TxHeader;
//     uint8_t TxData[8] = {0};            // Default data frame is all zeros
//     uint32_t TxMailbox;

//     TxHeader.IDE = CAN_ID_EXT;          // Extended CAN ID
//     TxHeader.ExtId = CHARGER_CAN_ID_TX; // Charger CAN ID
//     TxHeader.RTR = CAN_RTR_DATA;        // Sending data frame
//     TxHeader.DLC = 8;                   // 8 Bytes of data

//     if(enable)
//     {        
//         // Encode voltage and current requests
//         // [0] Voltage*10 HIGH Byte
//         // [1] Voltage*10 LOW Byte
//         // [2] Current*10 HIGH Byte
//         // [3] Current*10 LOW Byte
//         uint16_t deciVolts = voltageRequest * 10;
//         uint16_t deciAmps = currentRequest * 10;

//         TxData[0] = (uint8_t)(deciVolts >> 8);  
//         TxData[1] = (uint8_t)(deciVolts);
//         TxData[2] = (uint8_t)(deciAmps >> 8);  
//         TxData[3] = (uint8_t)(deciAmps);
//     }
//     else
//     {
//         // Disable Charging
//         TxData[4] = 1; // TODO Validate this disables charging
//     }

//     // Send CAN message to Charger
//     if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
//     {
//         Debug("Failed to send message to charger\n");
//     }
// }

void sendChargerMessage(float voltageRequest, float currentRequest, bool enable)
{
    CAN_TxHeaderTypeDef TxHeader = {0};
    uint8_t TxData[8] = {0};
    uint32_t TxMailbox;

    TxHeader.StdId = 0;
    TxHeader.ExtId = CHARGER_CAN_ID_TX & 0x1FFFFFFFU;
    TxHeader.IDE = CAN_ID_EXT;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.DLC = 8;
    TxHeader.TransmitGlobalTime = DISABLE;

    if (enable)
    {
        uint16_t deciVolts = (uint16_t)(voltageRequest * 10.0f);
        uint16_t deciAmps  = (uint16_t)(currentRequest * 10.0f);

        TxData[0] = (uint8_t)(deciVolts >> 8);
        TxData[1] = (uint8_t)(deciVolts);
        TxData[2] = (uint8_t)(deciAmps >> 8);
        TxData[3] = (uint8_t)(deciAmps);
    }
    else
    {
        TxData[4] = 1;
    }

    HAL_StatusTypeDef status =
        HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);

    if (status != HAL_OK)
    {
        Debug("CAN1 charger TX failed: status=%lu err=0x%08lX esr=0x%08lX free=%lu\r\n",
              (uint32_t)status,
              HAL_CAN_GetError(&hcan1),
              hcan1.Instance->ESR,
              HAL_CAN_GetTxMailboxesFreeLevel(&hcan1));
    }
}
