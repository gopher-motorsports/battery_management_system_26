/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "alerts.h"
#include "updateCellMonitorTask.h"

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

static bool overvoltageWarningPresent(cellMonitorTaskData_S* taskData)
{
    return (taskData->maxCellVoltage > MAX_BRICK_WARNING_VOLTAGE);
}

static bool overvoltageFaultPresent(cellMonitorTaskData_S* taskData)
{
    return (taskData->maxCellVoltage > MAX_BRICK_FAULT_VOLTAGE);
}

static bool undervoltageWarningPresent(cellMonitorTaskData_S* taskData)
{
    return (taskData->minCellVoltage < MIN_BRICK_WARNING_VOLTAGE);
}

static bool undervoltageFaultPresent(cellMonitorTaskData_S* taskData)
{
    return (taskData->minCellVoltage < MIN_BRICK_FAULT_VOLTAGE);
}

static bool cellImbalancePresent(cellMonitorTaskData_S* taskData)
{
    return (taskData->cellImbalance > MAX_CELL_IMBALANCE_V);
}

static bool overtemperatureWarningPresent(cellMonitorTaskData_S* taskData)
{
    return (taskData->maxCellTemp > MAX_BRICK_TEMP_WARNING_C);
}

static bool overtemperatureFaultPresent(cellMonitorTaskData_S* taskData)
{
    return (taskData->maxCellTemp > MAX_BRICK_TEMP_FAULT_C);
}

static bool badVoltageSensorStatusPresent(cellMonitorTaskData_S* taskData)
{
    for(uint32_t i = 0; i < NUM_CELL_MON; i++)
    {
        if(taskData->cellMonitor[i].numBadCellVoltage != 0)
        {
            return true;
        }
    }
    return false;
}

static bool badBrickTempSensorStatusPresent(cellMonitorTaskData_S* taskData)
{
    for(uint32_t i = 0; i < NUM_CELL_MON; i++)
    {
        if(taskData->cellMonitor[i].numBadCellTemp != 0)
        {
            return true;
        }
    }
    return false;
}

// static bool badBoardTempSensorStatusPresent(cellMonitorTaskData_S* taskData)
// {
//     for (uint32_t i = 0; i < NUM_CELL_MON; i++)
//     {
//         if(taskData->cellMonitor[i].boardTempStatus != GOOD)
//         {
//             return true;
//         }
//     }
//     return false;
// }

static bool insufficientTempSensePresent(cellMonitorTaskData_S* taskData)
{
    const uint32_t maxNumBadBrickTempAllowed = NUM_CELLS_PER_CELL_MONITOR * (100 - MIN_PERCENT_BRICK_TEMPS_MONITORED) / 100;
    for (uint32_t i = 0; i < NUM_CELL_MON; i++)
    {
        if(taskData->cellMonitor[i].numBadCellTemp > maxNumBadBrickTempAllowed)
        {
            return true;
        }
    }
    return false;
}

// static bool telemetryCommunicationErrorPresent(cellMonitorTaskData_S* taskData)
// {
//     for(uint32_t i = 0; i < NUM_CELL_MON; i++)
//     {
//         if(taskData->cellMonitorStatus[i] != GOOD)
//         {
//             return true;
//         }
//     }
//     return false;
// }

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

/*!
  @brief   Get the status of any given alert
  @param   alert - The alert data structure whose status to read
  @return  The current status of the alert
*/
AlertStatus_E getAlertStatus(Alert_S* alert)
{
    return alert->alertStatus;
}

/*!
  @brief   Run the alert monitor to update the status of the alert
  @param   alert - The Alert data structure
*/
void runAlertMonitor(Alert_S* alert)
{
    if (alert->alertStatus == ALERT_CLEARED || alert->alertStatus == ALERT_LATCHED)
    {
        // Determine if we need to set the alert
        if (alert->alertConditionPresent)
        {
            // Increment alert timer by 10ms
            updateTimer(&alert->alertTimer);
        }
        else
        {
            // Reset alert timer
            clearTimer(&alert->alertTimer);
        }

        // Determine if alert was set or in the case that the timer threshold is 0 then check whether the alert condition is present
        if (checkTimerExpired(&alert->alertTimer) && (!(alert->alertTimer.timThreshold <= 0) || alert->alertConditionPresent))
        {
            // Timer expired - Set alert
            alert->alertStatus = ALERT_SET;
            // Load timer with alert clear time
            configureTimer(&alert->alertTimer, alert->clearTime_MS);
        }

    }
    else if (alert->alertStatus == ALERT_SET)
    {
        // Determine if we can clear the alert
        if (!alert->alertConditionPresent)
        {
            // Increment clear timer by 10ms
            updateTimer(&alert->alertTimer);
        }
        else
        {
            // Alert conditions detected. Reset clear timer
            clearTimer(&alert->alertTimer);
        }

        // Determine if alert was cleared or in the case that the timer threshold is 0 then check whether the alert condition is not present
        if (checkTimerExpired(&alert->alertTimer) && (!(alert->alertTimer.timThreshold <= 0) || !alert->alertConditionPresent))
        {
            // Timer expired indicating alert is no longer present. Either set status to latched or clear
            if (alert->latching)
            {
                // Latching alerts can't be cleared - set status to latched to indicate that conditions are no longer met
                alert->alertStatus = ALERT_LATCHED;
            }
            else
            {
                // If non latching alert, the alert can be cleared
                alert->alertStatus = ALERT_CLEARED;
            }
            // Load timer with alert set time
            configureTimer(&alert->alertTimer, alert->setTime_MS);
        }
    }
}

void setBmsFault(bool set)
{
	// BMS fault pin is active high so if set == true then pin should be high
	HAL_GPIO_WritePin(BMS_FAULT_GPIO_Port, BMS_FAULT_Pin, set ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/* ==================================================================== */
/* ========================= GLOBAL VARIABLES ========================= */
/* ==================================================================== */

// Overvoltage Warning Alert
const AlertResponse_E overvoltageWarningAlertResponse[] = { DISABLE_CHARGING };
#define NUM_OVERVOLTAGE_WARNING_ALERT_RESPONSE sizeof(overvoltageWarningAlertResponse) / sizeof(AlertResponse_E)
Alert_S overvoltageWarningAlert =
{ 
    .alertName = "OvervoltageWarning",
    .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = OVERVOLTAGE_WARNING_ALERT_SET_TIME_MS}, 
    .setTime_MS = OVERVOLTAGE_WARNING_ALERT_SET_TIME_MS, .clearTime_MS = OVERVOLTAGE_WARNING_ALERT_CLEAR_TIME_MS, 
    .alertConditionPresent = false, 
    .numAlertResponse = NUM_OVERVOLTAGE_WARNING_ALERT_RESPONSE, .alertResponse =  overvoltageWarningAlertResponse
};

// Undervoltage Warning Alert
const AlertResponse_E undervoltageWarningAlertResponse[] = { LIMP_MODE };
#define NUM_UNDERVOLTAGE_WARNING_ALERT_RESPONSE sizeof(undervoltageWarningAlertResponse) / sizeof(AlertResponse_E)
Alert_S undervoltageWarningAlert = 
{ 
    .alertName = "UndervoltageWarning",
    .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = UNDERVOLTAGE_WARNING_ALERT_SET_TIME_MS}, 
    .setTime_MS = UNDERVOLTAGE_WARNING_ALERT_SET_TIME_MS, .clearTime_MS = UNDERVOLTAGE_WARNING_ALERT_CLEAR_TIME_MS, 
    .alertConditionPresent = false,
    .numAlertResponse = NUM_UNDERVOLTAGE_WARNING_ALERT_RESPONSE, .alertResponse = undervoltageWarningAlertResponse
};

// Overvoltage Fault Alert
const AlertResponse_E overvoltageFaultAlertResponse[] = { DISABLE_CHARGING, EMERGENCY_BLEED, BMS_FAULT};
#define NUM_OVERVOLTAGE_FAULT_ALERT_RESPONSE sizeof(overvoltageFaultAlertResponse) / sizeof(AlertResponse_E)
Alert_S overvoltageFaultAlert = 
{ 
    .alertName = "OvervoltageFault", .latching = true,
    .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = OVERVOLTAGE_FAULT_ALERT_SET_TIME_MS}, 
    .setTime_MS = OVERVOLTAGE_FAULT_ALERT_SET_TIME_MS, .clearTime_MS = OVERVOLTAGE_FAULT_ALERT_CLEAR_TIME_MS, 
    .alertConditionPresent = false, 
    .numAlertResponse = NUM_OVERVOLTAGE_FAULT_ALERT_RESPONSE, .alertResponse =  overvoltageFaultAlertResponse
};

// Undervoltage Fault Alert
const AlertResponse_E undervoltageFaultAlertResponse[] = { BMS_FAULT };
#define NUM_UNDERVOLTAGE_FAULT_ALERT_RESPONSE sizeof(undervoltageFaultAlertResponse) / sizeof(AlertResponse_E)
Alert_S undervoltageFaultAlert = 
{ 
    .alertName = "UndervoltageFault", .latching = true,
    .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = UNDERVOLTAGE_FAULT_ALERT_SET_TIME_MS}, 
    .setTime_MS = UNDERVOLTAGE_FAULT_ALERT_SET_TIME_MS, .clearTime_MS = UNDERVOLTAGE_FAULT_ALERT_CLEAR_TIME_MS, 
    .alertConditionPresent = false,
    .numAlertResponse = NUM_UNDERVOLTAGE_FAULT_ALERT_RESPONSE, .alertResponse = undervoltageFaultAlertResponse
};

// Cell Imbalance Alert
const AlertResponse_E cellImbalanceAlertResponse[] = {INFO_ONLY};
#define NUM_CELL_IMBALANCE_ALERT_RESPONSE sizeof(cellImbalanceAlertResponse) / sizeof(AlertResponse_E)
Alert_S cellImbalanceAlert = 
{
    .alertName = "CellImbalance",
    .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = CELL_IMBALANCE_ALERT_SET_TIME_MS}, 
    .setTime_MS = CELL_IMBALANCE_ALERT_SET_TIME_MS, .clearTime_MS = CELL_IMBALANCE_ALERT_CLEAR_TIME_MS, 
    .alertConditionPresent = false,
    .numAlertResponse = NUM_CELL_IMBALANCE_ALERT_RESPONSE, .alertResponse = cellImbalanceAlertResponse
};

// Overtemperature Warning Alert
const AlertResponse_E overtempWarningAlertResponse[] = { LIMP_MODE, DISABLE_CHARGING, DISABLE_BALANCING };
#define NUM_OVERTEMP_WARNING_ALERT_RESPONSE sizeof(overtempWarningAlertResponse) / sizeof(AlertResponse_E)
Alert_S overtempWarningAlert = 
{
    .alertName = "OvertempWarning",
    .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = OVERTEMPERATURE_WARNING_ALERT_SET_TIME_MS}, 
    .setTime_MS = OVERTEMPERATURE_WARNING_ALERT_SET_TIME_MS, .clearTime_MS = OVERTEMPERATURE_WARNING_ALERT_CLEAR_TIME_MS, 
    .alertConditionPresent = false,
    .numAlertResponse = NUM_OVERTEMP_WARNING_ALERT_RESPONSE, .alertResponse = overtempWarningAlertResponse
};

// Overtemperature Fault Alert
const AlertResponse_E overtempFaultAlertResponse[] = { DISABLE_CHARGING, DISABLE_BALANCING, BMS_FAULT };
#define NUM_OVERTEMP_FAULT_ALERT_RESPONSE sizeof(overtempFaultAlertResponse) / sizeof(AlertResponse_E)
Alert_S overtempFaultAlert = 
{
    .alertName = "OvertempFault", .latching = true,
    .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = OVERTEMPERATURE_FAULT_ALERT_SET_TIME_MS}, 
    .setTime_MS = OVERTEMPERATURE_FAULT_ALERT_SET_TIME_MS, .clearTime_MS = OVERTEMPERATURE_FAULT_ALERT_CLEAR_TIME_MS, 
    .alertConditionPresent = false,
    .numAlertResponse = NUM_OVERTEMP_FAULT_ALERT_RESPONSE, .alertResponse = overtempFaultAlertResponse
};

// Bad voltage sensor status
const AlertResponse_E badVoltageSenseStatusAlertResponse[] = { DISABLE_BALANCING, DISABLE_CHARGING, BMS_FAULT };
#define NUM_BAD_VOLTAGE_SENSE_STATUS_ALERT_RESPONSE sizeof(badVoltageSenseStatusAlertResponse) / sizeof(AlertResponse_E)
Alert_S badVoltageSenseStatusAlert = 
{
    .alertName = "BadVoltageSenseStatus", .latching = true,
    .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = BAD_VOLTAGE_SENSE_STATUS_ALERT_SET_TIME_MS}, 
    .setTime_MS = BAD_VOLTAGE_SENSE_STATUS_ALERT_SET_TIME_MS, .clearTime_MS = BAD_VOLTAGE_SENSE_STATUS_ALERT_CLEAR_TIME_MS, 
    .alertConditionPresent = false,
    .numAlertResponse = NUM_BAD_VOLTAGE_SENSE_STATUS_ALERT_RESPONSE, .alertResponse = badVoltageSenseStatusAlertResponse
};

// Bad brick temperature sensor status
const AlertResponse_E badBrickTempSenseStatusAlertResponse[] = { INFO_ONLY };
#define NUM_BAD_BRICK_TEMP_SENSE_STATUS_ALERT_RESPONSE sizeof(badBrickTempSenseStatusAlertResponse) / sizeof(AlertResponse_E)
Alert_S badBrickTempSenseStatusAlert = 
{
    .alertName = "BadBrickTempSenseStatus",
    .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = BAD_BRICK_TEMP_SENSE_STATUS_ALERT_SET_TIME_MS}, 
    .setTime_MS = BAD_BRICK_TEMP_SENSE_STATUS_ALERT_SET_TIME_MS, .clearTime_MS = BAD_BRICK_TEMP_SENSE_STATUS_ALERT_CLEAR_TIME_MS, 
    .alertConditionPresent = false,
    .numAlertResponse = NUM_BAD_BRICK_TEMP_SENSE_STATUS_ALERT_RESPONSE, .alertResponse = badBrickTempSenseStatusAlertResponse
};

// Bad board temperature sensor status
const AlertResponse_E badBoardTempSenseStatusAlertResponse[] = { INFO_ONLY };
#define NUM_BAD_BOARD_TEMP_SENSE_STATUS_ALERT_RESPONSE sizeof(badBoardTempSenseStatusAlertResponse) / sizeof(AlertResponse_E)
Alert_S badBoardTempSenseStatusAlert = 
{
    .alertName = "BadBoardTempSenseStatus",
    .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = BAD_BOARD_TEMP_SENSE_STATUS_ALERT_SET_TIME_MS}, 
    .setTime_MS = BAD_BOARD_TEMP_SENSE_STATUS_ALERT_SET_TIME_MS, .clearTime_MS = BAD_BOARD_TEMP_SENSE_STATUS_ALERT_CLEAR_TIME_MS, 
    .alertConditionPresent = false,
    .numAlertResponse = NUM_BAD_BOARD_TEMP_SENSE_STATUS_ALERT_RESPONSE, .alertResponse = badBoardTempSenseStatusAlertResponse
};

// Lost more than 75% of temp sensors in pack
const AlertResponse_E insufficientTempSensorsAlertResponse[] = { DISABLE_BALANCING, DISABLE_CHARGING, BMS_FAULT };
#define NUM_INSUFFICIENT_TEMP_SENSORS_ALERT_RESPONSE sizeof(insufficientTempSensorsAlertResponse) / sizeof(AlertResponse_E)
Alert_S insufficientTempSensorsAlert = 
{
    .alertName = "InsufficientTempSensors", .latching = true,
    .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = INSUFFICIENT_TEMP_SENSOR_ALERT_SET_TIME_MS}, 
    .setTime_MS = INSUFFICIENT_TEMP_SENSOR_ALERT_SET_TIME_MS, .clearTime_MS = INSUFFICIENT_TEMP_SENSOR_ALERT_CLEAR_TIME_MS, 
    .alertConditionPresent = false,
    .numAlertResponse = NUM_INSUFFICIENT_TEMP_SENSORS_ALERT_RESPONSE, .alertResponse = insufficientTempSensorsAlertResponse
};

// Chain break, loss of comms
const AlertResponse_E telemetryCommunicationAlertResponse[] = { DISABLE_BALANCING, DISABLE_CHARGING, BMS_FAULT };
#define NUM_TELEMETRY_COMMS_ALERT_RESPONSE sizeof(telemetryCommunicationAlertResponse) / sizeof(AlertResponse_E)
Alert_S telemetryCommunicationAlert = 
{
    .alertName = "telemetryCommunicationError", .latching = true,
    .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = BMB_COMMUNICATION_FAILURE_ALERT_SET_TIME_MS}, 
    .setTime_MS = BMB_COMMUNICATION_FAILURE_ALERT_SET_TIME_MS, .clearTime_MS = BMB_COMMUNICATION_FAILURE_ALERT_CLEAR_TIME_MS, 
    .alertConditionPresent = false,
    .numAlertResponse = NUM_TELEMETRY_COMMS_ALERT_RESPONSE, .alertResponse = telemetryCommunicationAlertResponse
};

// Overcurrent
const AlertResponse_E packOvercurrentFaultAlertResponse[] = { DISABLE_BALANCING, DISABLE_CHARGING, BMS_FAULT };
#define NUM_PACK_OVERCURRENT_FAULT_ALERT_RESPONSE sizeof(packOvercurrentFaultAlertResponse) / sizeof(AlertResponse_E)
Alert_S packOvercurrentFaultAlert = 
{
    .alertName = "overcurrentFault", .latching = true,
    .alertStatus = ALERT_CLEARED, .alertTimer = (Timer_S){.timCount = 0, .lastUpdate = 0, .timThreshold = PACK_OVERCURRENT_FAULT_ALERT_SET_TIME_MS}, 
    .setTime_MS = PACK_OVERCURRENT_FAULT_ALERT_SET_TIME_MS, .clearTime_MS = PACK_OVERCURRENT_FAULT_ALERT_CLEAR_TIME_MS, 
    .alertConditionPresent = false,
    .numAlertResponse = NUM_PACK_OVERCURRENT_FAULT_ALERT_RESPONSE, .alertResponse = packOvercurrentFaultAlertResponse
};

// Telemetry alerts

Alert_S* telemetryAlerts[] = 
{
    &overvoltageWarningAlert,
    &undervoltageWarningAlert,
    &overvoltageFaultAlert,
    &undervoltageFaultAlert,
    &cellImbalanceAlert,
    &overtempWarningAlert,
    &overtempFaultAlert,
    &badVoltageSenseStatusAlert,
    &badBrickTempSenseStatusAlert,
    &badBoardTempSenseStatusAlert,
    &insufficientTempSensorsAlert,
    &telemetryCommunicationAlert,
    &packOvercurrentFaultAlert
};

telemetryAlertCondition telemetryAlertConditionArray[] = 
{
    overvoltageWarningPresent,
    undervoltageWarningPresent,
    overvoltageFaultPresent,
    undervoltageFaultPresent,
    cellImbalancePresent,
    overtemperatureWarningPresent,
    overtemperatureFaultPresent,
    badVoltageSensorStatusPresent,
    badBrickTempSensorStatusPresent,
    // badBoardTempSensorStatusPresent,
    insufficientTempSensePresent
    // telemetryCommunicationErrorPresent,
    // packOvercurrentFaultPresent
};

// Number of alerts

const uint32_t NUM_TELEMETRY_ALERTS = sizeof(telemetryAlerts) / sizeof(Alert_S*);
