/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "updateCellMonitorTask.h"
#include "taskStatistics.h"
#include <stdio.h>

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define FORCE_BALANCING_ON      0

#define NUM_CELL_TEMP_ADCS      7

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

CHAIN_INFO_S chainInfo;

static ADBMS_CellMonitorData cellMonitorData[NUM_CELL_MON];

static cellMonitorTaskData_S taskData;

cellMonitorTaskData_S publicCellMonitorTaskData;

static TRANSACTION_STATUS_E updateBalancingState(ADBMS_CellMonitorData* cellMonitorData, cellMonitorTaskData_S* taskData)
{
    TRANSACTION_STATUS_E status = TRANSACTION_SUCCESS;

    static float floor = MAX_BRICK_VOLTAGE;

    if(taskData->balancingEnabled)
    {
        for(uint16_t i = 0; i < NUM_CELL_MON; i++)
        {
            for(uint16_t j = 0; j < NUM_CELLS_PER_CELL_MONITOR; j++)
            {
                if(taskData->cellMonitor[i].cellVoltage[j] > floor)
                {
                    cellMonitorData[i].dischargePWM[j] = 50.0f;
                }
            }

            cellMonitorData[i].configGroupB.dischargeTimeoutMinutes = 1;
        }      

        static uint32_t lastBalancingUpdate = 0;

        if((HAL_GetTick() - lastBalancingUpdate) > 1000)
        {
            lastBalancingUpdate = HAL_GetTick();

            status = updateCellBalancing(&chainInfo, cellMonitorData);
        }
    }
    else
    {
        floor = taskData->minCellVoltage + 0.001f;
        for(uint16_t i = 0; i < NUM_CELL_MON; i++)
        {
            cellMonitorData[i].configGroupB.dischargeTimeoutMinutes = 0;
        }
    }

    taskData->balancingEnabled = FORCE_BALANCING_ON;

    return status;
}

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void initUpdateCellMonitorTask()
{
    // Set both CS high upon start up
    HAL_GPIO_WritePin(PORTA_CS_GPIO_Port, PORTA_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PORTB_CS_GPIO_Port, PORTB_CS_Pin, GPIO_PIN_SET);

    // Disable balancing until we have the first minCellVoltage reading to set the floor
    taskData.balancingEnabled = 0;

    // TODO: Alerts
    HAL_GPIO_WritePin(BMS_FAULT_GPIO_Port, BMS_FAULT_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BMS_INB_N_GPIO_Port, BMS_INB_N_Pin, GPIO_PIN_SET);

}

void runUpdateCellMonitorTask()
{
    TRANSACTION_STATUS_E telemetryStatus = updateCellTelemetry(&chainInfo, cellMonitorData);

    if(telemetryStatus == TRANSACTION_CHAIN_BREAK_ERROR)
    {
        Debug("Chain Break!\n");
    }
    else if(telemetryStatus == TRANSACTION_SPI_ERROR)
    {
        Debug("SPI Failure!\n");
    }
    else if(telemetryStatus == TRANSACTION_POR_ERROR)
    {
        Debug("Failed to correct power on reset error!\n");
    }
    else if(telemetryStatus == TRANSACTION_COMMAND_COUNTER_ERROR)
    {
        Debug("Persistent Command Counter Error!\n");
    }

    // Filter and assign all voltages to task data struct
    for(uint32_t i = 0; i < NUM_CELL_MON; i++)
    {
        for(uint32_t j = 0; j < NUM_CELLS_PER_CELL_MONITOR; j++)
        {
            // Add filtering here
            taskData.cellMonitor[i].cellVoltage[j] = cellMonitorData[i].cellVoltage[j];
            taskData.cellMonitor[i].cellVoltageStatus[j] = GOOD;

            if((taskData.cellMonitor[i].cellVoltage[j] > MAX_BRICK_VOLTAGE) || (taskData.cellMonitor[i].cellVoltage[j] < 2.5f))
            {
                taskData.cellMonitor[i].cellVoltageStatus[j] = BAD;
            }
            else
            {
                taskData.cellMonitor[i].cellVoltageStatus[j] = GOOD;
            }
        }
    }

    // Filter and assign all cell temps and board temps
    for(uint32_t i = 0; i < NUM_CELL_MON; i++)
    {
        // Cell indexes are offset depending on the mux state, which is set by gpio10
        uint32_t cellOffset = cellMonitorData[i].configGroupA.gpo10State;

        // Cell temps
        for(uint32_t j = 0; j < NUM_CELL_TEMP_ADCS; j++)
        {
            float cellTemp = lookup(cellMonitorData[i].auxVoltage[j], &cellTempTable);
            taskData.cellMonitor[i].cellTemp[(j * 2) + cellOffset] = cellTemp;

            if(fequals(cellTemp, MIN_TEMP_SENSOR_VALUE_C) || fequals(cellTemp, MAX_TEMP_SENSOR_VALUE_C))
            {
                taskData.cellMonitor[i].cellTempStatus[(j * 2) + cellOffset] = BAD;
            }
            else
            {
                taskData.cellMonitor[i].cellTempStatus[(j * 2) + cellOffset] = GOOD;
            }
        }

        float boardTemp = lookup(cellMonitorData[i].auxVoltage[8], &cellTempTable);
        if(cellMonitorData[i].configGroupA.gpo10State == 0)
        {
            taskData.cellMonitor[i].boardTemp1 = boardTemp;
        }
        else if(cellMonitorData[i].configGroupA.gpo10State == 1)
        {
            taskData.cellMonitor[i].boardTemp2 = boardTemp;
        }
    }

    updateBatteryStatistics(&taskData);

    telemetryStatus = updateBalancingState(cellMonitorData, &taskData);

    if(telemetryStatus == TRANSACTION_CHAIN_BREAK_ERROR)
    {
        Debug("Chain Break!\n");
    }
    else if(telemetryStatus == TRANSACTION_SPI_ERROR)
    {
        Debug("SPI Failure!\n");
    }
    else if(telemetryStatus == TRANSACTION_POR_ERROR)
    {
        Debug("Failed to correct power on reset error!\n");
    }
    else if(telemetryStatus == TRANSACTION_COMMAND_COUNTER_ERROR)
    {
        Debug("Persistent Command Counter Error!\n");
    }

    // Copy task data to public struct
    vTaskSuspendAll();
    publicCellMonitorTaskData = taskData;
    xTaskResumeAll();

}
