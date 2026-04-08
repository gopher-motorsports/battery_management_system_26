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

static void printRedundantCellVoltages(cellMonitorTaskData_S* cellTaskPrintData);

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

static void printRedundantCellVoltages(cellMonitorTaskData_S* cellTaskPrintData)
{
    printf("Redundant Cell Voltage:\n");
    for(int32_t i = 0; i < NUM_CELLS_PER_CELL_MONITOR; i++)
    {
        printf("|    %02ld    |", i+1);
        for(int32_t j = 0; j < NUM_CELL_MON; j++)
        {
            float voltage = cellTaskPrintData->cellMonitor[j].cellVoltage[i];
            float redundantVoltage = cellTaskPrintData->cellMonitor[j].redundantCellVoltage[i];
            if(fabsf(voltage - redundantVoltage) < 0.002f)
            {
                printf("  %5.3f   |", redundantVoltage);
            }
            else
            {
                printf("  %5.3f ! |", redundantVoltage);
            }
        }
        printf("\n");
    }
	printf("\n");
}

static void printCellTemps(cellMonitorTaskData_S* cellTaskPrintData)
{
    // printf("Cell Temp:\n");
    // printf("|   BMB    |");
    // for(int32_t i = 0; i < NUM_CELL_MON; i++)
    // {
    //     printf("     %02ld   |", i+1);
    // }
    // printf("\n");
    // for(int32_t i = 0; i < NUM_CELLS_PER_CELL_MONITOR; i++)
    // {
    //     printf("|    %02ld    |", i+1);
    //     for(int32_t j = 0; j < NUM_CELL_MON; j++)
    //     {
    //         if(cellTaskPrintData->cellMonitor[j].cellTempStatus[i] == GOOD)
    //         {
    //             printf("   %3.1f   |", (double)cellTaskPrintData->cellMonitor[j].cellTemp[i]);
    //         }
    //         else if(cellTaskPrintData->cellMonitor[j].cellTempStatus[i] == BAD)
    //         {
    //             printf("  %3.1f ! |", (double)cellTaskPrintData->cellMonitor[j].cellTemp[i]);
    //         }
            
    //     }
    //     printf("\n");
    // }
    printf("|  Board   |");
    for(int32_t i = 0; i < NUM_CELL_MON; i++)
    {
        printf("   %3.1f   |", (double)cellTaskPrintData->cellMonitor[i].boardTemp1);
    }
	printf("\n");
    printf("|   Die    |");
    for(int32_t i = 0; i < NUM_CELL_MON; i++)
    {
        printf("   %3.1f   |", cellTaskPrintData->cellMonitor[i].dieTemp);
    }
	printf("\n\n");
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
    printf("Conversion Time: %hu us  ", packTaskPrintData->conversionTime_us);
    printf("Qualification Timer: %lu us\n", packTaskPrintData->socData.socByOcvQualificationTimer.timCount);
    printf("MilliColoumb Counter: %i mC\n", packTaskPrintData->socData.milliCoulombCounter);
    printf("SOC: %f by OCV, %f by CC   SOE: %f by OCV, %f by CC\n", packTaskPrintData->socData.socByOcv * 100, packTaskPrintData->socData.socByCoulombCounting * 100, packTaskPrintData->socData.soeByOcv * 100, packTaskPrintData->socData.soeByCoulombCounting * 100);
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
    printRedundantCellVoltages(&cellTaskPrintData);
    printCellTemps(&cellTaskPrintData);

    printf("Max Cell Voltage: %f\n", cellTaskPrintData.maxCellVoltage);
    printf("Min Cell Voltage: %f\n", cellTaskPrintData.minCellVoltage);
    printf("Max Cell Temp: %f\n", cellTaskPrintData.maxCellTemp);
    printf("Min Cell Temp: %f\n", cellTaskPrintData.minCellTemp);

    // printPackMonData(&packTaskPrintData);

    printf("\n");

    // bool cellAlerts = printActiveAlerts(cellMonitorAlerts, NUM_CELL_MONITOR_ALERTS);
    // bool packAlerts = printActiveAlerts(packMonitorAlerts, NUM_PACK_MONITOR_ALERTS);

    // if(!cellAlerts && !packAlerts)
    // {
    //     printf("NO ALERTS ACTIVE\n");
    // }

}
