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

#define SHDN_END_V_GAIN         11
#define SHDN_END_V_THRESHOLD    10.0f
#define PRECHARGE_WINDOW_MS     3000

/* ==================================================================== */
/* ======================= EXTERNAL VARIABLES ========================= */
/* ==================================================================== */

extern volatile uint32_t adcRawValue;
extern volatile uint32_t adcNewDataFlag;
extern volatile bool prechargeDelayComplete;

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

    // Delay between shut down circuit closing and IR+ closing by measuring end of shut down circuit
    bool prechargeDelayComplete;
    float shutdownEndVoltage_V;

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
