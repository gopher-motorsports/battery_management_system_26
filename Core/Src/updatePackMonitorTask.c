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

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

static void calculatePackParameters(ADBMS_PackMonitorData* packMonitorData, packMonitorTask_S* taskData);

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

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

        // printf("Battery Current: %f A\n", ((float)(-1000 * packMonitorData->currentAdc1_uV)) / ((float)(SHUNT_REF_RESISTANCE_NANO_OHMS)));
        // printf("MilliCoulombCounter: %li\n\n", milliCoulombCounter);
    }
    
}

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void initUpdatePackMonitorTask()
{
    // Set CS high upon start up
    HAL_GPIO_WritePin(PACK_MON_CS_N_GPIO_Port, PACK_MON_CS_N_Pin, GPIO_PIN_SET);

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
        taskData.packVoltage = packMonitorData.batteryVoltage1 * HV_DIV_GAIN;
        taskData.packPower = taskData.packCurrent * taskData.packVoltage;

        taskData.shuntTemp1 = lookup(packMonitorData.voltageAdc[SHUNT_TEMP1_INDEX], &packMonTempTable);
        taskData.prechargeTemp = lookup(packMonitorData.voltageAdc[PRECHARGE_TEMP_INDEX], &packMonTempTable);
        taskData.dischargeTemp = lookup(packMonitorData.voltageAdc[DISCHARGE_TEMP_INDEX], &packMonTempTable);

        taskData.linkVoltage = (packMonitorData.voltageAdc[LINK_PLUS_DIV_INDEX] - packMonitorData.voltageAdc[LINK_MINUS_DIV_INDEX]) * LINK_DIV_GAIN;

        if((taskData.linkVoltage < (0.93f * taskData.packVoltage)) || (taskData.linkVoltage < 10.0f))
        {
            HAL_GPIO_WritePin(PRECHARGE_DONE_GPIO_Port, PRECHARGE_DONE_Pin, GPIO_PIN_RESET);
        } else 
        {
            HAL_GPIO_WritePin(PRECHARGE_DONE_GPIO_Port, PRECHARGE_DONE_Pin, GPIO_PIN_SET);
        }

        // taskData.shuntResistanceMicroOhms = SHUNT_REF_RESISTANCE_UOHM + SHUNT_RESISTANCE_GAIN_UOHM * (taskData.shuntTemp1 - SHUNT_REF_TEMP_C);

        calculatePackParameters(&packMonitorData, &taskData);
    }

    static uint8_t counter = 0;

    if(++counter > 8)
    {
        printf("\e[1;1H\e[2J");
        Debug("Battery Current: %f A\n", taskData.packCurrent);
        Debug("Battery Voltage: %f V\n", taskData.packVoltage);
        Debug("Power: %f W\n", taskData.packPower);
        Debug("Shunt Temp: %f C\n", taskData.shuntTemp1);
        Debug("Precharge Temp: %f C\n", taskData.prechargeTemp);
        Debug("Discharge Temp: %f C\n", taskData.dischargeTemp);
        Debug("Link Voltage: %f V\n", taskData.linkVoltage);
        counter = 0;
    }
    
}
