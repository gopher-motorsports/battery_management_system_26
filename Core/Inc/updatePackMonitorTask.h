#ifndef INC_UPDATE_PACK_MONITOR_TASK_H_
#define INC_UPDATE_PACK_MONITOR_TASK_H_

/* ==================================================================== */
/* ============================== STRUCTS ============================= */
/* ==================================================================== */

typedef struct {

    // Primary pack measurements
    float packCurrent;
    float packVoltage;
    float packPower;
    
} packMonitorTask_S;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

void initUpdatePackMonitorTask();
void runUpdatePackMonitorTask();

#endif /* INC_UPDATE_PACK_MONITOR_TASK_H_ */
