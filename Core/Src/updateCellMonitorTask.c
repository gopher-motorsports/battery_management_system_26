/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "updateCellMonitorTask.h"
#include "taskStatistics.h"
#include "alerts.h"
#include <stdio.h>
#include "GopherCAN.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define FORCE_BALANCING_ON      0

#define NUM_CELL_TEMP_ADCS      7
#define BOARD_TEMP_ADC_INDEX    7
#define REG_TEMP_ADC_INDEX      8

#define ALLOWED_TEMP_VARIATION_C   20.0f

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

CHAIN_INFO_S chainInfo;

static ADBMS_CellMonitorData cellMonitorData[NUM_CELL_MON];

static cellMonitorTaskData_S taskData;

cellMonitorTaskData_S publicCellMonitorTaskData;

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

static TRANSACTION_STATUS_E updateBalancingState(ADBMS_CellMonitorData* cellMonitorData, cellMonitorTaskData_S* taskData);

static void runCellMonitorAlertMonitor(cellMonitorTaskData_S* taskData);

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

static TRANSACTION_STATUS_E updateBalancingState(ADBMS_CellMonitorData* cellMonitorData, cellMonitorTaskData_S* taskData)
{
    TRANSACTION_STATUS_E status = TRANSACTION_SUCCESS;

    static float floor = MAX_CELL_VOLTAGE;

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

    taskData->balancingEnabled = (FORCE_BALANCING_ON || forceEnableBalancing_state.data);

    return status;
}

static void runCellMonitorAlertMonitor(cellMonitorTaskData_S* taskData)
{
    // Accumulate alert statuses
    bool responseStatus[NUM_ALERT_RESPONSES] = {false};

    uint32_t numAlertsSet = 0;

    for(uint32_t i = 0; i < NUM_CELL_MONITOR_ALERTS; i++)
    {
        Alert_S* alert = cellMonitorAlerts[i];

        // Check alert condition and run alert monitor
        alert->alertConditionPresent = cellMonitorAlertConditionArray[i](taskData);
        runAlertMonitor(alert);

        // Get alert status and set response
        const AlertStatus_E alertStatus = getAlertStatus(alert);
        if((alertStatus == ALERT_SET) || (alertStatus == ALERT_LATCHED))
        {
            // Iterate through all alert responses and set them
            for (uint32_t j = 0; j < alert->numAlertResponse; j++)
            {
                const AlertResponse_E response = alert->alertResponse[j];
                // Set the alert response to active
                responseStatus[response] = true;
            }

            numAlertsSet++;

            // Bit encoding for GopherCAN
            uint8_t bitIndex  = alert->gcanAlertEncode;
            taskData->cellMonitorGcanAlerts |= (1U << bitIndex);
        }
        else 
        {
            // Bit encoding for GopherCAN
            uint8_t bitIndex = alert->gcanAlertEncode;
            taskData->cellMonitorGcanAlerts &= ~(1U << bitIndex);
        }
    }
    bmsFaultByTask.cellMonitorBmsFault = responseStatus[BMS_FAULT];
    setBmsFault();
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

    if((telemetryStatus == TRANSACTION_SUCCESS) || (telemetryStatus == TRANSACTION_CHAIN_BREAK_ERROR))
    {
        // Filter and assign all voltages to task data struct
        for(uint32_t i = 0; i < NUM_CELL_MON; i++)
        {
            for(uint32_t j = 0; j < NUM_CELLS_PER_CELL_MONITOR; j++)
            {
                taskData.cellMonitor[i].cellVoltage[j] = cellMonitorData[i].cellVoltage[j];

                // TODO: Could also add check for battery current to limit bounds more
                if((taskData.cellMonitor[i].cellVoltage[j] > MAX_CELL_VOLTAGE_LIMIT) || (taskData.cellMonitor[i].cellVoltage[j] < MIN_CELL_VOLTAGE_LIMIT))
                {
                    taskData.cellMonitor[i].cellVoltageStatus[j] = BAD;
                }
                else
                {
                    taskData.cellMonitor[i].cellVoltageStatus[j] = GOOD;
                }
            }
        }

        uint32_t tempArrayIndex = 0;
        float tempArray[NUM_SERIES_CELLS];
        static uint32_t cellTempStatusInitialized = 0;

        // Filter and assign all cell temps and board temps
        for(uint32_t i = 0; i < NUM_CELL_MON; i++)
        {
            // Cell indexes are offset depending on the mux state, which is set by gpio10
            uint32_t cellOffset = cellMonitorData[i].configGroupA.gpo10State;

            // Cell temps
            for(uint32_t j = 0; j < NUM_CELL_TEMP_ADCS; j++)
            {
                // Use lookup table to calculate temperature
                float cellTemp = lookup(cellMonitorData[i].auxVoltage[j], &cellTempTable);
                taskData.cellMonitor[i].cellTemp[(j * 2) + cellOffset] = cellTemp;

                // Add each temperature to array for filtering
                if(tempArrayIndex < NUM_SERIES_CELLS)
                {
                    tempArray[tempArrayIndex++] = cellTemp;
                }
            }

            float boardTemp = lookup(cellMonitorData[i].auxVoltage[BOARD_TEMP_ADC_INDEX], &cellTempTable);
            if(cellMonitorData[i].configGroupA.gpo10State == 0)
            {
                taskData.cellMonitor[i].boardTemp1 = boardTemp;
                taskData.cellMonitor[i].boardTemp1Status = GOOD;
            }
            else if(cellMonitorData[i].configGroupA.gpo10State == 1)
            {
                taskData.cellMonitor[i].boardTemp2 = boardTemp;
                taskData.cellMonitor[i].boardTemp2Status = GOOD;
            }

            taskData.cellMonitor[i].regTemp = lookup(cellMonitorData[i].auxVoltage[REG_TEMP_ADC_INDEX], &cellTempTable);
            taskData.cellMonitor[i].regTempStatus = GOOD;

            taskData.cellMonitor[i].dieTemp = cellMonitorData[i].statusGroupA.dieTemp;
            taskData.cellMonitor[i].dieTempStatus = GOOD;
        }

        // Calculate median temperature to eliminate outlier temp readings due to cold solder joints
        // Runs twice upon start up, then does not run again
        if(cellTempStatusInitialized < 3)
        {
            sort(tempArray, tempArrayIndex);
            float median = tempArray[tempArrayIndex / 2];

            // Set sensor status to BAD if the reading is an outlier
            for(uint32_t i = 0; i < NUM_CELL_MON; i++)
            {
                // Cell indexes are offset depending on the mux state, which is set by gpio10
                uint32_t cellOffset = cellMonitorData[i].configGroupA.gpo10State;

                for(uint32_t j = 0; j < NUM_CELL_TEMP_ADCS; j++)
                {
                    float cellTemp = taskData.cellMonitor[i].cellTemp[(j * 2) + cellOffset];
                
                    if(fabsf(cellTemp - median) > ALLOWED_TEMP_VARIATION_C)
                    {
                        taskData.cellMonitor[i].cellTempStatus[(j * 2) + cellOffset] = BAD;
                    }
                    else
                    {
                        taskData.cellMonitor[i].cellTempStatus[(j * 2) + cellOffset] = GOOD;
                    }
                }
            }

            cellTempStatusInitialized++;
        }

        updateBatteryStatistics(&taskData);
    }

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

    // Update cell monitor status
    if(chainInfo.chainStatus == MULTIPLE_CHAIN_BREAK)
    {
        uint32_t numCellMonitorA = chainInfo.availableDevices[PORTA];
        uint32_t numCellMonitorB = chainInfo.availableDevices[PORTB];

        for(uint32_t i = 0; i < NUM_CELL_MON; i++)
        {
            if((i < numCellMonitorA) || (i >= (NUM_CELL_MON - numCellMonitorB)))
            {
                taskData.cellMonitorStatus[i] = GOOD;
            }
            else
            {
                taskData.cellMonitorStatus[i] = BAD;
            }
        }
    }
    else
    {
        for(uint32_t i = 0; i < NUM_CELL_MON; i++)
        {
            taskData.cellMonitorStatus[i] = GOOD;
        }
    }

    // Regardless of whether or not chain initialized, run alert monitor
    runCellMonitorAlertMonitor(&taskData);

    // Copy task data to public struct
    vTaskSuspendAll();
    publicCellMonitorTaskData = taskData;
    xTaskResumeAll();

}
