#ifndef INC_UPDATE_CELL_MONITOR_TASK_H_
#define INC_UPDATE_CELL_MONITOR_TASK_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "adbms/adbmsCellMonitor.h"
#include "cellMonitorTelemetry.h"
#include "packData.h"

/* ==================================================================== */
/* ============================== STRUCTS ============================= */
/* ==================================================================== */

typedef struct 
{
    // Module measurements

    // Cell voltage array
    float cellVoltage[NUM_CELLS_PER_CELL_MONITOR];
    SENSOR_STATUS_E cellVoltageStatus[NUM_CELLS_PER_CELL_MONITOR];

    // Cell temp array
    float cellTemp[NUM_CELLS_PER_CELL_MONITOR];
    SENSOR_STATUS_E cellTempStatus[NUM_CELLS_PER_CELL_MONITOR];

    float boardTemp1;
    SENSOR_STATUS_E boardTemp1Status;

    float boardTemp2;
    SENSOR_STATUS_E boardTemp2Status;

    float regTemp;
    SENSOR_STATUS_E regTempStatus;

    // Cell monitor local voltage statistics
    float maxCellVoltage;
    float minCellVoltage;
    float sumCellVoltage;
    float avgCellVoltage;
    uint32_t numBadCellVoltage;

    // Cell monitor local temp statistics
    float maxCellTemp;
    float minCellTemp;
    float avgCellTemp;
    uint32_t numBadCellTemp;
        
} cellMonitor_S;

typedef struct
{
    cellMonitor_S cellMonitor[NUM_CELL_MON];

    bool balancingEnabled;
    float balancingFloor;

    float cellSumVoltage;

    float maxCellVoltage;
    float minCellVoltage;
    float avgCellVoltage;

    float cellImbalance;

    float maxCellTemp;
    float minCellTemp;
    float avgCellTemp;

    float maxBoardTemp;
    float minBoardTemp;
    float avgBoardTemp;
    uint32_t numBadBoardTemp;

    float maxDieTemp;
    float minDieTemp;
    float avgDieTemp;

} cellMonitorTaskData_S;

/* ==================================================================== */
/* ======================= EXTERNAL VARIABLES ========================= */
/* ==================================================================== */

extern cellMonitorTaskData_S publicCellMonitorTaskData;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

void initUpdateCellMonitorTask();
void runUpdateCellMonitorTask();

#endif /* INC_UPDATE_CELL_MONITOR_TASK_H_ */
