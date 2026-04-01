/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "main.h"
#include "statusUpdateTask.h"
#include "alerts.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define HEARTBEAT_BLINK_MS      500

#define CLEAR_ON_START_MS       15000

/* ==================================================================== */
/* ======================= EXTERNAL VARIABLES ========================= */
/* ==================================================================== */

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim3;

volatile uint32_t adcRawValue;
volatile uint32_t adcNewDataFlag;

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

static shutdownCircuitStatus_S shutdownCircuitData;

shutdownCircuitStatus_S publicShutdownCircuitData;

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

static void updateHeartbeat();
static void updateSdcStatus(shutdownCircuitStatus_S *shutdownCircuitData);
static void runStatusAlertMonitor(shutdownCircuitStatus_S *shutdownCircuitData);

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

static void updateHeartbeat()
{
    static uint32_t lastHeartBeatUpdate = 0;

    if((HAL_GetTick() - lastHeartBeatUpdate) > HEARTBEAT_BLINK_MS)
    {
        HAL_GPIO_TogglePin(MCU_HEART_GPIO_Port, MCU_HEART_Pin);
        lastHeartBeatUpdate = HAL_GetTick();
    }
}

static void updateSdcStatus(shutdownCircuitStatus_S *shutdownCircuitData)
{
    shutdownCircuitData->imdLatchOpen = ((HAL_GPIO_ReadPin(IMD_FAULT_READ_GPIO_Port, IMD_FAULT_READ_Pin)) && (HAL_GetTick() > CLEAR_ON_START_MS));
    shutdownCircuitData->bmsLatchOpen = ((HAL_GPIO_ReadPin(BMS_FAULT_READ_GPIO_Port, BMS_FAULT_READ_Pin)) && (HAL_GetTick() > CLEAR_ON_START_MS));
    shutdownCircuitData->bmsInhibitActive = HAL_GPIO_ReadPin(BMS_INB_N_GPIO_Port, BMS_INB_N_Pin) ^ 1; // TODO
    shutdownCircuitData->sdcStatusI2BInterlock = HAL_GPIO_ReadPin(SDC1_GPIO_Port, SDC1_Pin);
    shutdownCircuitData->sdcStatusTBInterlock = HAL_GPIO_ReadPin(SDC2_GPIO_Port, SDC2_Pin);

    if(adcNewDataFlag)
    {
        adcNewDataFlag = 0;
        shutdownCircuitData->shutdownEndVoltage_V = (adcRawValue / 4095.0f) * 3.3f;
        printf("Voltage: %f\n", shutdownCircuitData->shutdownEndVoltage_V);
    }
}

// static void runStatusAlertMonitor(shutdownCircuitStatus_S *shutdownCircuitData)
// {
//     // Accumulate alert statuses
//     bool responseStatus[NUM_ALERT_RESPONSES] = {false};

//     for(int32_t i = 0; i < NUM_STATUS_ALERTS; i++)
//     {
//         Alert_S* alert = statusAlerts[i];

//         // Check alert condition and run alert monitor
//         alert->alertConditionPresent = statusAlertConditionArray[i](shutdownCircuitData);
//         runAlertMonitor(alert);

//         // Get alert status and set response
//         const AlertStatus_E alertStatus = getAlertStatus(alert);
//         if((alertStatus == ALERT_SET) || (alertStatus == ALERT_LATCHED))
//         {
//             // Iterate through all alert responses and set them
//             for (uint32_t j = 0; j < alert->numAlertResponse; j++)
//             {
//                 const AlertResponse_E response = alert->alertResponse[j];
//                 // Set the alert response to active
//                 responseStatus[response] = true;
//             }
//         }
//     }
//     setAmsFault(responseStatus[AMS_FAULT]);
// }

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void initStatusUpdateTask()
{
    HAL_GPIO_WritePin(MCU_HEART_GPIO_Port, MCU_HEART_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MCU_FAULT_GPIO_Port, MCU_FAULT_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MCU_GSENSE_GPIO_Port, MCU_GSENSE_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BMS_INB_N_GPIO_Port, BMS_INB_N_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(BMS_FAULT_GPIO_Port, BMS_FAULT_Pin, GPIO_PIN_RESET); // TODO: Should fault be asserted initially?

    HAL_ADC_Start_IT(&hadc1);
}

void runStatusUpdateTask()
{
    // Update hearbeat led
    updateHeartbeat();

    // Update shutdown circuit data
    updateSdcStatus(&shutdownCircuitData);

    // TODO: Run alert monitor
    // runStatusAlertMonitor(&statusUpdateTaskDataLocal);

    // Copy task data to public struct
    vTaskSuspendAll();
    publicShutdownCircuitData = shutdownCircuitData;
    xTaskResumeAll();
}