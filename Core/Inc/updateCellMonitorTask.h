#ifndef INC_UPDATE_CELL_MONITOR_TASK_H_
#define INC_UPDATE_CELL_MONITOR_TASK_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "adbms/adbmsCellMonitor.h"

/* ==================================================================== */
/* ============================== STRUCTS ============================= */
/* ==================================================================== */

typedef struct 
{
    // Module measurements

    // Cell voltage array
    float cellVoltage[NUM_CELLS_PER_CELL_MONITOR];

    // Cell temp array
    float cellTemp[NUM_CELLS_PER_CELL_MONITOR];

    float boardTemp1;
    float boardTemp2;

    // Calculated values

    float maxCellVoltage;
    float minCellVoltage;

    float maxCellTemp;
    float minCellTemp;
        
} cellMonitorTask_S;

/* ==================================================================== */
/* ======================= EXTERNAL VARIABLES ========================= */
/* ==================================================================== */

extern cellMonitorTask_S cellTaskDataPublic;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

void initUpdateCellMonitorTask();
void runUpdateCellMonitorTask();

#endif /* INC_UPDATE_CELL_MONITOR_TASK_H_ */
