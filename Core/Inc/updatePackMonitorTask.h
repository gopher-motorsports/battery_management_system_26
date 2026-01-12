#ifndef INC_UPDATE_PACK_MONITOR_TASK_H_
#define INC_UPDATE_PACK_MONITOR_TASK_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

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

    // Calculated values

    int32_t shuntResistance_nOhms;

    uint16_t conversionTime_us;
    
} packMonitorTask_S;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

void initUpdatePackMonitorTask();
void runUpdatePackMonitorTask();

#endif /* INC_UPDATE_PACK_MONITOR_TASK_H_ */
