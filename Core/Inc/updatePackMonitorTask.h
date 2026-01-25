#ifndef INC_UPDATE_PACK_MONITOR_TASK_H_
#define INC_UPDATE_PACK_MONITOR_TASK_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "soc.h"
#include <stdint.h>

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
