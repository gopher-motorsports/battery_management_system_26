#ifndef INC_UPDATE_PACK_MONITOR_TASK_H_
#define INC_UPDATE_PACK_MONITOR_TASK_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "soc.h"
#include <stdint.h>

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define SDC_END_V_GAIN              11
#define SDC_END_V_THRESHOLD         10.0f
#define PRECHARGE_WINDOW_MS         3000
#define POSITIVE_IR_COOLDOWN_MS     3000

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
extern volatile uint32_t adcNewDataFlag;

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
    uint32_t positiveIROpenTime;
    PositiveIRStatus_E positiveIRStatus;

    // minCellVoltage from cell monitor task
    float minCellVoltage;

    // Calculated values

    int32_t shuntResistance_nOhms;

    uint16_t conversionTime_us;

    Soc_S socData;
    
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
