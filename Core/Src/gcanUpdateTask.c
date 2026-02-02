/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "gcanUpdateTask.h"
#include "gcanUtils.h"
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

#define LOW_FREQ_LOGGING_DELAY          (LOW_FREQ_UPDATE_PERIOD / NUM_CELL_MON)

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
void updateLowFrequencyVariables(gcanTaskInputData_S* gcanData, uint32_t cellMonitorIndex);

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
//     update_and_queue_param_u8(&bmsFault_state, gcanData->statusUpdateTaskData.shutdownCircuitData.bmsLatchOpen);
}

void updateMediumFrequencyVariables(gcanTaskInputData_S* gcanData)
{
    // Pack statistics
    // TODO: Add soc and soe here
    update_and_queue_param_float(&maxCellVoltage_V, gcanData->cellMonitorTaskData.maxCellVoltage);
    update_and_queue_param_float(&minCellVoltage_V, gcanData->cellMonitorTaskData.minCellVoltage);

    update_and_queue_param_float(&avgCellVoltage_V, gcanData->cellMonitorTaskData.avgCellVoltage);
    update_and_queue_param_float(&cellImbalance_mV, gcanData->cellMonitorTaskData.cellImbalance * 1000.0f);
    update_and_queue_param_float(&maxCellTemp_C, gcanData->cellMonitorTaskData.maxCellTemp);
    update_and_queue_param_float(&minCellTemp_C, gcanData->cellMonitorTaskData.minCellTemp);

    update_and_queue_param_float(&avgCellTemp_C, gcanData->cellMonitorTaskData.avgCellTemp);
    update_and_queue_param_float(&maxBoardTemp_C, gcanData->cellMonitorTaskData.maxBoardTemp);
    update_and_queue_param_float(&minBoardTemp_C, gcanData->cellMonitorTaskData.minBoardTemp);
    update_and_queue_param_float(&avgBoardTemp_C, gcanData->cellMonitorTaskData.avgBoardTemp);

    // update_and_queue_param_u8(&bmsNumActiveAlerts_info,);
    update_and_queue_param_u16(&cellMonitorAlerts_info, gcanData->cellMonitorTaskData.cellMonitorGcanAlerts);
    // update_and_queue_param_u8(&packMonitorAlerts_info, );
    // update_and_queue_param_u8(&forceEnableBalancing_state, );
    // update_and_queue_param_u8(&bmsInhibitActive_state, );
}

void updateLowFrequencyVariables(gcanTaskInputData_S* gcanData, uint32_t cellMonitorIndex)
{
    // Log all segment variables
    // for(uint32_t i = 0; i < NUM_CELL_MON; i++)
    // {
        // for(uint32_t j = 0; j < NUM_CELLS_PER_CELL_MONITOR; j++)
        // {
        //     update_and_queue_param_float(cellVoltageParams[cellMonitorIndex][j], gcanData->telemetryTaskData.bmb[cellMonitorIndex].cellVoltage[j]);
        //     update_and_queue_param_float(cellTempParams[cellMonitorIndex][j], gcanData->telemetryTaskData.bmb[cellMonitorIndex].cellTemp[j]);
        // }

        update_and_queue_param_float(cellStatParams[cellMonitorIndex][0], gcanData->cellMonitorTaskData.cellMonitor[cellMonitorIndex].maxCellVoltage);
        update_and_queue_param_float(cellStatParams[cellMonitorIndex][1], gcanData->cellMonitorTaskData.cellMonitor[cellMonitorIndex].minCellVoltage);
        update_and_queue_param_float(cellStatParams[cellMonitorIndex][2], gcanData->cellMonitorTaskData.cellMonitor[cellMonitorIndex].avgCellVoltage);
        // update_and_queue_param_float(cellStatParams[cellMonitorIndex][3], gcanData->cellMonitorTaskData.cellMonitor[cellMonitorIndex].dieTemp);
        update_and_queue_param_float(cellStatParams[cellMonitorIndex][4], gcanData->cellMonitorTaskData.cellMonitor[cellMonitorIndex].maxCellTemp);
        update_and_queue_param_float(cellStatParams[cellMonitorIndex][5], gcanData->cellMonitorTaskData.cellMonitor[cellMonitorIndex].minCellTemp);
        update_and_queue_param_float(cellStatParams[cellMonitorIndex][6], gcanData->cellMonitorTaskData.cellMonitor[cellMonitorIndex].avgCellTemp);
        // update_and_queue_param_float(cellStatParams[cellMonitorIndex][7], gcanData->cellMonitorTaskData.cellMonitor[cellMonitorIndex].boardTemp);
    // }
}

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void initGcanUpdateTask()
{
    // Initially set all param statuses to no send needed, so that they only send if the value changes
    gsense_reset();
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
    static uint32_t lastLowFreqUpdateTick = 0;
    if((HAL_GetTick() - lastLowFreqUpdateTick) >= LOW_FREQ_LOGGING_DELAY)
    {
        lastLowFreqUpdateTick = HAL_GetTick();

        static uint32_t cellMonitorIndex = 0;
        updateLowFrequencyVariables(&gcanTaskInputData, cellMonitorIndex);

        cellMonitorIndex++;
        if(cellMonitorIndex >= NUM_CELL_MON)
        {
            cellMonitorIndex = 0;
        }
    }

    // Update gcan tx
    service_can_tx(&hcan2);

    // Update gcan rx
    service_can_rx_buffer();

}
