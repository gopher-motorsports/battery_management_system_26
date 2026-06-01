/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "updatePackMonitorTask.h"
#include "packMonitorTelemetry.h"
#include "updateCellMonitorTask.h"
#include "packData.h"
#include "pid.h"
#include "alerts.h"
#include <stdio.h>
#include <stdlib.h>
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
#define ACCUMULATION_REGISTER_COUNT     ((PACK_MON_ACCN_SETTING + 1) * 4)
#define MIN_VALID_IADC_READING_UV       10
#define ACCUMULATED_CURRENT_THRES_UV    100

// Mapping of pack monitor voltage inputs
#define SHUNT_TEMP1_INDEX       1
#define PRECHARGE_TEMP_INDEX    2
#define LINK_PLUS_DIV_INDEX     3
#define SHUNT_TEMP2_INDEX       4
#define LINK_MINUS_DIV_INDEX    5
#define REF_1P25_INDEX          6
#define DISCHARGE_TEMP_INDEX    7

// Shutdown circuit and precharge control
#define SDC_END_V_DIV_GAIN      11.0f
#define SDC_END_V_THRESHOLD     10.0f

#define ADC_MAX_COUNTS          4095.0f
#define ADC_REF_VOLTAGE         3.3f

#define PRECHARGE_WINDOW_MS     3000

/* ==================================================================== */
/* ========================= ENUMERATED TYPES========================== */
/* ==================================================================== */

typedef enum
{
    OPEN_IR = 0,
    CLOSE_IR
} PositiveIRControl_E;

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

static CHAIN_INFO_S packMonInfo;

static ADBMS_PackMonitorData packMonitorData;

static packMonitorTaskData_S taskData;

static const float adcCountsToSdcVoltsGain = (ADC_REF_VOLTAGE / ADC_MAX_COUNTS) * SDC_END_V_DIV_GAIN;

/* ==================================================================== */
/* ========================= GLOBAL VARIABLES ========================= */
/* ==================================================================== */

packMonitorTaskData_S publicPackMonitorTaskData;

volatile uint32_t adcRawValue = 0;

/* ==================================================================== */
/* ======================= EXTERNAL VARIABLES ========================= */
/* ==================================================================== */

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim3;

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

static void calculatePackParameters(ADBMS_PackMonitorData* packMonitorData, packMonitorTaskData_S* taskData);

static void controlPositiveIR(PositiveIRControl_E irCommand);

static void updatePrechargeLogic(packMonitorTaskData_S* taskData);

static void runPackMonitorAlertMonitor(packMonitorTaskData_S* taskData);

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

static void calculatePackParameters(ADBMS_PackMonitorData* packMonitorData, packMonitorTaskData_S* taskData)
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

        // Update milliCoulombCounter for every new I1ACC register value
        accConversions += deltaConversions;
        if(accConversions >= (ACCUMULATION_REGISTER_COUNT * PHASE_COUNTS_PER_CONVERSION))
        {
            accConversions %= (ACCUMULATION_REGISTER_COUNT * PHASE_COUNTS_PER_CONVERSION);
            if(abs(packMonitorData->currentAdcAccumulator1_uV) >= MIN_VALID_IADC_READING_UV)
            {
                int32_t picoVoltSeconds = -1 * packMonitorData->currentAdcAccumulator1_uV * taskData->conversionTime_us;
                taskData->socData.milliCoulombCounter += picoVoltSeconds / taskData->shuntResistance_nOhms;
            }            
        }

        // Update qualification timer
        // TODO: Can this go in the above if statement?
        if(abs(packMonitorData->currentAdcAccumulator1_uV) > ACCUMULATED_CURRENT_THRES_UV)
        {
            clearTimer(&taskData->socData.socByOcvQualificationTimer);
        }
        else
        {
            updateTimer(&taskData->socData.socByOcvQualificationTimer);
        }

        // Update soc
        updateSocSoe(&taskData->socData, taskData->minCellVoltage);
    }
}

static void controlPositiveIR(PositiveIRControl_E irCommand)
{
    if(irCommand == OPEN_IR)
    {
        HAL_GPIO_WritePin(PRECHARGE_DONE_GPIO_Port, PRECHARGE_DONE_Pin, GPIO_PIN_RESET);
    }
    else if(irCommand == CLOSE_IR)
    {
        HAL_GPIO_WritePin(PRECHARGE_DONE_GPIO_Port, PRECHARGE_DONE_Pin, GPIO_PIN_SET);
    }
}

static void updatePrechargeLogic(packMonitorTaskData_S* taskData)
{
    uint32_t now = HAL_GetTick();

    // Update status at end of shutdown circuit (using TIM3 interrupts on ADC1 for ADC trigger)
    uint16_t localAdcValue = (uint16_t)adcRawValue;
    taskData->sdcEndVoltage_V = localAdcValue * adcCountsToSdcVoltsGain;

    bool sdcClosed = (taskData->sdcEndVoltage_V > SDC_END_V_THRESHOLD);
    bool linkReady = (taskData->linkVoltage > (0.93f * taskData->packVoltage)) && (taskData->packVoltage > 20.0f);
    bool sdcDelayComplete = ((now - taskData->sdcCloseTime) > PRECHARGE_WINDOW_MS) && (taskData->sdcCloseTime != 0);

    switch(taskData->positiveIRStatus)
    {
        case IR_STATE_SDC_OPEN:
        {
            if(sdcClosed)
            {
                taskData->sdcCloseTime = now;
                taskData->positiveIRStatus = IR_STATE_PRECHARGING;
            }
            break;
        }
        case IR_STATE_PRECHARGING:
        {
            if(!sdcClosed)
            {
                controlPositiveIR(OPEN_IR);
                taskData->sdcCloseTime = 0;
                taskData->positiveIRStatus = IR_STATE_SDC_OPEN;
                break;
            }
            else if(linkReady && sdcDelayComplete)
            {
                controlPositiveIR(CLOSE_IR);
                taskData->positiveIRStatus = IR_STATE_CLOSED;
            }
            break;
        }
        case IR_STATE_CLOSED:
        {
            if(!sdcClosed)
            {
                controlPositiveIR(OPEN_IR);
                taskData->sdcCloseTime = 0;
                taskData->positiveIRStatus = IR_STATE_SDC_OPEN;
            }
            break;
        }
        default:
        {
            taskData->positiveIRStatus = IR_STATE_SDC_OPEN;
            break;
        }
    }
}

static void runPackMonitorAlertMonitor(packMonitorTaskData_S* taskData)
{
    // Accumulate alert statuses
    bool responseStatus[NUM_ALERT_RESPONSES] = {false};

    for(uint32_t i = 0; i < NUM_PACK_MONITOR_ALERTS; i++)
    {
        Alert_S* alert = packMonitorAlerts[i];

        // Check alert condition and run alert monitor
        alert->alertConditionPresent = packMonitorAlertConditionArray[i](taskData);
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

            // Bit encoding for GopherCAN
            uint8_t bitIndex = alert->gcanAlertEncode;
            taskData->packMonitorGcanAlerts |= (1U << bitIndex);
        }
        else 
        {
            // Bit encoding for GopherCAN
            uint8_t bitIndex = alert->gcanAlertEncode;
            taskData->packMonitorGcanAlerts &= ~(1U << bitIndex);
        }
    }
    bmsFaultByTask.packMonitorBmsFault = responseStatus[BMS_FAULT];
    setBmsFault();
}

static void updateCurrentLimit(packMonitorTaskData_S* taskData)
{
    static PID_S currentPID = 
    {
        .kp = 0,
        .kiNeg = 0,
        .kiPos = 0,
        .kaw = 0,
        .outputMax = 175,
        .outputMin = 0,
        .dt = 2,
        .integratorMax = 50,
        .integratorMin = 0,
        .setPoint = 175,
    };

    float feedForward = currentPID.previousOutput + ((taskData->minCellVoltage - 3.05f) / 2.0f);

    pidStep(&currentPID, taskData->minCellVoltage, feedForward);

    updateI(&currentPID, taskData->minCellVoltage);
}

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void initUpdatePackMonitorTask()
{
    // Set CS high upon start up
    HAL_GPIO_WritePin(PACK_MON_CS_N_GPIO_Port, PACK_MON_CS_N_Pin, GPIO_PIN_SET);

    // Initialize SOC/SOE qualification timer
    taskData.socData.socByOcvQualificationTimer = (Timer_S){.timCount = CELL_POLARIZATION_REST_MS, .lastUpdate = 0, .timThreshold = CELL_POLARIZATION_REST_MS};

    // Start ADC to measure voltage at end of shutdown circuit and timer to trigger ADC conversions
    HAL_ADC_Start_IT(&hadc1);
    HAL_TIM_Base_Start(&htim3);
}

void runUpdatePackMonitorTask()
{
    // Get minCellVoltage value from cell monitor task
    vTaskSuspendAll();
    taskData.minCellVoltage = publicCellMonitorTaskData.minCellVoltage;
    xTaskResumeAll();

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
        taskData.shuntTemp1 = lookup(packMonitorData.voltageAdc[SHUNT_TEMP1_INDEX], &shuntTempTable);
        taskData.prechargeTemp = lookup(packMonitorData.voltageAdc[PRECHARGE_TEMP_INDEX], &prechargeDischargeTempTable);
        taskData.dischargeTemp = lookup(packMonitorData.voltageAdc[DISCHARGE_TEMP_INDEX], &prechargeDischargeTempTable);

        taskData.shuntResistance_nOhms = (int32_t)lroundf(lookup(packMonitorData.voltageAdc[SHUNT_TEMP1_INDEX], &shuntResistanceTable));

        taskData.packCurrent = (float)packMonitorData.currentAdc1_uV * 1000 / taskData.shuntResistance_nOhms;
        taskData.packVoltage = packMonitorData.batteryVoltage1 * HV_DIV_GAIN;
        taskData.packPower = taskData.packCurrent * taskData.packVoltage;

        taskData.linkVoltage = (packMonitorData.voltageAdc[LINK_PLUS_DIV_INDEX] - packMonitorData.voltageAdc[LINK_MINUS_DIV_INDEX]) * LINK_DIV_GAIN;

        calculatePackParameters(&packMonitorData, &taskData);
    }

    // Regardless of status, update precharge logic
    // TODO: Need a way to detect if you lose comms with the 2950 and have stale link/batt voltage values
    updatePrechargeLogic(&taskData);

    // static uint32_t lastPrint = 0;

    // if(HAL_GetTick() - lastPrint > 1500)
    // {
    //     lastPrint = HAL_GetTick();

    //     printf("\e[1;1H\e[2J");

    //     printf("// Pack Parameters //\n");
    //     printf("BATTERY VOLTAGE: %f V\n", taskData.packVoltage);
    //     printf("LINK VOLTAGE: %f V\n", taskData.linkVoltage);
    //     printf("SHDN END VOLTAGE: %f V\n", taskData.sdcEndVoltage_V);
    //     printf("SDC CLOSE TIME: %u ms\n", taskData.sdcCloseTime);
    //     if(taskData.positiveIRStatus == IR_STATE_SDC_OPEN)
    //     {
    //         printf("STATE: SDC_OPEN\n");
    //     }
    //     else if(taskData.positiveIRStatus == IR_STATE_PRECHARGING)
    //     {
    //         printf("STATE: PRECHARGING\n");
    //     }
    //     else if(taskData.positiveIRStatus == IR_STATE_CLOSED)
    //     {
    //         printf("STATE: IR CLOSED\n");
    //     }               
    // }

    // Regardless of status, run alert monitor
    runPackMonitorAlertMonitor(&taskData);

    // Copy task data to public struct
    vTaskSuspendAll();
    publicPackMonitorTaskData = taskData;
    xTaskResumeAll();
    
}
