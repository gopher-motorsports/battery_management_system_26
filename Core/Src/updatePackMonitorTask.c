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

// Coulomb counting
#define MAX_13BIT_UINT                  0x1FFF
#define PHASE_COUNTS_PER_CONVERSION     4
// Windowed (256 samples @ 1 kHz) conversion time measurement with 0.5 Hz IIR low-pass filtering
#define CONV_COUNT_IIR_FILTER           553
#define CONV_UPPER_BOUND                1500
#define CONV_LOWER_BOUND                500
#define PACK_MON_ACCN_SETTING           ACCUMULATE_4_SAMPLES
#define ACCUMULATION_REGISTER_COUNT     ((PACK_MON_ACCN_SETTING + 1) * 4)
#define MIN_VALID_IADC_READING          10 // microvolts

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

packMonitorTask_S publicPackMonitorTaskData;

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

static void calculatePackParameters(ADBMS_PackMonitorData* packMonitorData, packMonitorTask_S* taskData);

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

static void calculatePackParameters(ADBMS_PackMonitorData* packMonitorData, packMonitorTask_S* taskData)
{
    static uint32_t sumConversions = 0; // Accumulates conversions, used for updating conversion time
    static uint32_t lastConversionCounter = 0;
    
    uint32_t deltaConversions = (packMonitorData->flagGroup.conversionCounter1 - lastConversionCounter) & (MAX_13BIT_UINT);

    if((deltaConversions == 0) || (packMonitorData->statGroup.currentAdc1Initialized == 0))
    {
        lastConversionCounter = packMonitorData->flagGroup.conversionCounter1;
        sumConversions = 0;
        packMonitorData->convCountTimer_us = 0;
    }
    else
    {
        sumConversions += deltaConversions;
        lastConversionCounter = packMonitorData->flagGroup.conversionCounter1;

        // Update conversionTime_us at low frequency because accumulating more samples before dividing reduces integer rounding error (quantization error)
        // Each conversion is approx 1 ms, so this if statement should execute at approx 3.9 Hz
        if(sumConversions >= 1024)
        {
            // Conversion time = timer ticks / number of conversions
            uint32_t conversionTimeRaw = (packMonitorData->convCountTimer_us * PHASE_COUNTS_PER_CONVERSION) / sumConversions;
            sumConversions = 0;
            packMonitorData->convCountTimer_us = 0;

            // IIR low pass
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

        static uint8_t accConversions = 0; // Accumulates conversions, used for processing I1ACC results
        static int32_t milliCoulombCounter = 0;

        // Update milliCoulombCounter for every new I1ACC register value
        accConversions += deltaConversions;
        if(accConversions >= (ACCUMULATION_REGISTER_COUNT * PHASE_COUNTS_PER_CONVERSION))
        {
            accConversions %= (ACCUMULATION_REGISTER_COUNT * PHASE_COUNTS_PER_CONVERSION);
            if(abs(packMonitorData->currentAdcAccumulator1_uV) >= MIN_VALID_IADC_READING)
            {
                int32_t picoVoltSeconds = -1 * packMonitorData->currentAdcAccumulator1_uV * taskData->conversionTime_us;
                milliCoulombCounter += picoVoltSeconds / taskData->shuntResistance_nOhms;
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
        // Shunt uses same NTCs as cell temp sensors
        taskData.shuntTemp1 = lookup(packMonitorData.voltageAdc[SHUNT_TEMP1_INDEX], &cellTempTable);
        taskData.prechargeTemp = lookup(packMonitorData.voltageAdc[PRECHARGE_TEMP_INDEX], &prechargeDischargeTempTable);
        taskData.dischargeTemp = lookup(packMonitorData.voltageAdc[DISCHARGE_TEMP_INDEX], &prechargeDischargeTempTable);

        taskData.shuntResistance_nOhms = lroundf(lookup(packMonitorData.voltageAdc[SHUNT_TEMP1_INDEX], &shuntResistanceTable));

        taskData.packCurrent = packMonitorData.currentAdc1_uV * 1000 / taskData.shuntResistance_nOhms;
        taskData.packVoltage = packMonitorData.batteryVoltage1 * HV_DIV_GAIN;
        taskData.packPower = taskData.packCurrent * taskData.packVoltage;

        taskData.linkVoltage = (packMonitorData.voltageAdc[LINK_PLUS_DIV_INDEX] - packMonitorData.voltageAdc[LINK_MINUS_DIV_INDEX]) * LINK_DIV_GAIN;

        if((taskData.linkVoltage < (0.93f * taskData.packVoltage)) || (taskData.linkVoltage < 10.0f))
        {
            HAL_GPIO_WritePin(PRECHARGE_DONE_GPIO_Port, PRECHARGE_DONE_Pin, GPIO_PIN_RESET);
        } else 
        {
            HAL_GPIO_WritePin(PRECHARGE_DONE_GPIO_Port, PRECHARGE_DONE_Pin, GPIO_PIN_SET);
        }

        calculatePackParameters(&packMonitorData, &taskData);
    }

    // Copy task data to public struct
    vTaskSuspendAll();
    publicPackMonitorTaskData = taskData;
    xTaskResumeAll();
    
}
