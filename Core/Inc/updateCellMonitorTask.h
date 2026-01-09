#ifndef INC_UPDATE_CELL_MONITOR_TASK_H_
#define INC_UPDATE_CELL_MONITOR_TASK_H_

/* ==================================================================== */
/* ============================== STRUCTS ============================= */
/* ==================================================================== */

typedef struct 
{
    // Module measurements

    // Cell voltage array
    float cellVoltage[14]; // TODO: what is the correct way to include NUM_CELLS_PER_CELL_MONITOR from adbmsCellMonitor.h?

    // Cell temp array
    float cellTemp[14];

    float boardTemp1;
    float boardTemp2;

    // Calculated values

    float maxCellVoltage;
    float minCellVoltage;

    float maxCellTemp;
    float minCellTemp;
        
} cellMonitorTask_S;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

void initUpdateCellMonitorTask();
void runUpdateCellMonitorTask();

#endif /* INC_UPDATE_CELL_MONITOR_TASK_H_ */
