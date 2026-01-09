/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "updateCellMonitorTask.h"
#include "cellMonitorTelemetry.h"
#include "adbms/adbmsCellMonitor.h"
#include "packData.h"
#include <stdio.h>

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define NUM_CELL_TEMP_ADCS      7

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

CHAIN_INFO_S chainInfo;

static ADBMS_CellMonitorData cellMonitorData;

static cellMonitorTask_S taskData;

uint8_t txBuffer[6] = {0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF};

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void initUpdateCellMonitorTask()
{
    // Set both CS high upon start up
    HAL_GPIO_WritePin(PORTA_CS_GPIO_Port, PORTA_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PORTB_CS_GPIO_Port, PORTB_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(BMS_FAULT_GPIO_Port, BMS_FAULT_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BMS_INB_N_GPIO_Port, BMS_INB_N_Pin, GPIO_PIN_SET);


}

void runUpdateCellMonitorTask()
{
    TRANSACTION_STATUS_E telemetryStatus = updateCellTelemetry(&chainInfo, &cellMonitorData);

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

    // Assign all cell voltages
    for(uint8_t i = 0; i < NUM_CELLS_PER_CELL_MONITOR; i++)
    {
        taskData.cellVoltage[i] = cellMonitorData.cellVoltage[i];
    }

    // Filter and assign all cell temps
    // Cell indexes are offset depending on the mux state, which is set by gpio10
    uint32_t cellOffset = cellMonitorData.configGroupA.gpo10State;

    for(uint32_t j = 0; j < NUM_CELL_TEMP_ADCS; j++)
    {
        float cellTemp = lookup(cellMonitorData.auxVoltage[j], &cellMonTempTable);
        taskData.cellTemp[(j * 2) + cellOffset] = cellTemp;
    }

    float boardTemp = lookup(cellMonitorData.auxVoltage[8], &cellMonTempTable);
    if(cellMonitorData.configGroupA.gpo10State == 0)
    {
        taskData.boardTemp1 = boardTemp;
    }
    else if(cellMonitorData.configGroupA.gpo10State == 1)
    {
        taskData.boardTemp2 = boardTemp;
    }

    for(uint8_t i = 0; i < NUM_CELLS_PER_CELL_MONITOR; i++)
    {
        printf("Cell Voltage %u: %f V\n", i, taskData.cellVoltage[i]);
        printf("Cell Temp %u: %f C\n", i, taskData.cellTemp[i]);
        printf("Board Temp 1: %f\n", taskData.boardTemp1);
        printf("Board Temp 2: %f\n", taskData.boardTemp2);
    }

}
