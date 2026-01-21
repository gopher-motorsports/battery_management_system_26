/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "printTask.h"
#include "updateCellMonitorTask.h"
#include "cellMonitorTelemetry.h"
#include "updatePackMonitorTask.h"
#include "alerts.h"
#include <stdio.h>
#include <cmsis_os.h>

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

cellMonitorTaskData_S cellTaskPrintData;

packMonitorTaskData_S packTaskPrintData;

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

static void printCellVoltages(cellMonitorTaskData_S* cellTaskPrintData);

static void printCellTemps(cellMonitorTaskData_S* cellTaskPrintData);

static void printPackMonData(packMonitorTaskData_S* packTaskPrintData);

static bool printActiveAlerts(Alert_S** alerts, uint16_t num_alerts);

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

static void printCellVoltages(cellMonitorTaskData_S* cellTaskPrintData)
{
    printf("Cell Voltage:\n");
    printf("|   CELL   |");
    for(int32_t i = 0; i < NUM_CELL_MON; i++)
    {
        printf("    %02ld    |", i);
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
	printf("\n");
}

static void printCellTemps(cellMonitorTaskData_S* cellTaskPrintData)
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
            printf("   %3.1f   |", (double)cellTaskPrintData->cellMonitor[j].cellTemp[i]);
        }
        printf("\n");
    }
    printf("|  Board   |");
    for(int32_t i = 0; i < NUM_CELL_MON; i++)
    {
        printf("    %3.1f   |", (double)cellTaskPrintData->cellMonitor[i].boardTemp1);
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

static void printPackMonData(packMonitorTaskData_S* packTaskPrintData)
{
    printf("// Pack Parameters //\n");
    printf("Battery Current: %f A,    ", packTaskPrintData->packCurrent);
    printf("Battery Voltage: %f V,    ", packTaskPrintData->packVoltage);
    printf("Power: %f W\n", packTaskPrintData->packPower);
    printf("Shunt Temp: %f C,        ", packTaskPrintData->shuntTemp1);
    printf("Shunt Resistance: %li nOhms\n", packTaskPrintData->shuntResistance_nOhms);
    printf("Precharge Temp: %f C,   ", packTaskPrintData->prechargeTemp);
    printf("Discharge Temp: %f C\n", packTaskPrintData->dischargeTemp);
    printf("Link Voltage: %f V,       ", packTaskPrintData->linkVoltage);
    printf("Conversion Time: %hu us\n", packTaskPrintData->conversionTime_us);
}

static bool printActiveAlerts(Alert_S** alerts, uint16_t num_alerts)
{
    bool alertActive = false;

    for (uint16_t i = 0; i < num_alerts; i++) {
        if (alerts[i]->alertStatus == ALERT_SET) {
            printf("ALERT: %s\n", alerts[i]->alertName);
            alertActive = true;
        } else if (alerts[i]->alertStatus == ALERT_LATCHED) {
            printf("ALERT: %s LATCHED\n", alerts[i]->alertName);
            alertActive = true;
        }
    }

    return alertActive;
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
    packTaskPrintData = publicPackMonitorTaskData;
    xTaskResumeAll();

    printf("\e[1;1H\e[2J");
    printCellVoltages(&cellTaskPrintData);
    // printCellTemps(&cellTaskPrintData);

    printf("Max Cell Voltage: %f\n", cellTaskPrintData.maxCellVoltage);
    printf("Min Cell Voltage: %f\n", cellTaskPrintData.minCellVoltage);
    printf("Max Cell Temp: %f\n", cellTaskPrintData.maxCellTemp);
    printf("Min Cell Temp: %f\n", cellTaskPrintData.minCellTemp);

    printPackMonData(&packTaskPrintData);

    printf("\n");

    if(!printActiveAlerts(cellMonitorAlerts, NUM_CELL_MONITOR_ALERTS)
    && !printActiveAlerts(packMonitorAlerts, NUM_PACK_MONITOR_ALERTS)) {
    printf("NO ALERTS ACTIVE\n");
    }

}
