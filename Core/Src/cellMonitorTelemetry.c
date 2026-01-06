/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "cellMonitorTelemetry.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define NUM_COMMAND_BLOCK_RETRYS    3

#define NUM_CELL_MON    10

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */



/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

static TRANSACTION_STATUS_E runCellMonitorCommandBlock(TRANSACTION_STATUS_E (*telemetryFunction)(CHAIN_INFO_S*, ADBMS_CellMonitorData*), CHAIN_INFO_S* chainInfoData, ADBMS_CellMonitorData* cellMonitorData)
{
    for(uint32_t attempt = 0; attempt < NUM_COMMAND_BLOCK_RETRYS; attempt++)
    {
        // Run the telemetry function
        TRANSACTION_STATUS_E status = telemetryFunction(chainInfoData, cellMonitorData);

        // Check return status
        if(status == TRANSACTION_COMMAND_COUNTER_ERROR)
        {
            // On command counter error, retry the command block
            Debug("Command counter mismatch! Retrying command block!\n");
        }
        else
        {
            // On all other errors, return error
            return status;
        }
    }

    // After the max function attempts, return command counter error
    return TRANSACTION_COMMAND_COUNTER_ERROR;
}

static TRANSACTION_STATUS_E initCellMonitor(CHAIN_INFO_S* chainInfoData, ADBMS_CellMonitorData* cellMonitorData)
{
    activatePort(chainInfoData, TIME_WAKE_US);

    TRANSACTION_STATUS_E status;

    // Reset chain info struct to default values
    chainInfoData->numDevs = NUM_CELL_MON;
    chainInfoData->chainStatus = MULTIPLE_CHAIN_BREAK;
    chainInfoData->availableDevices[PORTA] = NUM_CELL_MON;
    chainInfoData->availableDevices[PORTB] = NUM_CELL_MON;
    chainInfoData->currentPort = PORTA;
    chainInfoData->localCommandCounter = 0;

    if(chainInfoData->chainStatus != CHAIN_COMPLETE)
    {
        status = updateChainStatus(chainInfoData);
        // Ignore all errors except for spi, since the response to all other errors is to reinitialize the chain
        if(status == TRANSACTION_SPI_ERROR)
        {
            return status;
        }
    }

    // Set configuration for Group A

    for(uint32_t i = 0; i < NUM_CELL_MON; i++)
    {
        cellMonitorData[i].configGroupA.referenceOn = 1;
        cellMonitorData[i].configGroupA.digitalFilterSetting = FILTER_CUTOFF_10_HZ;
        cellMonitorData[i].configGroupA.gpo1State = 1;
        cellMonitorData[i].configGroupA.gpo2State = 1;
        cellMonitorData[i].configGroupA.gpo3State = 1;
        cellMonitorData[i].configGroupA.gpo4State = 1;
        cellMonitorData[i].configGroupA.gpo5State = 1;
        cellMonitorData[i].configGroupA.gpo6State = 1;
        cellMonitorData[i].configGroupA.gpo7State = 1;
        cellMonitorData[i].configGroupA.gpo8State = 1;
        cellMonitorData[i].configGroupA.gpo9State = 1;
        cellMonitorData[i].configGroupA.gpo10State = 0;
    }

    status = writeCellMonitorConfigA(chainInfoData, cellMonitorData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    status = startCellConversions(chainInfoData, NON_REDUNDANT_MODE, CONTINUOUS_MODE, DISCHARGE_DISABLED, FILTER_RESET, CELL_OPEN_WIRE_DISABLED);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    status = startAuxConversions(chainInfoData, AUX_ALL_CHANNELS, AUX_OPEN_WIRE_DISABLED);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    return readCellMonitorSerialId(chainInfoData, cellMonitorData);
}

static TRANSACTION_STATUS_E startNewCellReadCycle(CHAIN_INFO_S* chainInfoData, ADBMS_CellMonitorData* cellMonitorData)
{
    activatePort(chainInfoData, TIME_READY_US);

    TRANSACTION_STATUS_E status;

    // If the chain is broken in any way, attempt to correct the chain at the start of a new cycle
    // If a correction occurs, and a POR error is detected as a result, that will be returned by status
    if(chainInfoData->chainStatus != CHAIN_COMPLETE)
    {
        status = updateChainStatus(chainInfoData);
        // Ignore all errors except for spi, since the response to all other errors is to reinitialize the chain
        if((status == TRANSACTION_SPI_ERROR) || (status == TRANSACTION_POR_ERROR))
        {
            return status;
        }
    }

    // Unfreeze read registers
    status = unfreezeRegisters(chainInfoData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    // Freeze read registers for new read cycle
    status = freezeRegisters(chainInfoData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    return readCellMonitorSerialId(chainInfoData, cellMonitorData);
}

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

TRANSACTION_STATUS_E updateCellTelemetry(CHAIN_INFO_S* chainInfoData, ADBMS_CellMonitorData* cellMonitorData)
{

}
