/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "printTask.h"
#include "updateCellMonitorTask.h"
#include "cellMonitorTelemetry.h"
// #include "alerts.h"
#include <stdio.h>
#include <cmsis_os.h>

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

cellMonitorTaskData_S cellTaskPrintData;

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

static void printCellVoltages(cellMonitorTaskData_S* cellTaskPrintData);

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

static void printCellVoltages(cellMonitorTaskData_S* cellTaskPrintData)
{
    printf("Cell Voltage:\n");
    printf("|   CELL   |");
    for(int32_t i = 0; i < NUM_CELL_MON; i++)
    {
        printf("    %02ld    |", i+1);
    }
    printf("\n");
    for(int32_t i = 0; i < NUM_CELLS_PER_CELL_MONITOR; i++)
    {
        printf("|    %02ld    |", i+1);
        for(int32_t j = 0; j < NUM_CELL_MON; j++)
        {
            printf("  %5.3f   |", cellTaskPrintData->cellMonitor[j].cellVoltage[i]);
        }
        printf("\n");
    }
    printf("|   MIN    |");
    for(int32_t i = 0; i < NUM_CELL_MON; i++)
    {
        float min = 5.0f;
        for(int32_t j = 0; j < NUM_CELLS_PER_CELL_MONITOR; j++)
        {
            if(cellTaskPrintData->cellMonitor[i].cellVoltage[j] < min)
            {
                min = cellTaskPrintData->cellMonitor[i].cellVoltage[j];
            }
        }
        printf("  %5.3f   |", min);
    }
	printf("\n");
}

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void initPrintTask()
{

}

void runPrintTask()
{
    // Critical section - copy data from public task structs into local print task structs
    vTaskSuspendAll();
    cellTaskPrintData = publicCellMonitorTaskData;
    xTaskResumeAll();

    printf("\e[1;1H\e[2J");
    printCellVoltages(&cellTaskPrintData);
}