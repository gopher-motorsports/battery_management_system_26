/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "updatePackMonitorTask.h"
#include "packMonitorTelemetry.h"
#include <stdio.h>

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define HV_DIV_GAIN                     247.0f
#define LINK_DIV_GAIN                   483.35f

#define SHUNT_REF_RESISTANCE_UOHM       78.0f

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

extern SPI_HandleTypeDef hspi2;

PORT_INSTANCE_S packMonPort = {
    .spiHandle = &hspi2,
    .csPort = PACK_MON_CS_N_GPIO_Port,
    .csPin = PACK_MON_CS_N_Pin
};

static CHAIN_INFO_S packMonInfo;

static ADBMS_PackMonitorData packMonitorData;

static packMonitorTask_S taskData;

/* TEMP LOCAL FUNCTIONS */

void printPackMonitor(ADBMS_PackMonitorData* packMonitor)
{
    printf("==== ADBMS2950 Pack Monitor ====\n");

    /* ---------------- Current ADCs ---------------- */
    printf("Current ADCs:\n");
    printf("  I1: %ld uV\n", (long)packMonitor->currentAdc1_uV);
    printf("  I2: %ld uV\n", (long)packMonitor->currentAdc2_uV);

    /* ---------------- Battery Voltage ADCs ---------------- */
    printf("Battery Voltage Adcs:\n");
    printf("  VBAT1: %f V\n", packMonitor->batteryVoltage1);
    printf("  VBAT2: %f V\n", packMonitor->batteryVoltage1);
    printf("  Battery Voltage: %f V\n", (packMonitor->batteryVoltage1 * HV_DIV_GAIN));

    /* ---------------- Voltage ADCs ---------------- */
    printf("Voltage ADCs:\n");
    for (uint16_t i = 0; i < NUM_VOLTAGE_ADC; i++)
    {
        printf("  V%u: %.3f V\n", (i + 1), packMonitor->voltageAdc[i]);
    }

    /* ---------------- Aux Voltages ---------------- */
    printf("Aux Voltages:\n");
    printf("  VREF:      %.3f V\n", packMonitor->referenceVoltage);
    printf("  VREF_RED:  %.3f V\n", packMonitor->redundantReferenceVoltage);
    printf("  VREF_1P25: %.3f V\n", packMonitor->referenceVoltage1P25);
    printf("  VREG:      %.3f V\n", packMonitor->vregPowerSupply);
    printf("  VDD:       %.3f V\n", packMonitor->vddPowerSupply);
    printf("  VDIG:      %.3f V\n", packMonitor->digitalSupply);
    printf("  EPAD:      %.3f V\n", packMonitor->exposedPadVoltage);
    printf("  VREF_DIV:  %.3f V\n", packMonitor->dividedReferenceVoltage);

    /* ---------------- Temperature Channels ---------------- */
    printf("Temperature Channels:\n");
    printf("  T_INT1:    %.2f C\n", packMonitor->primaryIntTemp);
    printf("  T_INT2:    %.2f C\n", packMonitor->secondaryIntTemp);

    printf("================================\n\n");
}

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void initUpdatePackMonitorTask()
{
    // Set CS high upon start up
    HAL_GPIO_WritePin(PACK_MON_CS_N_GPIO_Port, PACK_MON_CS_N_Pin, GPIO_PIN_SET);

    // Init chain to default values (this will become initChain function)
    packMonInfo.commPorts[PORTA] = packMonPort;
    packMonInfo.commPorts[PORTB] = packMonPort;
    packMonInfo.numDevs = 1;
    packMonInfo.currentPort = PORTA;
    packMonInfo.chainStatus = CHAIN_COMPLETE;

    // // Ready the device
    // activatePort(&packMonInfo, TIME_WAKE_US);

    // readPackMonitorConfigA(&packMonInfo, &packMonitor);
    // packMonitor.configGroupA.v4Reference = 1;
    // packMonitor.configGroupA.v6Reference = 1;
    // packMonitor.configGroupA.gpo1HighZMode = 0;
    // writePackMonitorConfigA(&packMonInfo, &packMonitor);

    // startAdcConversions(&packMonInfo, REDUNDANT_MODE, CONTINUOUS_MEASUREMENT);
    // startVoltageConversions(&packMonInfo, OPEN_WIRE_DISABLED, PACK_ALL_CHANNELS);
    // startAuxVoltageConversions(&packMonInfo);

}

void runUpdatePackMonitorTask()
{
    TRANSACTION_STATUS_E telemetryStatus = updatePackTelemetry(&packMonInfo, &packMonitorData);

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
        // Update task data
        taskData.packCurrent = packMonitorData.currentAdc1_uV / SHUNT_REF_RESISTANCE_UOHM;
        taskData.packVoltage = packMonitorData.batteryVoltage1 * HV_DIV_GAIN;
        taskData.packPower = taskData.packCurrent * taskData.packVoltage;
    }

    static uint8_t counter = 0;

    if(++counter > 8)
    {
        printf("\e[1;1H\e[2J");
        Debug("Battery Current: %f A\n", taskData.packCurrent);
        Debug("Battery Voltage: %f V\n", taskData.packVoltage);
        Debug("Power: %f W\n", taskData.packPower);
        counter = 0;
    }




    // Ready the device
    // activatePort(&packMonInfo, TIME_READY_US);



    // if(resetCount > 30)
    // {
    //     stop = 1;
    // }

    // clearPackMonitorVoltageRegisters(&packMonInfo);
    

    // updateChainStatus(&packMonInfo);

    // TRANSACTION_STATUS_E status = readPackMonitorSerialId(&packMonInfo, &packMonitor);
    
    // printf("Serial ID Transaction Status: %u\n", status);

    // for(uint8_t i = 0; i < REGISTER_SIZE_BYTES; i++)
    // {
    //     printf("Serial ID Byte [%u]: %X\n", i, packMonitor.serialId[i]);
    // }

    // static bool voltageAdcStarted = 0;

    // if(!voltageAdcStarted)
    // {
    //     status = startAdcConversions(&packMonInfo, REDUNDANT_MODE, CONTINUOUS_MEASUREMENT);
    //     voltageAdcStarted = 1;
    // }

    // status = readVoltageAdc1(&packMonInfo, &packMonitor);

    // status = startVoltageConversions(&packMonInfo, OPEN_WIRE_DISABLED, PACK_ALL_CHANNELS);

    // float linkPlusDivVoltage = packMonitor.voltageAdc[3];
    // float linkMinusDivVoltage = packMonitor.voltageAdc[5];

    // printf("Link+ Div Voltage: %f\n", linkPlusDivVoltage);
    // printf("Link- Div Voltage: %f\n", linkMinusDivVoltage);
    // // TODO: calculate using divider gain

    // float linkVoltage = (packMonitor.voltageAdc[3] - packMonitor.voltageAdc[5]) * LINK_DIV_GAIN;

    // printf("Link Voltage: %f\n", linkVoltage);

    
}
