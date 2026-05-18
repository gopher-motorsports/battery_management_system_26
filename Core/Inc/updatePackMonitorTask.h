#ifndef INC_UPDATE_PACK_MONITOR_TASK_H_
#define INC_UPDATE_PACK_MONITOR_TASK_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "soc.h"
#include <stdint.h>

/* ==================================================================== */
/* ========================= ENUMERATED TYPES========================== */
/* ==================================================================== */

typedef enum
{
    IR_STATE_SDC_OPEN,          // Shutdown circuit open, battery isolated
    IR_STATE_PRECHARGING,       // Shutdown circuit closed, precharging LINK bus
    IR_STATE_CLOSED,            // IR+ and IR- closed, normal operation
} PositiveIRStatus_E;

/* ==================================================================== */
/* ======================= EXTERNAL VARIABLES ========================= */
/* ==================================================================== */

extern volatile uint32_t adcRawValue;

/* ==================================================================== */
/* ============================== STRUCTS ============================= */
/* ==================================================================== */

typedef struct 
{
    // Pack measurements

    float packCurrent;
    float packVoltage;
    float packPower;

    float shuntTemp1;
    float prechargeTemp;
    float dischargeTemp;

    float linkVoltage;

    // Parameters for positive IR control
    float sdcEndVoltage_V;
    uint32_t sdcCloseTime;
    PositiveIRStatus_E positiveIRStatus;

    // minCellVoltage from cell monitor task
    float minCellVoltage;

    // Calculated values

    int32_t shuntResistance_nOhms;

    uint16_t conversionTime_us;

    Soc_S socData;

    // Alerts Bit Encoded for GopherCAN
    uint8_t packMonitorGcanAlerts;

} packMonitorTaskData_S;

/* ==================================================================== */
/* ======================= EXTERNAL VARIABLES ========================= */
/* ==================================================================== */

extern packMonitorTaskData_S publicPackMonitorTaskData;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

void initUpdatePackMonitorTask();
void runUpdatePackMonitorTask();

#endif /* INC_UPDATE_PACK_MONITOR_TASK_H_ */
