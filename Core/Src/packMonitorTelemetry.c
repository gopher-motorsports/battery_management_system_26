/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "packMonitorTelemetry.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define NUM_COMMAND_BLOCK_RETRYS    3

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

static TRANSACTION_STATUS_E runPackMonitorCommandBlock(TRANSACTION_STATUS_E (*telemetryFunction)(CHAIN_INFO_S*, ADBMS_PackMonitorData*), CHAIN_INFO_S* chainInfoData, ADBMS_PackMonitorData* packMonitorData);

static TRANSACTION_STATUS_E initPackMonitor(CHAIN_INFO_S* chainInfoData, ADBMS_PackMonitorData* packMonitorData);

static TRANSACTION_STATUS_E startNewReadCycle(CHAIN_INFO_S* chainInfoData, ADBMS_PackMonitorData* packMonitorData);

static TRANSACTION_STATUS_E readPackAdcs(CHAIN_INFO_S* chainInfoData, ADBMS_PackMonitorData* packMonitorData);

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

static TRANSACTION_STATUS_E runPackMonitorCommandBlock(TRANSACTION_STATUS_E (*telemetryFunction)(CHAIN_INFO_S*, ADBMS_PackMonitorData*), CHAIN_INFO_S* chainInfoData, ADBMS_PackMonitorData* packMonitorData)
{
    for(uint32_t attempt = 0; attempt < NUM_COMMAND_BLOCK_RETRYS; attempt++)
    {
        // Run the telemetry function
        TRANSACTION_STATUS_E status = telemetryFunction(chainInfoData, packMonitorData);

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

static TRANSACTION_STATUS_E initPackMonitor(CHAIN_INFO_S* chainInfoData, ADBMS_PackMonitorData* packMonitorData)
{
    activatePort(chainInfoData, TIME_WAKE_US);

    TRANSACTION_STATUS_E status;

    if(chainInfoData->chainStatus != CHAIN_COMPLETE)
    {
        status = updateChainStatus(chainInfoData);
        // Ignore all errors except for spi, since the response to all other errors is to reinitialize the chain
        if(status == TRANSACTION_SPI_ERROR)
        {
            return status;
        }
    }

    status = clearPackMonitorFlags(chainInfoData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }    

    packMonitorData->configGroupA.gpo1HighZMode = 0;
    packMonitorData->configGroupA.gpo1State = 1;

    status = writePackMonitorConfigA(chainInfoData, packMonitorData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    status = startAdcConversions(chainInfoData, REDUNDANT_MODE, CONTINUOUS_MEASUREMENT);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    return readPackMonitorSerialId(chainInfoData, packMonitorData);

}

static TRANSACTION_STATUS_E startNewReadCycle(CHAIN_INFO_S* chainInfoData, ADBMS_PackMonitorData* packMonitorData)
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

    return readPackMonitorSerialId(chainInfoData, packMonitorData);
}

static TRANSACTION_STATUS_E readPackAdcs(CHAIN_INFO_S* chainInfoData, ADBMS_PackMonitorData* packMonitorData)
{
    TRANSACTION_STATUS_E status = readFlagRegister(chainInfoData, packMonitorData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    if(packMonitorData->flagGroup.resetDetected)
    {
        return TRANSACTION_POR_ERROR;
    }

    status = readPrimaryAdcs(chainInfoData, packMonitorData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    status = readPrimaryAccumulators(chainInfoData, packMonitorData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    status = readVoltageAdcs(chainInfoData, packMonitorData);
    if((status != TRANSACTION_SUCCESS) && (status != TRANSACTION_CHAIN_BREAK_ERROR))
    {
        return status;
    }

    return readAuxiliaryVoltages(chainInfoData, packMonitorData);
}

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

TRANSACTION_STATUS_E updatePackTelemetry(CHAIN_INFO_S* chainInfoData, ADBMS_PackMonitorData* packMonitorData)
{
    TRANSACTION_STATUS_E telemetryStatus;

    static bool initialized = false;

    if(initialized)
    {
        telemetryStatus = runPackMonitorCommandBlock(startNewReadCycle, chainInfoData, packMonitorData);

        if((telemetryStatus == TRANSACTION_SUCCESS) || (telemetryStatus == TRANSACTION_CHAIN_BREAK_ERROR))
        {
            telemetryStatus = runPackMonitorCommandBlock(updatePackTelemetry, chainInfoData, packMonitorData);
        }

    }

    if((!initialized) || (telemetryStatus == TRANSACTION_POR_ERROR))
    {
        Debug("Initializing chain...\n");

        telemetryStatus = runPackMonitorCommandBlock(initPackMonitor, chainInfoData, packMonitorData);

        if((telemetryStatus == TRANSACTION_SUCCESS) || (telemetryStatus == TRANSACTION_CHAIN_BREAK_ERROR))
        {
            Debug("Chain initialization successful!\n");
            initialized = true;
        }
        else
        {
            Debug("Chain failed to initialize!\n");
            initialized = false;
        }
    }

    return telemetryStatus;
}
