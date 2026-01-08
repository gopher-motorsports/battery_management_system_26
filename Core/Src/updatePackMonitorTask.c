/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "updatePackMonitorTask.h"
#include "packMonitorTelemetry.h"
#include "packData.h"
#include <stdio.h>
#include <math.h>

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define HV_DIV_GAIN                     247.0f
#define LINK_DIV_GAIN                   483.35f

// Shunt characteristics
#define SHUNT_REF_RESISTANCE_NANO_OHMS  78000
#define SHUNT_REF_TEMP_C                25.0f
#define SHUNT_RESISTANCE_GAIN_UOHM      0.005f

// Windowed (256 samples @ 1 kHz) conversion time measurement with 0.5 Hz IIR low-pass filtering
#define CONV_COUNT_IIR_FILTER       553
#define CONV_UPPER_BOUND            1500
#define CONV_LOWER_BOUND            500

// Mapping of pack monitor voltage inputs
#define SHUNT_TEMP1_INDEX       1
#define PRECHARGE_TEMP_INDEX    2
#define LINK_PLUS_DIV_INDEX     3
#define SHUNT_TEMP2_INDEX       4
#define LINK_MINUS_DIV_INDEX    5
#define REF_1P25_INDEX          6
#define DISCHARGE_TEMP_INDEX    7

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

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

static void calculatePackParameters(ADBMS_PackMonitorData* packMonitorData, packMonitorTask_S* taskData)
{
    static uint32_t conversionCounterSum = 0;
    static uint32_t lastConversionCounter = 0;
    
    uint32_t deltaConversions = (packMonitorData->flagGroup.conversionCounter1 - lastConversionCounter) & (0x1FFF);

    if((deltaConversions == 0) || (packMonitorData->statGroup.currentAdc1Initialized == 0))
    {
        conversionCounterSum = 0;
        lastConversionCounter = packMonitorData->flagGroup.conversionCounter1;
        packMonitorData->convCountTimer = 0;
    }
    else
    {
        conversionCounterSum += deltaConversions;
        lastConversionCounter = packMonitorData->flagGroup.conversionCounter1;

        if(conversionCounterSum >= 1024)
        {
            uint32_t conversionTimeRaw = (packMonitorData->convCountTimer * 4) / conversionCounterSum;
            conversionCounterSum = 0;
            packMonitorData->convCountTimer = 0;
            taskData->conversionTime_us = (((conversionTimeRaw * CONV_COUNT_IIR_FILTER) + (taskData->conversionTime_us * (1000 - CONV_COUNT_IIR_FILTER))) / 1000);
            
            if(taskData->conversionTime_us > CONV_UPPER_BOUND)
            {
                taskData->conversionTime_us = CONV_UPPER_BOUND;
            }
            else if(taskData->conversionTime_us < CONV_LOWER_BOUND)
            {
                taskData->conversionTime_us = CONV_LOWER_BOUND;
            }
        }

        static uint8_t accCount = 0;
        static int32_t milliCoulombCounter = 0;

        accCount += deltaConversions;
        if(accCount >= 16)
        {
            accCount %= 16;
            if(abs(packMonitorData->currentAdcAccumulator1_uV) >= 10)
            {
                int32_t picoVoltSeconds = -1 * packMonitorData->currentAdcAccumulator1_uV * taskData->conversionTime_us;
                milliCoulombCounter += picoVoltSeconds / SHUNT_REF_RESISTANCE_NANO_OHMS;
            }
            
        }

        printf("Battery Current: %f A\n", ((float)(-1000 * packMonitorData->currentAdc1_uV)) / ((float)(SHUNT_REF_RESISTANCE_NANO_OHMS)));
        printf("MilliCoulombCounter: %li\n\n", milliCoulombCounter);
    }
    
}

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void initUpdatePackMonitorTask()
{
    // Set CS high upon start up
    HAL_GPIO_WritePin(PACK_MON_CS_N_GPIO_Port, PACK_MON_CS_N_Pin, GPIO_PIN_SET);

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
        // taskData.packCurrent = packMonitorData.currentAdc1_uV / SHUNT_REF_RESISTANCE_UOHM;
        // taskData.packVoltage = packMonitorData.batteryVoltage1 * HV_DIV_GAIN;
        // taskData.packPower = taskData.packCurrent * taskData.packVoltage;

        // taskData.shuntTemp1 = lookup(packMonitorData.voltageAdc[SHUNT_TEMP1_INDEX], &packMonTempTable);
        // taskData.prechargeTemp = lookup(packMonitorData.voltageAdc[PRECHARGE_TEMP_INDEX], &packMonTempTable);
        // taskData.dischargeTemp = lookup(packMonitorData.voltageAdc[DISCHARGE_TEMP_INDEX], &packMonTempTable);

        // taskData.linkVoltage = (packMonitorData.voltageAdc[LINK_PLUS_DIV_INDEX] - packMonitorData.voltageAdc[LINK_MINUS_DIV_INDEX]) * LINK_DIV_GAIN;

        // taskData.shuntResistanceMicroOhms = SHUNT_REF_RESISTANCE_UOHM + SHUNT_RESISTANCE_GAIN_UOHM * (taskData.shuntTemp1 - SHUNT_REF_TEMP_C);

        calculatePackParameters(&packMonitorData, &taskData);
    }

    static uint8_t counter = 0;

    if(++counter > 8)
    {
        // printf("\e[1;1H\e[2J");
        Debug("Battery Current: %f A\n", taskData.packCurrent);
        Debug("Battery Voltage: %f V\n", taskData.packVoltage);
        Debug("Power: %f W\n", taskData.packPower);
        Debug("Shunt Temp: %f C\n", taskData.shuntTemp1);
        Debug("Precharge Temp: %f C\n", taskData.prechargeTemp);
        Debug("Discharge Temp: %f C\n", taskData.dischargeTemp);
        Debug("Link Voltage: %f V\n", taskData.linkVoltage);
        Debug("Shunt Resistance: %f uOhms\n", taskData.shuntResistanceMicroOhms);
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
