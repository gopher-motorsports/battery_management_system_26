/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "taskStatistics.h"
#include "adbms/adbmsCellMonitor.h"
#include "packData.h"
#include "cellMonitorTelemetry.h"

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

static void updateCellMonitorStatistics(cellMonitor_S *bmb);

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

void updateCellMonitorStatistics(cellMonitor_S *bmb)
{
    for(uint32_t i = 0; i < NUM_CELL_MON; i++)
    {
        cellMonitor_S* pBmb = &bmb[i];
        float maxCellVoltage = MIN_CELLV_SENSOR_VALUE;
        float minCellVoltage = MAX_CELLV_SENSOR_VALUE;
        float sumVoltage = 0.0f;
        uint32_t numGoodCellVoltage = 0;

        float maxCellTemp = MIN_TEMP_SENSOR_VALUE_C;
        float minCellTemp = MAX_TEMP_SENSOR_VALUE_C;
        float sumCellTemp = 0.0f;
        uint32_t numGoodCellTemp = 0;

        // Aggregate cell voltage and temperature data
        for(uint32_t j = 0; j < NUM_CELLS_PER_CELL_MONITOR; j++)
        {
            // Only update stats if sense status is good
            if(pBmb->cellVoltageStatus[j] == GOOD)
            {
                float cellV = pBmb->cellVoltage[j];

                if(cellV > maxCellVoltage)
                {
                    maxCellVoltage = cellV;
                }
                if(cellV < minCellVoltage)
                {
                    minCellVoltage = cellV;
                }
                numGoodCellVoltage++;
                sumVoltage += cellV;
            }

            // Only update stats if sense status is good
            if(pBmb->cellTempStatus[j] == GOOD)
            {
                float cellTemp = pBmb->cellTemp[j];

                if (cellTemp > maxCellTemp)
                {
                    maxCellTemp = cellTemp;
                }
                if (cellTemp < minCellTemp)
                {
                    minCellTemp = cellTemp;
                }
                numGoodCellTemp++;
                sumCellTemp += cellTemp;
            }
        }

        // Update BMB statistics
        // TODO: determine what to do with BAD sensor status variables
        if(numGoodCellVoltage > 0)
        {
            pBmb->maxCellVoltage = maxCellVoltage;
            pBmb->minCellVoltage = minCellVoltage;
            pBmb->sumCellVoltage = sumVoltage;
            pBmb->avgCellVoltage = (sumVoltage / numGoodCellVoltage);
            pBmb->numBadCellVoltage = NUM_CELLS_PER_CELL_MONITOR - numGoodCellVoltage;
        }

        if(numGoodCellTemp > 0)
        {
            pBmb->maxCellTemp = maxCellTemp;
            pBmb->minCellTemp = minCellTemp;
            pBmb->avgCellTemp = (sumCellTemp / numGoodCellTemp);
            pBmb->numBadCellTemp = NUM_CELLS_PER_CELL_MONITOR - numGoodCellTemp;
        }
    }
}

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void updateBatteryStatistics(cellMonitorTaskData_S *taskData)
{
    // Update BMB level stats
	updateCellMonitorStatistics(taskData->cellMonitor);

	float maxCellVoltage = MIN_CELLV_SENSOR_VALUE;
    float minCellVoltage = MAX_CELLV_SENSOR_VALUE;
    float sumVoltage = 0.0f;
    float sumAvgCellVoltage = 0.0f;
    uint32_t numGoodBmbsCellV = 0;

    float maxCellTemp = MIN_TEMP_SENSOR_VALUE_C;
    float minCellTemp = MAX_TEMP_SENSOR_VALUE_C;
    float sumAvgCellTemp = 0.0f;
    uint32_t numGoodBmbsCellTemp = 0;

	float maxBoardTemp = MIN_TEMP_SENSOR_VALUE_C;
	float minBoardTemp = MAX_TEMP_SENSOR_VALUE_C;
	float sumBoardTemp = 0.0f;
    uint32_t numGoodBoardTemp = 0;

    float maxDieTemp = MIN_TEMP_SENSOR_VALUE_C;
    float minDieTemp = 200.0f;
    float sumDieTemp = 0.0f;
    uint32_t numGoodDieTemp;

	for(int32_t i = 0; i < NUM_CELL_MON; i++)
	{
		cellMonitor_S* pBmb = &taskData->cellMonitor[i];

        if(pBmb->numBadCellVoltage != NUM_CELLS_PER_CELL_MONITOR)
        {
            if(pBmb->maxCellVoltage > maxCellVoltage)
            {
                maxCellVoltage = pBmb->maxCellVoltage;
            }
            if(pBmb->minCellVoltage < minCellVoltage)
            {
                minCellVoltage = pBmb->minCellVoltage;
            }

            numGoodBmbsCellV++;
            sumAvgCellVoltage += pBmb->avgCellVoltage;
            sumVoltage += pBmb->sumCellVoltage;
        }

        if(pBmb->numBadCellTemp != NUM_CELLS_PER_CELL_MONITOR)
        {
            if (pBmb->maxCellTemp > maxCellTemp)
            {
                maxCellTemp = pBmb->maxCellTemp;
            }
            if (pBmb->minCellTemp < minCellTemp)
            {
                minCellTemp = pBmb->minCellTemp;
            }

            numGoodBmbsCellTemp++;
            sumAvgCellTemp += pBmb->avgCellTemp;
        }

        if(pBmb->boardTemp1Status == GOOD)
        {
            if (pBmb->boardTemp1 > maxBoardTemp)
            {
                maxBoardTemp = pBmb->boardTemp1;
            }
            if (pBmb->boardTemp1 < minBoardTemp)
            {
                minBoardTemp = pBmb->boardTemp1;
            }
            numGoodBoardTemp++;
            sumBoardTemp += pBmb->boardTemp1;
        }

        if(pBmb->boardTemp2Status == GOOD)
        {
            if (pBmb->boardTemp2 > maxBoardTemp)
            {
                maxBoardTemp = pBmb->boardTemp2;
            }
            if (pBmb->boardTemp2 < minBoardTemp)
            {
                minBoardTemp = pBmb->boardTemp2;
            }
            numGoodBoardTemp++;
            sumBoardTemp += pBmb->boardTemp2;
        }

        if(pBmb->regTempStatus == GOOD)
        {
            if (pBmb->regTemp > maxBoardTemp)
            {
                maxBoardTemp = pBmb->regTemp;
            }
            if (pBmb->regTemp < minBoardTemp)
            {
                minBoardTemp = pBmb->regTemp;
            }
            numGoodBoardTemp++;
            sumBoardTemp += pBmb->regTemp;
        }

        // if(pBmb->dieTempStatus == GOOD)
        // {
        //     if (pBmb->dieTemp > maxDieTemp)
        //     {
        //         maxDieTemp = pBmb->dieTemp;
        //     }
        //     if (pBmb->dieTemp < minDieTemp)
        //     {
        //         minDieTemp = pBmb->dieTemp;
        //     }

        //     numGoodDieTemp++;
        //     sumDieTemp += pBmb->dieTemp;
        // }
	}

    // TODO: Should i ignore this if bad sensors or open wires?
    taskData->cellSumVoltage = sumVoltage;

    if(numGoodBmbsCellV > 0)
    {
        taskData->maxCellVoltage = maxCellVoltage;
        taskData->minCellVoltage = minCellVoltage;
        taskData->avgCellVoltage = sumAvgCellVoltage / numGoodBmbsCellV;
        taskData->cellImbalance = maxCellVoltage - minCellVoltage;
    }

    if(numGoodBmbsCellTemp > 0)
    {
        taskData->maxCellTemp = maxCellTemp;
        taskData->minCellTemp = minCellTemp;
        taskData->avgCellTemp = sumAvgCellTemp / numGoodBmbsCellTemp;
    }

    if(numGoodBoardTemp > 0)
    {
        taskData->maxBoardTemp = maxBoardTemp;
        taskData->minBoardTemp = minBoardTemp;
        taskData->avgBoardTemp = sumBoardTemp / numGoodBoardTemp;
        taskData->numBadBoardTemp = NUM_BOARD_TEMP_SENSORS - numGoodBoardTemp;
    }

    if(numGoodDieTemp > 0)
    {
        taskData->maxDieTemp = maxDieTemp;
        taskData->minDieTemp = minDieTemp;
        taskData->avgDieTemp = sumDieTemp / numGoodDieTemp;
    }
}
