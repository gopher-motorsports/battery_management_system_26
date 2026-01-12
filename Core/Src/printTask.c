/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "printTask.h"
#include "updateCellMonitorTask.h"
#include "cellMonitorTelemetry.h"
#include "updatePackMonitorTask.h"
#include <stdio.h>
#include <cmsis_os.h>

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

cellMonitorTask_S cellTaskPrintData;

packMonitorTask_S packTaskPrintData;

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

static void printCellVoltages(cellMonitorTask_S* cellTaskPrintData);

static void printCellTemps(cellMonitorTask_S* cellTaskPrintData);

static void printPackMonData(packMonitorTask_S* packTaskPrintData);

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

static void printCellVoltages(cellMonitorTask_S* cellTaskPrintData)
{
    printf("Cell Voltage:\n");
    printf("|   CELL   |");
    for(int32_t i = 0; i < NUM_CELL_MON; i++)
    {
        printf("    %02ld     |", i);
    }
    printf("\n");
    for(int32_t i = 0; i < NUM_CELLS_PER_CELL_MONITOR; i++)
    {
        printf("|    %02ld    |", i+1);
        for(int32_t j = 0; j < NUM_CELL_MON; j++)
        {
            if((cellTaskPrintData->cellVoltage[i] < 0.0f) || cellTaskPrintData->cellVoltage[i] >= 100.0f)
            {
                printf("  %5.3f   |", cellTaskPrintData->cellVoltage[i]);
            }
            else
            {
                printf("   %5.3f   |", cellTaskPrintData->cellVoltage[i]);
            }
        }
        printf("\n");
    }
	printf("\n");
}

static void printCellTemps(cellMonitorTask_S* cellTaskPrintData)
{
    printf("Cell Temp:\n");
    printf("|   BMB    |");
    for(int32_t i = 0; i < NUM_CELL_MON; i++)
    {
        printf("     %02ld   |", i);
    }
    printf("\n");
    for(int32_t i = 0; i < NUM_CELLS_PER_CELL_MONITOR; i++)
    {
        printf("|    %02ld    |", i+1);
        for(int32_t j = 0; j < NUM_CELL_MON; j++)
        {
            if((cellTaskPrintData->cellTemp[i] < 0.0f) || cellTaskPrintData->cellTemp[i] >= 100.0f)
            {
                printf("   %3.1f   |", (double)cellTaskPrintData->cellTemp[i]);
            }
            else
            {
                printf("    %3.1f   |", (double)cellTaskPrintData->cellTemp[i]);
            }
        }
        printf("\n");
    }
    printf("|  Board   |");
    for(int32_t j = 0; j < NUM_CELL_MON; j++)
    {
        printf("   %3.1f    |", (double)cellTaskPrintData->boardTemp1);
    }
	printf("\n\n");
    // printf("|   Die   |");
    // for(int32_t j = 0; j < NUM_CELL_MON; j++)
    // {
    //     if(dieTempStatus == GOOD)
    //     {
    //         if((dieTemp < 0.0f) || dieTemp >= 100.0f)
    //         {
    //             printf("   %3.1f   |", (double)dieTemp);
    //         }
    //         else
    //         {
    //             printf("    %3.1f   |", (double)dieTemp);
    //         }
    //         // printf("  %04X", gBms.cellVoltage[i]);
    //     }
    //     else
    //     {
    //         printf(" NO SIGNAL |");
    //     }
    // }
	// printf("\n");
}

static void printPackMonData(packMonitorTask_S* packTaskPrintData)
{
    printf("// Pack Parameters //\n");
    printf("Battery Current: %f A,    ", packTaskPrintData->packCurrent);
    printf("Battery Voltage: %f V\n", packTaskPrintData->packVoltage);
    printf("Power: %f W\n", packTaskPrintData->packPower);
    printf("Shunt Temp: %f C,    ", packTaskPrintData->shuntTemp1);
    printf("Shunt Resistance: %li nOhms\n", packTaskPrintData->shuntResistance_nOhms);
    printf("Precharge Temp: %f C,    ", packTaskPrintData->prechargeTemp);
    printf("Discharge Temp: %f C\n", packTaskPrintData->dischargeTemp);
    printf("Link Voltage: %f V\n", packTaskPrintData->linkVoltage);
    printf("Conversion Time: %hu us\n", packTaskPrintData->conversionTime_us);
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
    cellTaskPrintData = cellTaskDataPublic;
    packTaskPrintData = packTaskDataPublic;
    xTaskResumeAll();

    printf("\e[1;1H\e[2J");
    printCellVoltages(&cellTaskPrintData);
    printCellTemps(&cellTaskPrintData);
    printPackMonData(&packTaskPrintData);

}
