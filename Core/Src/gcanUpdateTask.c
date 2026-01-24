/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "gcanUpdateTask.h"
#include "main.h"
#include "updateCellMonitorTask.h"
#include "updatePackMonitorTask.h"
#include "alerts.h"
#include "gopher_sense.h"
#include "GopherCAN.h"
#include "cmsis_os.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

// Frequency group update periods millisecs
#define HIGH_FREQ_UPDATE_PERIOD         10
#define MEDIUM_FREQ_UPDATE_PERIOD       100
#define LOW_FREQ_UPDATE_PERIOD          1000

/* ==================================================================== */
/* ============================== STRUCTS ============================= */
/* ==================================================================== */

typedef struct
{
    cellMonitorTaskData_S cellMonitorTaskData;
    packMonitorTaskData_S packMonitorTaskData;
} gcanTaskInputData_S;

/* ==================================================================== */
/* ======================= EXTERNAL VARIABLES ========================= */
/* ==================================================================== */

extern CAN_HandleTypeDef hcan2;

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

void updateHighFrequencyVariables(gcanTaskInputData_S* gcanData);
void updateMediumFrequencyVariables(gcanTaskInputData_S* gcanData);
void updateLowFrequencyVariables(gcanTaskInputData_S* gcanData, uint32_t segmentIndex);

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

void updateHighFrequencyVariables(gcanTaskInputData_S* gcanData)
{
    // Pack current
    update_and_queue_param_float(&bmsBatteryCurrent_A, gcanData->packMonitorTaskData.packCurrent);

    // Pack voltage
    update_and_queue_param_float(&bmsBatteryVoltage_V, gcanData->packMonitorTaskData.packVoltage);

    // Link voltage
    update_and_queue_param_float(&bmsTractiveSystemVoltage_V, gcanData->packMonitorTaskData.linkVoltage);

    // Flags
//     update_and_queue_param_u8(&imdFault_state, gcanData->statusUpdateTaskData.shutdownCircuitData.imdLatchOpen);
//     update_and_queue_param_u8(&amsFault_state, gcanData->statusUpdateTaskData.shutdownCircuitData.bmsLatchOpen);
}

void updateMediumFrequencyVariables(gcanTaskInputData_S* gcanData)
{
    // Pack statistics
    update_and_queue_param_float(&maxCellVoltage_V, gcanData->cellMonitorTaskData.maxCellVoltage);
    update_and_queue_param_float(&minCellVoltage_V, gcanData->cellMonitorTaskData.minCellVoltage);
    update_and_queue_param_float(&avgCellVoltage_V, gcanData->cellMonitorTaskData.avgCellVoltage);
    update_and_queue_param_float(&cellImbalance_mV, gcanData->cellMonitorTaskData.cellImbalance * 1000.0f);
    update_and_queue_param_float(&minCellTemp_C, gcanData->cellMonitorTaskData.minCellTemp);
    update_and_queue_param_float(&avgCellTemp_C, gcanData->cellMonitorTaskData.avgCellTemp);
    update_and_queue_param_float(&maxCellTemp_C, gcanData->cellMonitorTaskData.maxCellTemp);
    update_and_queue_param_float(&maxBoardTemp_C, gcanData->cellMonitorTaskData.maxBoardTemp);
    update_and_queue_param_float(&minBoardTemp_C, gcanData->cellMonitorTaskData.minBoardTemp);
    update_and_queue_param_float(&avgBoardTemp_C, gcanData->cellMonitorTaskData.avgBoardTemp);

}

void updateLowFrequencyVariables(gcanTaskInputData_S* gcanData, uint32_t segmentIndex)
{
    
}

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void initGcanUpdateTask()
{
    
}

void runGcanUpdateTask()
{
    gcanTaskInputData_S gcanTaskInputData;
    vTaskSuspendAll();
    gcanTaskInputData.cellMonitorTaskData = publicCellMonitorTaskData;
    gcanTaskInputData.packMonitorTaskData = publicPackMonitorTaskData;
    xTaskResumeAll();

    // High frequency update variables - 100Hz
    static uint32_t lastHighFreqUpdateTick = 0;
    if((HAL_GetTick() - lastHighFreqUpdateTick) >= HIGH_FREQ_UPDATE_PERIOD)
    {
        lastHighFreqUpdateTick = HAL_GetTick();
        updateHighFrequencyVariables(&gcanTaskInputData);
    }

    // Medium frequency update variables - 10Hz
    static uint32_t lastMediumFreqUpdateTick = 0;
    if((HAL_GetTick() - lastMediumFreqUpdateTick) >= MEDIUM_FREQ_UPDATE_PERIOD)
    {
        lastMediumFreqUpdateTick = HAL_GetTick();
        updateMediumFrequencyVariables(&gcanTaskInputData);
    }

    // Low frequency update variables - 1Hz
    // static uint32_t lastLowFreqUpdateTick = 0;
    // if((HAL_GetTick() - lastLowFreqUpdateTick) >= LOW_FREQ_LOGGING_DELAY)
    // {
    //     lastLowFreqUpdateTick = HAL_GetTick();

    //     static uint32_t segmentIndex = 0;
    //     updateLowFrequencyVariables(&gcanTaskInputData, segmentIndex);

    //     segmentIndex++;
    //     if(segmentIndex >= NUM_CELL_MON)
    //     {
    //         segmentIndex = 0;
    //     }
    // }

    // Update gcan tx
    service_can_tx(&hcan2);

    // Update gcan rx
    service_can_rx_buffer();

}
